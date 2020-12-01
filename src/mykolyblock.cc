#include <iostream>
#include <arpa/inet.h>
#include <uuid/uuid.h>
#include <cassert>
#include <cstring>
#include "mykolyblock.h"
//#include "b64.h"
//#include "parse_zlib_data.h"
#include "core_st.h"

BLKXTable* mish_block(unsigned char* mish) {
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

	return mishblock;
}

void process_plist_xml(std::ifstream& file, uint64_t start_offset, uint64_t size) {

	file.seekg(start_offset);
	auto xml = std::make_shared<core_::PLIST_XML>();
	xml->data = new char[size];
	xml->size = size;
	file.read(xml->data, size);

	auto dmg = std::make_shared<core_::DMG>(); 
	core_::parse_xml(xml, dmg);
	dmg->_file = std::move(file);
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
	process_plist_xml(rfile, swapByteOrder(kolyblock->XMLOffset), swapByteOrder(kolyblock->XMLLength));
	//std::cout << "+++++++++++++++++++mish=====================" << std::endl;
	//outXML(rfile, file_dmg, swapByteOrder(kolyblock->XMLOffset), swapByteOrder(kolyblock->XMLLength));
	return 0;
}

int main(int argc, char** argv) {
	koly_block(argv[1]);
	return 0;
}

