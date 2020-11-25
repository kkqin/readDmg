#include <iostream>
#include <arpa/inet.h>
#include <uuid/uuid.h>
#include <fstream>
#include <cassert>
#include <cstring>
#include "mymishblock.h"
#include "mykolyblock.h"
#include "b64.h"
#include "parse_zlib_data.h"

typedef struct {
        char     Signature[4];          // Magic ('koly')
        uint32_t Version;               // Current version is 4
        uint32_t HeaderSize;            // sizeof(this), always 512
        uint32_t Flags;                 // Flags
        uint64_t RunningDataForkOffset; //
        uint64_t DataForkOffset;        // Data fork offset (usually 0, beginning of file)
        uint64_t DataForkLength;        // Size of data fork (usually up to the XMLOffset, below)
        uint64_t RsrcForkOffset;        // Resource fork offset, if any
        uint64_t RsrcForkLength;        // Resource fork length, if any
        uint32_t SegmentNumber;         // Usually 1, may be 0
        uint32_t SegmentCount;          // Usually 1, may be 0
        uuid_t   SegmentID;             // 128-bit GUID identifier of segment (if SegmentNumber !=0)

	uint32_t DataChecksumType;      // Data fork 
        uint32_t DataChecksumSize;      //  Checksum Information
        uint32_t DataChecksum[32];      // Up to 128-bytes (32 x 4) of checksum

        uint64_t XMLOffset;             // Offset of property list in DMG, from beginning
        uint64_t XMLLength;             // Length of property list
        uint8_t  Reserved1[120];        // 120 reserved bytes - zeroed

	uint32_t ChecksumType;          // Master
        uint32_t ChecksumSize;          //  Checksum information
        uint32_t Checksum[32];          // Up to 128-bytes (32 x 4) of checksum

        uint32_t ImageVariant;          // Commonly 1
        uint64_t SectorCount;           // Size of DMG when expanded, in sectors

        uint32_t reserved2;             // 0
        uint32_t reserved3;             // 0 
        uint32_t reserved4;             // 0

} __attribute__((packed, scalar_storage_order("big-endian"))) UDIFResourceFile;

unsigned long long swapByteOrder(unsigned long long ull)
{
    ull = (ull >> 56) |
          ((ull<<40) & 0x00FF000000000000) |
          ((ull<<24) & 0x0000FF0000000000) |
          ((ull<<8) & 0x000000FF00000000) |
          ((ull>>8) & 0x00000000FF000000) |
          ((ull>>24) & 0x0000000000FF0000) |
          ((ull>>40) & 0x000000000000FF00) |
          (ull << 56);
    return ull;
}

static void trim_space(char** location) {
	for(;strncmp(*location, "<", 1); (*location)++);
}

void mish_block(unsigned char* mish) {
	BLKXTable* mishblock = (BLKXTable*)mish;
	std::cout <<"fUDIFBlocksSignature:" << ntohl(mishblock->fUDIFBlocksSignature) << std::endl;
	std::cout <<"infoVersion:" << ntohl(mishblock->infoVersion) << std::endl;
	std::cout <<"firstSectorNumber:" << swapByteOrder(mishblock->firstSectorNumber) << std::endl;
	std::cout <<"sectorCount:" << swapByteOrder(mishblock->sectorCount) << std::endl;
	std::cout <<"dataStart:" << swapByteOrder(mishblock->dataStart) << std::endl;
	std::cout <<"decompressBufferRequested:" << ntohl(mishblock->decompressBufferRequested) << std::endl;
	std::cout <<"blocksDescriptor:" << ntohl(mishblock->blocksDescriptor) << std::endl;
	std::cout <<"blocksRunCount:" << ntohl(mishblock->blocksRunCount) << std::endl;

	std::cout << "===================BLKXRun++++++++++++++++" << std::endl;

	BLKXRun* entry = (BLKXRun*)mishblock->runs;
	std::cout <<"type:" << ntohl(entry->type) << std::endl;
	std::cout <<"reserved:" << ntohl(entry->reserved) << std::endl;
	std::cout <<"sectorStart:" << swapByteOrder(entry->sectorStart) << std::endl;
	std::cout <<"sectorCount:" << swapByteOrder(entry->sectorCount) << std::endl;
	std::cout <<"compOffset:" << swapByteOrder(entry->compOffset) << std::endl;
	std::cout <<"compLength:" << swapByteOrder(entry->compLength) << std::endl;
}

