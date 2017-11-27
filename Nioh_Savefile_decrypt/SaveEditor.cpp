#include "stdafx.h"
#include "SaveEditor.h"
#include "io.h"
#include <string>


SaveEditor::SaveEditor(std::string& file_path)
{
	SaveEditor::load_save_file(file_path);
	cs.crypt(savedata_encr, savedata_clear);
}

SaveEditor::~SaveEditor()
{
	delete[] savedata_clear;
	delete[] savedata_encr;
}

void SaveEditor::load_save_file(std::string& file_path){
	switch (io::get_file_size(file_path)) {
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

	savedata_clear = new unsigned char[CryptoState::get_file_size() + BLOCK_SIZE];
	savedata_encr = new unsigned char[CryptoState::get_file_size() + BLOCK_SIZE];

	if(!io::parse_savefile(file_path, savedata_encr, CryptoState::get_file_size())) {
		printf("Failed to read file. \n\nPress Enter To Exit...\n");
		std::cin.ignore();
		exit(1);
	}
}

void SaveEditor::encrypt() {
	unsigned char* tmp = savedata_clear;
	savedata_clear = savedata_encr;
	savedata_encr = tmp;
	cs.crypt(savedata_encr, savedata_clear);
}

void SaveEditor::store_save_file(std::string& file_path, std::string prefix) {
	io::save_decrypted_savefile(file_path, savedata_clear, CryptoState::get_file_size(), prefix);
}

void SaveEditor::transfer_to_steam_account(int& SteamID3) {
	int steamID_offset = 0x10;
	*reinterpret_cast<int*>(savedata_clear + steamID_offset) = SteamID3;
}

void SaveEditor::set_disable_integrity_check_flags() {
	if(!is_encrypted()){
		*(savedata_clear + 0x16938D) = 0;
		*(savedata_clear + 0x16934C) = 0;
		*(savedata_clear + 0x169390) = 0;
		*(savedata_clear + 0x1693B4) = 0;
		*(savedata_clear + 0x1693BF) = 0;
		*(savedata_clear + 0x17CE54) = 0;
		*(savedata_clear + 0x1DE8DC) = 0;
	}
}

bool SaveEditor::is_encrypted() {
	return cs.is_encrypted(savedata_clear);
}
