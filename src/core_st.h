#pragma once
#include "mymishblock.h"
#include <vector>
#include <memory>
#include <cstring>

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

namespace core_ {

typedef struct DMG {
	std::vector<BLKXTable*> blkx;
} DMG;

typedef struct PLIST_XML {
	char* data; 
	uint32_t size;
} PLIST_XML;

void parse_xml(std::shared_ptr<PLIST_XML> xml, std::shared_ptr<DMG> dmg);	

}