static char ascii_to_base(char str) 
{
	if (str >= 65 && str <= 90) {
		return str - 65;
	} else if (str >= 97 && str <= 122) {
		return str - 71;
	} else if (str >= 48 && str <= 57) {
		return str + 4;
	} else if (str == '+') {
		return 62;
	} else {
		return 63;
	}
}

unsigned char* decodeBase64(char* toDecode, size_t* dataLength) {
	uint8_t buffer[4];
	uint8_t charsInBuffer;
	unsigned char* curChar;
	unsigned char* decodeBuffer;
	unsigned int decodeLoc;
	unsigned int decodeBufferSize;
	uint8_t bytesToDrop;

	curChar = (unsigned char*) toDecode;
	charsInBuffer = 0;

	decodeBufferSize = 100;
	decodeLoc = 0;
	decodeBuffer = (unsigned char*) malloc(decodeBufferSize);

	bytesToDrop = 0;

	while((*curChar) != '\0') {
		if((*curChar) >= 'A' && (*curChar) <= 'Z') {
			buffer[charsInBuffer] = (*curChar) - 'A';
			charsInBuffer++;
		}

		if((*curChar) >= 'a' && (*curChar) <= 'z') {
			buffer[charsInBuffer] = ((*curChar) - 'a') + ('Z' - 'A' + 1);
			charsInBuffer++;
		}

		if((*curChar) >= '0' && (*curChar) <= '9') {
			buffer[charsInBuffer] = ((*curChar) - '0') + ('Z' - 'A' + 1)  + ('z' - 'a' + 1);
			charsInBuffer++;
		}

		if((*curChar) == '+') {
			buffer[charsInBuffer] = ('Z' - 'A' + 1)  + ('z' - 'a' + 1)  + ('9' - '0' + 1);
			charsInBuffer++;
		}

		if((*curChar) == '/') {
			buffer[charsInBuffer] = ('Z' - 'A' + 1)  + ('z' - 'a' + 1)  + ('9' - '0' + 1) + 1;
			charsInBuffer++;
		}

		if((*curChar) == '=') {
			bytesToDrop++;
		}

		if(charsInBuffer == 4) {
			charsInBuffer = 0;

			if((decodeLoc + 3) >= decodeBufferSize) {
				decodeBufferSize <<= 1;
				decodeBuffer = (unsigned char*) realloc(decodeBuffer, decodeBufferSize);
			}
			decodeBuffer[decodeLoc] = ((buffer[0] << 2) & 0xFC) + ((buffer[1] >> 4) & 0x3F);
			decodeBuffer[decodeLoc + 1] = ((buffer[1] << 4) & 0xF0) + ((buffer[2] >> 2) & 0x0F);
			decodeBuffer[decodeLoc + 2] = ((buffer[2] << 6) & 0xC0) + (buffer[3] & 0x3F);

			decodeLoc += 3;
			buffer[0] = 0;
			buffer[1] = 0;
			buffer[2] = 0;
			buffer[3] = 0;
		}

		curChar++;
	}

	if(bytesToDrop != 0) {  
		if((decodeLoc + 3) >= decodeBufferSize) {
			decodeBufferSize <<= 1;
			decodeBuffer = (unsigned char*) realloc(decodeBuffer, decodeBufferSize);
		}

		decodeBuffer[decodeLoc] = ((buffer[0] << 2) & 0xFC) | ((buffer[1] >> 4) & 0x3F);

		if(bytesToDrop <= 2)
			decodeBuffer[decodeLoc + 1] = ((buffer[1] << 4) & 0xF0) | ((buffer[2] >> 2) & 0x0F);

		if(bytesToDrop <= 1)
			decodeBuffer[decodeLoc + 2] = ((buffer[2] << 6) & 0xC0) | (buffer[3] & 0x3F);

		*dataLength = decodeLoc + 3 - bytesToDrop;
	} else {
		*dataLength = decodeLoc;
	}

	return decodeBuffer;
}

