#include "core_st.h"
#include <iostream>
#include <cstring>
#include <cassert>

namespace core_ {

static void trim_space(char** location) {
	for(;strncmp(*location, "<", 1); (*location)++);
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

void fill_mishblk(unsigned char* c, BLKXTable* m)
{
        memset(m, 0, sizeof(BLKXTable));
        memcpy(m, c, 0xCC);

        m->fUDIFBlocksSignature = convert_int(m->fUDIFBlocksSignature);
        m->infoVersion = convert_int(m->infoVersion);
        m->firstSectorNumber = convert_int64(m->firstSectorNumber);
        m->sectorCount = convert_int64(m->sectorCount);
        m->dataStart = convert_int64(m->dataStart);
        m->decompressBufferRequested = convert_int(m->decompressBufferRequested);
        m->blocksDescriptor = convert_int(m->blocksDescriptor);
        //m->checksum = convert_m->checksum;
        m->blocksRunCount = convert_int(m->blocksRunCount);
}

static BLKXTable* getXMLData(char** location, size_t *dataLength) {
	char* curLoc;
	char* tagEnd;
	char* AAAAA;
	size_t strLen;
	unsigned char* decodeData;

	curLoc = *location;

	curLoc = strstr(curLoc, "<data>");
	if(!curLoc)
		return NULL;
	curLoc += sizeof("<data>") - 1;

	tagEnd = strstr(curLoc, "</data>");

	*dataLength = (size_t)(tagEnd - curLoc);

	AAAAA = (char*) malloc(*dataLength + 1);
	memcpy(AAAAA, curLoc, *dataLength); // raw AAAA 
	AAAAA[*dataLength] = '\0';

 	decodeData = decodeBase64(AAAAA, dataLength); 
	BLKXTable* mishblk = (BLKXTable*)malloc(sizeof(BLKXTable));
	fill_mishblk(decodeData, mishblk);
	mishblk->runs = (char*)malloc(sizeof(BLKXRun) * mishblk->blocksRunCount);
	memcpy(mishblk->runs, decodeData + sizeof(BLKXTable) - sizeof(char*), sizeof(BLKXRun) * mishblk->blocksRunCount);

	free(AAAAA);
	free(decodeData);
	curLoc = tagEnd + sizeof("</data>") - 1;

	*location = curLoc;

	return mishblk;
}

void dispath(BLKXTable* mish, std::shared_ptr<DMG> dmg) {
	unsigned int block_type;
	uint64_t out_offs, out_size;
	for(auto i = 0; i < mish->blocksRunCount; i++) {
		BLKXRun* runs = (BLKXRun*)(mish->runs + sizeof(BLKXRun) * i);
		block_type = convert_int(runs->type);
		if(block_type == BT_TERM)
			continue;	
		out_offs = convert_int64(runs->sectorStart) * 512;
		out_size = convert_int64(runs->sectorCount) * 512;
		dmg->disk_size += out_size;
		dmg->forward_size = dmg->disk_size - out_size;
		dmg->blkx_runs[dmg->forward_size] = runs;
	}
}

void parse_xml(std::shared_ptr<PLIST_XML> xml, std::shared_ptr<DMG> dmg) {
	char* curLoc;
	char* tagEnd;

	curLoc = strstr(xml->data, "<key>resource-fork</key>");
	if(!curLoc)
		return;
	curLoc += sizeof("<key>resource-fork</key>") - 1;

	curLoc = strstr(curLoc, "<dict>");
	if(!curLoc)
		return ;
	curLoc += sizeof("<dict>") - 1;

	for(; ;) {
		curLoc = strstr(curLoc, "<key>");	
		curLoc += sizeof("<key>") - 1;
		tagEnd = strstr(curLoc, "</key>");
		if(!tagEnd)
			break;
	
		// key is not the blkx should break out
		size_t strLen = (size_t)(tagEnd - curLoc);
		if(strncmp(curLoc, "blkx", strLen) != 0) 
			break;

		curLoc = strstr(curLoc, "<array>");
		tagEnd = strstr(curLoc, "</array>");
		if(!tagEnd)
			break;
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

				if(strncmp(tagBegin, "Data", strLen) == 0) {
					size_t dataSize;			
					BLKXTable* mish = getXMLData(&curLoc, &dataSize);
					dispath(mish, dmg);
					dmg->blkx.push_back(mish);
				}
				else if(strncmp(tagBegin, "CFName", strLen) == 0) {
					
				}
				
				trim_space(&curLoc);
			}

			curLoc = dictEnd + sizeof("</dict>") - 1;
			trim_space(&curLoc);
		}
		curLoc = tagEnd + sizeof("</array>") - 1;
	}
}

