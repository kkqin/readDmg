#pragma once
#include <stdint.h>

typedef struct {
	uint32_t type;
	uint32_t size;
	uint32_t data[0x20];
} __attribute__((__packed__)) UDIFChecksum;

typedef struct {
	uint32_t type;
	uint32_t reserved;
	uint64_t sectorStart;
	uint64_t sectorCount;
	uint64_t compOffset;
	uint64_t compLength;
} __attribute__((__packed__)) BLKXRun;

typedef struct {
	uint32_t fUDIFBlocksSignature;
	uint32_t infoVersion;
	uint64_t firstSectorNumber;
	uint64_t sectorCount;
	
	uint64_t dataStart;
	uint32_t decompressBufferRequested;
	uint32_t blocksDescriptor;
	
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
	uint32_t reserved4;
	uint32_t reserved5;
	uint32_t reserved6;
	
	UDIFChecksum checksum;
	
	uint32_t blocksRunCount;
	char* runs;
} __attribute__((__packed__)) BLKXTable;


