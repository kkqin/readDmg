#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <random>
#ifdef _TSK_
	#include <tsk/libtsk.h>
	#include "/home/k/Documents/sleuthkit/tsk/img/dmg.h"
#else
	#include "core_st.h"
#endif

class RawDmg {
	std::ifstream file_;
public:
	bool open_file(std::string fileName) { 
		file_.open(fileName, std::ifstream::in | std::ifstream::binary);
		if(!file_.is_open()) {
			std::cout << "error" << std::endl;
			return false;
		}

		file_.seekg(0);	
		return true;
	}

	void read_file(uint64_t offset, char* buf, size_t a_len) {
		file_.seekg(offset);
		file_.read(buf, a_len);
	}

	void close () {
		file_.close();
	}
};

int main(int argc ,char** argv) {
	auto dmg = core_::koly_block(argv[1]);
	auto file_dmg = RawDmg(); 
	file_dmg.open_file(argv[2]);

	std::cout << "target file 1: " << argv[1] << std::endl;
	std::cout << "target file 2: " << argv[2] << std::endl;

	uint64_t max_offset = 12735471616;
	unsigned i = 0;
	char* buf1, *buf2;
	int cmp_size = 0x100000;

	//////////tsk test//////////
#ifdef _TSK_
	TSK_IMG_INFO * img_info = tsk_img_open_sing((const TSK_TCHAR *) argv[1], TSK_IMG_TYPE_DETECT, 0);
        if (img_info == NULL) {
                fprintf(stderr, "Error opening file\n");
                tsk_error_print(stderr);
                exit(1);
        }
#endif
	
//	ddmg_read(img_info, 0, buf1, 512);

	////////////////////////////

	std::random_device rd;
	std::uniform_int_distribution<uint64_t> dist(0, max_offset);

	//uint64_t offset = 1957691392; 
	//uint64_t offset = 511; 
	//uint64_t offset = 343933439; 
	uint64_t offset = 12734956031; 
	while(offset < max_offset) {
		
		buf1 = new char[0x100000];
		buf2 = new char[0x100000];
#ifdef _TSK_
		ddmg_read(img_info, offset, buf1, cmp_size);
#else
		dmg->read(offset, buf1, cmp_size);
#endif
		file_dmg.read_file(offset, buf2, cmp_size);

		if(strncmp(buf1, buf2, cmp_size)) {
			std::cout << std::endl;
			std::cout << std::dec << "offset: " << offset << " buffer different." << std::endl; 
			std::cout << std::endl;
		}
		else {
			std::cout << "\r";
			std::cout << std::dec << "offset : " << offset << 
			std::hex << 
			" \tbyte1:" << static_cast<unsigned short>(buf1[0]) << 
			" \tbyte2:" << static_cast<unsigned short>(buf2[0]) <<
			" \tcool" << std::flush;
		}
	
		offset += cmp_size;		
		delete  buf1;
		delete  buf2;
	}

	file_dmg.close();
}
