#include <iostream>
#include <arpa/inet.h>
#include <uuid/uuid.h>
#include <cassert>
#include <cstring>
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


int main(int argc, char** argv) {
	auto dmg = core_::koly_block(argv[1]);
	unsigned block_size = 4;
	//uint64_t offset = 209735680;
	//uint64_t offset = 482099200;
	uint64_t offset = 3776959008;
	//uint64_t offset = 1224;
	char* buf = new char[block_size];
	dmg->read(offset, buf, block_size);
	std::cout << std::hex << buf[block_size - 1] << std::endl;
	return 0;
}

