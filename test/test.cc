#include <iostream>
#include <fstream>
#include "core_st.h"
#include <string>
#include <random>

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

	uint64_t max_offset = dmg->disk_size;
	unsigned i = 1000;
	char* buf1, *buf2;
	buf1 = new char;
	buf2 = new char;

	std::random_device rd;
	std::uniform_int_distribution<uint64_t> dist(0, max_offset);

	while(i--) {
		uint64_t offset = dist(rd);
		
		dmg->read(offset, buf1, 3);
		file_dmg.read_file(offset, buf2, 3);

		if(strncmp(buf1, buf2, 3)) {
			std::cout << "offset: " << offset << " buffer different." << std::endl; 
		}
		else {
			std::cout << "\r";
			std::cout << std::dec << "offset : " << offset << 
			std::hex << 
			" \tbyte1:" << static_cast<unsigned short>(buf1[0]) << 
			" \tbyte2:" << static_cast<unsigned short>(buf2[0]) <<
			" \tcool" <<std::flush;
		}
	}

	file_dmg.close();
	delete buf1;
	delete buf2;
}
