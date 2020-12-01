#pragma once
#include "mymishblock.h"
#include <vector>
#include <memory>
#include <cstring>
#include <map>
#include <zlib.h>
#include <fstream>

#define BT_ADC   0x80000004
#define BT_ZLIB  0x80000005
#define BT_BZLIB 0x80000006

#define BT_ZERO 0x00000000
#define BT_RAW 0x00000001
#define BT_IGNORE 0x00000002
#define BT_COMMENT 0x7ffffffe
#define BT_TERM 0xffffffff

#define CHUNKSIZE 0x100000
#define ASSERT(x, m) if(!(x)) { fflush(stdout); fprintf(stderr, "error: %s\n", m); perror("error"); fflush(stderr); exit(1); }


inline unsigned long long swapByteOrder(unsigned long long ull)
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

inline int convert_int(int i)
{
        int o;
        char *p_i = (char *) &i;
        char *p_o = (char *) &o;
        p_o[0] = p_i[3];
        p_o[1] = p_i[2];
        p_o[2] = p_i[1];
        p_o[3] = p_i[0];
        return o;
}

inline uint64_t convert_int64(uint64_t i)
{
        uint64_t o;
        char *p_i = (char *) &i;
        char *p_o = (char *) &o;
        p_o[0] = p_i[7];
        p_o[1] = p_i[6];
        p_o[2] = p_i[5];
        p_o[3] = p_i[4];
        p_o[4] = p_i[3];
        p_o[5] = p_i[2];
        p_o[6] = p_i[1];
        p_o[7] = p_i[0];
        return o;
}

namespace core_ {

typedef struct DMG {
	std::vector<BLKXTable*> blkx;
	std::map<uint64_t, BLKXRun*> blkx_runs;
	uint64_t disk_size;
	uint64_t forward_size;
	std::ifstream _file;

	DMG() : disk_size(0), forward_size(0) {}
	void read(uint64_t offset, char* buf, size_t a_len);
	int parse_run(BLKXRun* run, char* buf);
} DMG;

typedef struct PLIST_XML {
	char* data; 
	uint32_t size;
} PLIST_XML;

void parse_xml(std::shared_ptr<PLIST_XML> xml, std::shared_ptr<DMG> dmg);	

}