char* prepare_decode(std::string xml_inside_data_file, size_t *filesize) {
	std::ifstream rfile;
	rfile.open(xml_inside_data_file, std::ifstream::in | std::ifstream::binary);
	if(!rfile.is_open()) {
		std::cout << "error" << std::endl;
		return NULL;
	}

	rfile.seekg(0, std::ios_base::end);
	*filesize =  rfile.tellg();
	char* buf = new char[*filesize];
	rfile.seekg(0);
	rfile.read(buf, *filesize);
	rfile.close();

	return buf;
}


void open_and_decode_mish(std::string name) {
	size_t fileSize;
	char* buf = prepare_decode(name, &fileSize);
	buf[fileSize] = '\0';
	size_t length;
	unsigned char* outDecode = b64_decode_ex(buf, fileSize, &length); //// 
	//unsigned char* res = decodeBase64(buf, &filesize);
	mish_block(outDecode);
	delete [] buf;
}

static char* getXMLData(char** location, size_t *dataLength) {
	char* curLoc;
	char* tagEnd;
	char* toReturn;
	size_t strLen;

	curLoc = *location;

	curLoc = strstr(curLoc, "<data>");
	if(!curLoc)
		return NULL;
	curLoc += sizeof("<data>") - 1;

	tagEnd = strstr(curLoc, "</data>");

	*dataLength = (size_t)(tagEnd - curLoc);

	toReturn = (char*) malloc(*dataLength + 1);
	memcpy(toReturn, curLoc, *dataLength); // 原始AAAA, 不解压
	toReturn[*dataLength] = '\0';

	curLoc = tagEnd + sizeof("</data>") - 1;

	*location = curLoc;

	return toReturn;
}

char* read_xml(char* xml, uint64_t* dataSize) {
	char* curLoc;
	char* tagEnd;
	char* toReturn;

	curLoc = strstr(xml, "<dict>");
	if(!curLoc)
		return NULL;

	curLoc += sizeof("<dict>") - 1;

	for(; ;) {
		// key
		curLoc = strstr(curLoc, "<key>");	
		curLoc += sizeof("<key>") - 1;
		tagEnd = strstr(curLoc, "</key>");

		curLoc = strstr(curLoc, "<array>");
		tagEnd = strstr(curLoc, "</array>");

		int i = 0; 
		while(curLoc < tagEnd) {
			
			curLoc = strstr(curLoc, "<dict>");
			char* dictEnd = strstr(curLoc, "</dict>");
			
			while(curLoc < dictEnd) {
				curLoc = strstr(curLoc, "<key>");			
				curLoc += sizeof("<key>") - 1;				
				
				char* tagEnd = strstr(curLoc, "</key>");
				size_t strLen = (size_t)(tagEnd - curLoc);
				char* tagBegin = curLoc;
				curLoc = tagEnd + sizeof("</key>") - 1;

				if(strncmp(tagBegin, "Data", strLen) == 0 && i == 0) {
					toReturn = getXMLData(&curLoc, dataSize);
					return toReturn;
				}
				
				trim_space(&curLoc);
			}

			curLoc = dictEnd + sizeof("</dict>") - 1;
			trim_space(&curLoc);
			i++;
		}
		curLoc = tagEnd + sizeof("</array>") - 1;
	}
	
	return NULL;
}

void outXML(std::ifstream& file, std::string file_dmg, uint64_t start_offset, uint64_t size) {
	if(!size) 
		return ;
	std::ofstream wfile;
	wfile.open(file_dmg + ".xml", std::ofstream::out | std::ifstream::binary);
	file.seekg(start_offset);
	 
	char *buf = new char[size]; 
	file.read(buf, size);
	uint64_t dataSize;
	char* data = read_xml(buf, &dataSize);
	size_t length;
	unsigned char* outDecode = decodeBase64(data, &length); //base_decode(data);// 
	mish_block(outDecode);
	//wfile.write(buf, size);
}