void DMG::read(uint64_t offset, char* buf, size_t a_len) {
	if(offset > disk_size || a_len > disk_size)  
		return;

	auto iter = blkx_runs.upper_bound(offset);
	--iter;

	uint64_t start_offset = iter->first;
	uint64_t diff_byte = offset - start_offset;

	size_t out_size, tmp_len;
	char* step_buf = buf;
	for(; a_len; iter++) {
		uint64_t offset = iter->first;
		BLKXRun* run = iter->second;
		out_size = convert_int64(run->sectorCount) * 512 ;
		
		// parse read
		char* inside_buf = new char[out_size]; 
		parse_run(run, inside_buf, out_size); // out_size 
		if(out_size <= a_len)
			tmp_len = out_size;
		else 
			tmp_len = a_len;

		memcpy(step_buf, inside_buf + diff_byte, tmp_len); 
		a_len -= tmp_len;
		step_buf += tmp_len; 
		diff_byte = 0;
		delete inside_buf;
	}
}

int DMG::parse_run(BLKXRun* run, char* buf_, size_t min_size) {
	unsigned int block_type = convert_int(run->type);
	uint64_t in_offs = convert_int64(run->compOffset), 
		in_size = convert_int64(run->compLength),
		out_size = convert_int64(run->sectorCount) * 512;
	
	uint64_t to_read, chunk, to_write;
	int err;
	char* buf_head = buf_; // point to start
	z_stream z;
	z.zalloc = (alloc_func) 0;
        z.zfree = (free_func) 0;
        z.opaque = (voidpf) 0;
	Bytef* otmp = (Bytef *) malloc(CHUNKSIZE);
	Bytef* tmp = (Bytef *) malloc(CHUNKSIZE);
	if(block_type == BT_ZLIB) {
		err = inflateInit(&z);
		if (err != Z_OK) {
			fprintf(stderr, "Can't initialize inflate stream: %d\n", err);
			return 1;
		}

		to_read = in_size;
		do {
			if (!to_read)
				break;
			if (to_read > CHUNKSIZE)
				chunk = CHUNKSIZE;
			else
				chunk = to_read;
		
			_file.seekg(in_offs);
			_file.read(reinterpret_cast<char*>(tmp), chunk);
			z.avail_in = chunk;
			to_read -= z.avail_in;
			z.next_in = tmp;
			do {
				z.avail_out = CHUNKSIZE;
				z.next_out = otmp;
				err = inflate(&z, Z_NO_FLUSH);
				assert(err != Z_STREAM_ERROR);  /* state not clobbered */
				switch (err) {
					case Z_NEED_DICT:
						err = Z_DATA_ERROR;     /* and fall through */
					case Z_DATA_ERROR:
					case Z_MEM_ERROR:
						(void)inflateEnd(&z);
						fprintf(stderr, "Inflation failed\n");
						return 1;
				}
				to_write = CHUNKSIZE - z.avail_out;
				if(min_size < to_write)
					to_write = min_size;
				memcpy(buf_head, otmp, to_write);
				buf_head += to_write;
			} while( z.avail_out == 0 );
		} while ( err != Z_STREAM_END );
	}
	else if (block_type == BT_ZERO || block_type == BT_IGNORE) {
		memset(tmp, 0, CHUNKSIZE);
		to_write = out_size;
		if(min_size < to_write)
			to_write = min_size;

		while (to_write > 0) {
			if (to_write > CHUNKSIZE)
				chunk = CHUNKSIZE;
			else
				chunk = to_write;

			memcpy(buf_head, tmp, chunk);
			buf_head += chunk;
			to_write -= chunk;
		}
	} 
	else if (block_type == BT_RAW) {
		to_read = in_size;
		_file.seekg(in_offs);
	
		if(min_size < to_read)
			to_read = min_size;

		while (to_read > 0) {
			if (to_read > CHUNKSIZE)
				chunk = CHUNKSIZE;
			else
				chunk = to_read;

			_file.read(reinterpret_cast<char*>(tmp), chunk);
			memcpy(buf_head, tmp, chunk);
			buf_head += chunk;
			to_read -= chunk;
		}
	}

	if (tmp != NULL)
                free(tmp);
        if (otmp != NULL)
                free(otmp);

	buf_head = NULL;

	(void)inflateEnd(&z);
	return 0;
}

}
