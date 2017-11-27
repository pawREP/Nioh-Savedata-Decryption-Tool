#pragma once
#include "CryptoState.h"
#include <string>

class SaveEditor
{
private:
	CryptoState cs;
	unsigned char* savedata_encr;
	unsigned char* savedata_clear;

	void load_save_file(std::string& file_path);
public:
	SaveEditor(std::string& file_path);
	~SaveEditor();

	void transfer_to_steam_account(int& SteamID3);
	void set_disable_integrity_check_flags();
	void store_save_file(std::string& file_path, std::string prefix = "");
	bool is_encrypted();
	void encrypt();
};

