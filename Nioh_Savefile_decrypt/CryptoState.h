#pragma once
#define BLOCK_SIZE 0x10
#define ROOT_CRYPTO_BLOB_SIZE 148
#define HEADER_SIZE 0x148
#define USR_BODY_SIZE 0x1F2C50 
#define SYS_BODY_SIZE 0x2518
//#define SAVEDATA_SIZE 0x1F2D98

enum DECRYPTION_TYPE { HEADER, BODY };
enum FILE_TYPE { USR, SYS };

class CryptoState {
public:
	static FILE_TYPE file_type;

private:
	unsigned char root_crypto_blob[ROOT_CRYPTO_BLOB_SIZE];
	unsigned char root_key[BLOCK_SIZE];
	unsigned char root_IV[BLOCK_SIZE];
	unsigned char s_key_1[BLOCK_SIZE];
	unsigned char s_key_2[BLOCK_SIZE];
	unsigned char s_IV_1[BLOCK_SIZE];
	unsigned char s_IV_2[BLOCK_SIZE];

	unsigned char* savedata_encr;
	unsigned char* savedata_clear;

public:
	CryptoState(unsigned char* root_crypto_blob);
	bool decrypt(unsigned char* cipher_text, unsigned char* clear_text);
	static int get_file_size();
private:
	void decrypt_header();
	void decrypt_body();
	void key_setup(DECRYPTION_TYPE type);
	void deconstruct_root_key_pair(DECRYPTION_TYPE type);
	void incr_byte_array(unsigned char* arr, unsigned int incr = 0);
	void flip_32bit_endianness(unsigned char* arr);
	bool is_encrypted(unsigned char* buf);
};

