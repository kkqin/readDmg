#pragma once

#include <zlib.h>

/*
// from the tttttUDZO.dmg offset: 469
+++++++++++++++++++mish=====================
fUDIFBlocksSignature:1835627368
infoVersion:1
firstSectorNumber:0
sectorCount:1
dataStart:0
decompressBufferRequested:2056
blocksDescriptor:0
blocksRunCount:2
===================BLKXRun++++++++++++++++
type:2147483653
reserved:3
sectorStart:0
sectorCount:1
compOffset:469
compLength:32
*/
unsigned char firstblock[32] = {
	0x78, 0x01, 0x63, 0x60, 0x18, 0x05, 0x43, 0x38, 0x04, 0xFE, 0xFD, 0xFF, 0xFF, 0x0E, 0x88, 0x19, 
	0x81, 0x5E, 0x78, 0xDF, 0x5D, 0x0D, 0xA2, 0x48, 0x02, 0xA1, 0xAB, 0x00, 0x17, 0xBE, 0x09, 0xDD
};


#define CHUNKSIZE 0x100000

struct outST {
	unsigned char* out;
	uint64_t size;
};

struct outST* start_parse(unsigned char* src, uint64_t size) {
	z_stream z;
	z.zalloc = Z_NULL;
	z.zfree = Z_NULL;
	z.opaque = Z_NULL;

	int err = inflateInit(&z);
	if (err != Z_OK) {
		fprintf(stderr, "Can't initialize inflate stream: %d\n", err);
		return NULL;
	}

	z.avail_in = size;
	if (z.avail_in == 0)
		return NULL;;

	z.next_in = src;
	z.avail_out = CHUNKSIZE;

	struct outST* st = new struct outST; 
	st->out = (Bytef *) malloc(CHUNKSIZE);
	z.next_out = st->out;

	err = inflate(&z, Z_NO_FLUSH);
	assert(err != Z_STREAM_ERROR);  /* state not clobbered */

	st->size = CHUNKSIZE - z.avail_out;

	return st;
}