void show_kolyblock(UDIFResourceFile* kolyblock) {
	std::cout <<"Signature: " << kolyblock->Signature << std::endl;
	std::cout <<"Version: " << ntohl(kolyblock->Version) << std::endl; // 转成大端
	std::cout <<"HeaderSize: " << ntohl(kolyblock->HeaderSize) << std::endl;
	std::cout <<"Flags: " << ntohl(kolyblock->Flags) << std::endl;
	std::cout <<"RunningDataForkOffset: " << swapByteOrder(kolyblock->RunningDataForkOffset) << std::endl;
	std::cout <<"DataForkOffset: " << swapByteOrder(kolyblock->DataForkOffset) << std::endl;
	std::cout <<"DataForkLength: " << swapByteOrder(kolyblock->DataForkLength) << std::endl;
	std::cout <<"RsrcForkOffset: " << swapByteOrder(kolyblock->RsrcForkOffset) << std::endl;
	std::cout <<"RsrcForkLength: " << swapByteOrder(kolyblock->RsrcForkLength) << std::endl;
	std::cout <<"SegmentNumber: " << ntohl(kolyblock->SegmentNumber) << std::endl;
	std::cout <<"SegmentCount: " << ntohl(kolyblock->SegmentCount) << std::endl;
	std::cout <<"SegmentID: " << kolyblock->SegmentID << std::endl;

	std::cout <<"DataChecksumType: " << ntohl(kolyblock->DataChecksumType) << std::endl;
	std::cout <<"DataChecksumSize: " << ntohl(kolyblock->DataChecksumSize) << std::endl;
	std::cout <<"DataChecksum: " << kolyblock->DataChecksum << std::endl;

	std::cout <<"XMLOffset: " << swapByteOrder(kolyblock->XMLOffset) << std::endl;
	std::cout <<"XMLLength: " << swapByteOrder(kolyblock->XMLLength) << std::endl;
	std::cout <<"Reserved1: " << kolyblock->Reserved1 << std::endl;

	std::cout <<"ChecksumType: " << ntohl(kolyblock->ChecksumType) << std::endl;
	std::cout <<"ChecksumSize: " << ntohl(kolyblock->ChecksumSize) << std::endl;
	std::cout <<"Checksum: " << kolyblock->Checksum << std::endl;

	std::cout <<"ImageVariant: " << kolyblock->ImageVariant << std::endl;
	std::cout <<"SectorCount: " << swapByteOrder(kolyblock->SectorCount) << std::endl;

	std::cout <<"reserved2: " << kolyblock->reserved2 << std::endl;
	std::cout <<"reserved3: " << kolyblock->reserved3 << std::endl;
	std::cout <<"reserved4: " << kolyblock->reserved4 << std::endl;
}

int koly_block(std::string file_dmg) {
	std::ifstream rfile;
	rfile.open(file_dmg, std::ifstream::in | std::ifstream::binary);
	if(!rfile.is_open()) {
		std::cout << "error" << std::endl;
		return -1;
	}
	rfile.seekg(0, std::ios_base::end);
	uint64_t filesize = rfile.tellg();
	rfile.seekg(filesize - sizeof(UDIFResourceFile));
	char *_data = new char[sizeof(UDIFResourceFile)];
	rfile.read(_data, sizeof(UDIFResourceFile));
	
	UDIFResourceFile* kolyblock = (UDIFResourceFile*)_data;

	assert(!strcmp(kolyblock->Signature, "koly"));
	std::cout << "+++++++++++++++++++koly=====================" << std::endl;
	show_kolyblock(kolyblock);
	std::cout << "+++++++++++++++++++mish=====================" << std::endl;
	outXML(rfile, file_dmg, swapByteOrder(kolyblock->XMLOffset), swapByteOrder(kolyblock->XMLLength));
}



int main(int argc, char** argv) {
	koly_block(argv[1]);

	std::cout << "++++++++++++++++++++=====================" << std::endl;
	//mish_block(mish);
	//open_and_decode_mish(argv[2]);

	return 0;
}

