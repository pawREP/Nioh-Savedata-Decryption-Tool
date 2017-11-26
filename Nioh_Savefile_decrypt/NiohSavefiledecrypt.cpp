#include "stdafx.h"
#include "io.h"
#include "CryptoState.h"
#include <string>

int main(int argc, char* argv[]){

	if (argc == 1) {
		printf("No input file specified. \n\nPress Enter To Exit...\n");
		std::cin.ignore();
		exit(1);
	}

	switch (io::get_file_size(argv[1])) {
	case HEADER_SIZE + USR_BODY_SIZE:
		CryptoState::file_type = FILE_TYPE::USR;
		break;
	case HEADER_SIZE + SYS_BODY_SIZE:
		CryptoState::file_type = FILE_TYPE::SYS;
		break;
	default:
		printf("Wrong file size. \n\nPress Enter To Exit...\n");
		std::cin.ignore();
		exit(1);
	}

	unsigned char* savedata_encr  = new unsigned char[CryptoState::get_file_size() + BLOCK_SIZE]; //the extra block is to deal with file blocks that are not 16 byte aligned.
	unsigned char* savedata_clear = new unsigned char[CryptoState::get_file_size() + BLOCK_SIZE];

	printf("----------------------------------------------\n");
	printf("Nioh Savedata decryption/encryption tool by B3.\n");
	printf("Tool version: 1.1 - No body sub key conservation branch\n");
	printf("Developed for game version 1.21.02\n");
	printf("----------------------------------------------\n\n");

	if (!io::parse_savefile(argv[1], savedata_encr, CryptoState::get_file_size())) {
		printf("Failed to read file. \n\nPress Enter To Exit...\n");
		std::cin.ignore();
		exit(1);
	}

	CryptoState cs;
	if (!cs.decrypt(savedata_encr, savedata_clear)) {
		printf("\n\nDecryption failed. \n\nPress Enter To Exit...\n");
	}
	else {
		io::save_decrypted_savefile(std::string(argv[1]), savedata_clear, CryptoState::get_file_size());
		printf("success! \n\nPress Enter To Exit...\n");
	}

	delete[] savedata_encr;
	delete[] savedata_clear;

	std::cin.ignore();
	return 0;
}

