#include "stdafx.h"
#include "io.h"
#include "SaveEditor.h"
#include <string>

bool argument_exists(std::string arg, int argc, char** argv, int& idx) {
	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == arg) {
			idx = i;
			return true;
		}
	}
	return false;
}

int main(int argc, char* argv[]){

	printf("----------------------------------------------\n");
	printf("Nioh Savedata decryption/encryption tool by B3.\n");
	printf("Tool version: 1.3\n");
	printf("Developed for game version 1.21.02\n");
	printf("\n");
	printf("Use \"-h\" for help.\n");
	printf("----------------------------------------------\n\n");

	if (argc == 1 || std::string(argv[1]) == "-h") {
		printf("The tool will automatically detect if the given input file is encrypted or decrypted.\n");
		printf("If no other inputs than the save file is given than only simple de/encryption is performed.\n\n");

		printf("Advanced options:\n");
		printf("-i [file_path]  : Path to Nioh input file\n");
		printf("-o [file_name]  : Output file name\n");
		printf("-cs             : Disables save file integrity checks. If this flag is set the save file will be modified in a way that prevents integrity checking at run time. Use this is you want to edit the save data. Only effective when decrypting USR save files. \n");
		//printf("-p	            : Preserves body sub keys in header. All body sub keys will be set to zero by default. Only effective when decrypting.\n");
		printf("-sid [SteamID3] : Transfers a encrypted save file to a given Steam ID.\n");
		printf("\n\nPress Enter To Exit...\n");
		std::cin.ignore();
		return 0;
	}

	SaveEditor* se;
	std::string file_prefix = "decr_";

	if(argc==2){
		se = new SaveEditor(std::string(argv[1]));
		se->store_save_file(std::string(argv[1]), file_prefix);
		printf("Success!\n\nPress Enter To Exit...\n");
		std::cin.ignore();
		return 0;
	}

	int idx = 0;

	if (argument_exists("-i", argc, argv, idx)) {
		se = new SaveEditor(std::string(argv[idx+1]));
	}
	else {
		printf("No input file given. Use \"-h\" for help. \n\nPress Enter To Exit...\n");
		std::cin.ignore();
		return 0;
	}

	std::string out_path = argv[idx + 1];
	if (argument_exists("-o", argc, argv, idx)) {
		out_path = std::string(argv[idx + 1]);
		file_prefix = "";
	}

	if (argument_exists("-cs", argc, argv, idx)) {
		//This test looks a bit odd because the file get's automatically decrypted in the SaveEditor constructor
		if (se->is_encrypted()) {
			printf("-cs can only be used on encrypted save files \n\nPress Enter To Exit...\n");
			std::cin.ignore();
			return 1;
		}
		se->set_disable_integrity_check_flags();
		printf("Disabled integrity checks.\n");
	}
	if (argument_exists("-sid", argc, argv, idx)) {
		//This test looks a bit odd because the file get's automatically decrypted in the SaveEditor constructor
		if (se->is_encrypted()) {
			printf("-sid can only be used on encrypted save files \n\nPress Enter To Exit...\n");
			std::cin.ignore();
			return 1;
		}
		int sid = std::stoi(std::string(argv[idx + 1]));
		se->transfer_to_steam_account(sid);
		se->encrypt();
		se->store_save_file(out_path, file_prefix);
		printf("Transfered save file to SteamID %d.\n",sid);
	}

	se->store_save_file(out_path, file_prefix);
	printf("Success! \n\nPress Enter To Exit...\n");
	
	std::cin.ignore();
	return 0;
}

