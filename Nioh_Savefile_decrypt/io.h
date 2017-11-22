#pragma once

#include <iostream>
#include <fstream>
#include <assert.h>
#include <experimental\filesystem>

class io
{

public:
	static void print_array(unsigned char* c, int size) {
		for (int j = 0; j < size; j++) {
			if (j % 16 == 0)
				printf("\n");
			printf("%02X ", c[j]);
		}
		printf("\n");
	}

	static void print_array_as_string(unsigned char* c, int size) {
		for (int j = 0; j < size; j++) {
			if (j % 32 == 0)
				printf("\n");
			printf("%c", c[j]);

		}
		printf("\n");
	}

	static bool parse_savefile(std::string path, unsigned char* buffer,unsigned int size) {
		std::ifstream savefile(path, std::ios::in|std::ios::binary);
		if (savefile.is_open()) {
			savefile.read(reinterpret_cast<char*>(buffer), size);
			savefile.close();
			return true;
		}
		return false;
	}

	static bool save_decrypted_savefile(std::string path, unsigned char* buffer, unsigned int size) {
		std::experimental::filesystem::path file_path(path);
		std::ofstream savefile("decr_"+file_path.filename().string(), std::ios::out | std::ios::binary);
		if (savefile.is_open()) {
			savefile.write(reinterpret_cast<char*>(buffer), size);
			savefile.close();
			return true;
		}
		return false;
	}
};

