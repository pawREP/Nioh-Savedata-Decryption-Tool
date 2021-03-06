#include "stdafx.h"
#include "CryptoState.h"
#include "io.h"

extern "C" {
	#include "aes.h"
}

FILE_TYPE CryptoState::file_type = FILE_TYPE::USR;

void CryptoState::flip_32bit_endianness(unsigned char* arr) {
	unsigned char a, b, c, d;
	for (size_t i = 0; i < 4; i++) {
		a = arr[4 * i + 0];
		b = arr[4 * i + 1];
		c = arr[4 * i + 2];
		d = arr[4 * i + 3];
		arr[4 * i + 0] = d;
		arr[4 * i + 1] = c;
		arr[4 * i + 2] = b;
		arr[4 * i + 3] = a;
	}
}

CryptoState::CryptoState() : root_crypto_blob{
		0x54, 0x19, 0x31, 0x3E, 0xF4, 0x6B, 0xE4, 0x24, 0xCD, 0xA7, 0x96, 0x6F,	0xAB, 0xF0, 0x69, 0xCA,
		0x00, 0x00, 0x80, 0xBF, 0xEC, 0x3B, 0x9D, 0xA1, 0x46, 0x0C, 0xDD, 0x33,	0xF3, 0xD2, 0x58, 0xE0,
		0xC0, 0x9F, 0xC7, 0xD4, 0xF6, 0xEF, 0xDC, 0x70, 0x92, 0x6F, 0x52, 0xD8, 0xF1, 0xBD, 0x54, 0x36,
		0xA2, 0xCB, 0xAC, 0xA3, 0x99, 0xFC, 0xC8, 0xD2, 0xA9, 0x61, 0x72, 0x6B, 0xD7, 0x8D, 0x15, 0xB8,
		0x80, 0xAC, 0xA0, 0xB5, 0x9A, 0xA0, 0xEE, 0x1E, 0x8B, 0xF5, 0xD9, 0xDA, 0x2C, 0x92, 0xAE, 0xB4,
		0x9D, 0x92, 0xE0, 0x79, 0xAA, 0x76, 0x55, 0x31, 0xBC, 0xE3, 0x02, 0x00, 0x7A, 0xB9, 0x53, 0x7F,
		0xE2, 0x60, 0xF5, 0x26, 0x2B, 0x1E, 0x7D, 0xA7, 0x5D, 0xD1, 0xBD, 0x84, 0x23, 0x3B, 0xE4, 0x32,
		0x33, 0x03, 0xA4, 0x81, 0x84, 0x98, 0x97, 0xAB, 0x63, 0x7A, 0x82, 0x25, 0x39, 0x9F, 0xC0, 0x73,
		0x49, 0x63, 0x94, 0xFD, 0xD8, 0xDE, 0xA8, 0xC8, 0xB0, 0x36, 0x52, 0xCD, 0x07, 0xD6, 0xA2, 0x0A,
		0xF2, 0x00, 0x8C, 0x62
	} { }

void CryptoState::key_setup(DECRYPTION_TYPE type) {
	deconstruct_root_key_pair(type);

	switch (type) {
	case DECRYPTION_TYPE::HEADER:
		memcpy_s(s_key_1, BLOCK_SIZE, root_crypto_blob + 84, BLOCK_SIZE);
		memcpy_s(s_IV_1, BLOCK_SIZE, root_crypto_blob + 116, BLOCK_SIZE);
		memcpy_s(s_key_2, BLOCK_SIZE, root_crypto_blob + 100, BLOCK_SIZE);
		memcpy_s(s_IV_2, BLOCK_SIZE, root_crypto_blob + 132, BLOCK_SIZE);
		break;
	case DECRYPTION_TYPE::BODY:
		memcpy_s(s_key_1, BLOCK_SIZE, savedata_clear + 0x40, BLOCK_SIZE);
		memcpy_s(s_IV_1, BLOCK_SIZE, savedata_clear + 0x50, BLOCK_SIZE);
		memcpy_s(s_key_2, BLOCK_SIZE, savedata_clear + 0x60, BLOCK_SIZE);
		memcpy_s(s_IV_2, BLOCK_SIZE, savedata_clear + 0x70, BLOCK_SIZE);
		break;
	}

	unsigned char out[BLOCK_SIZE];
	AES_ECB_encrypt(root_IV, root_key, out, BLOCK_SIZE);
	for (int i = 0; i < BLOCK_SIZE; i++)
		s_key_1[i] ^= out[i];
	flip_32bit_endianness(s_key_1);

	AES_ECB_encrypt(root_IV, root_key, out, BLOCK_SIZE);
	for (int i = 0; i < BLOCK_SIZE; i++)
		s_IV_1[i] ^= out[i];

	AES_ECB_encrypt(s_IV_1, s_key_1, out, BLOCK_SIZE);
	for (int i = 0; i < BLOCK_SIZE; i++)
		s_key_2[i] ^= out[i];
	flip_32bit_endianness(s_key_2);

	AES_ECB_encrypt(s_IV_1, s_key_1, out, BLOCK_SIZE);
	for (int i = 0; i < BLOCK_SIZE; i++)
		s_IV_2[i] ^= out[i];


#ifdef DEBUG
	printf("s_key_1:");
	io::print_array(s_key_1, BLOCK_SIZE);
	printf("s_IV_1:");
	io::print_array(s_IV_1, BLOCK_SIZE);
	printf("s_key_2:");
	io::print_array(s_key_2, BLOCK_SIZE);
	printf("s_IV_2:");
	io::print_array(s_IV_2, BLOCK_SIZE);
	printf("\n");
#endif // DEBUG

}

void CryptoState::deconstruct_root_key_pair(DECRYPTION_TYPE type) {
	unsigned char buffer[32];
	unsigned char c1, c2, c3, c4, c5, c6, c7;

	switch (type) {
		case DECRYPTION_TYPE::HEADER:
			for (int i = 0; i<4; i++) {
				c1 = root_crypto_blob[8 + i];
				c2 = root_crypto_blob[0 + i];
				c3 = root_crypto_blob[12 + i];
				c4 = root_crypto_blob[4 + i];

				buffer[0 + i] = c1^root_crypto_blob[20 + i];
				buffer[4 + i] = c2^root_crypto_blob[24 + i];
				buffer[8 + i] = c3^root_crypto_blob[28 + i];
				buffer[12 + i] = c4^root_crypto_blob[32 + i];
				buffer[16 + i] = c1^root_crypto_blob[36 + i];
				buffer[20 + i] = c2^root_crypto_blob[40 + i];
				buffer[24 + i] = c3^root_crypto_blob[44 + i];
				buffer[28 + i] = c4^root_crypto_blob[48 + i];
			}
			memcpy_s(root_key, BLOCK_SIZE, buffer, BLOCK_SIZE);
			memcpy_s(root_IV, BLOCK_SIZE, buffer + BLOCK_SIZE, BLOCK_SIZE);

			break;
		case DECRYPTION_TYPE::BODY:
			for (int i = 0; i < 4; i++) {
				c1 = root_crypto_blob[0 + i];
				c2 = root_crypto_blob[8 + i];
				c3 = root_crypto_blob[12 + i];
				c4 = root_crypto_blob[56 + i];
				c5 = root_crypto_blob[52 + i];
				c6 = root_crypto_blob[60 + i];
				c7 = root_crypto_blob[4 + i];

				buffer[0 + i] = c2^root_crypto_blob[68 + i];
				buffer[4 + i] = c1^root_crypto_blob[72 + i];
				buffer[8 + i] = c3^root_crypto_blob[76 + i];
				buffer[12 + i] = c7^root_crypto_blob[80 + i];
				buffer[16 + i] = c2^c5;
				buffer[20 + i] = c1^c4;
				buffer[24 + i] = c3^c6;
				buffer[28 + i] = c7^root_crypto_blob[64 + i];
			}
			memcpy_s(root_IV, BLOCK_SIZE, buffer, BLOCK_SIZE);				//reverse order compared to header
			memcpy_s(root_key, BLOCK_SIZE, buffer + BLOCK_SIZE, BLOCK_SIZE);
			break;
	}
	flip_32bit_endianness(root_key);
	

#ifdef DEBUG
	printf("root key pair:");
	io::print_array(root_key, BLOCK_SIZE);
	io::print_array(root_IV, BLOCK_SIZE);
	printf("\n");
#endif // DEBUG
}

void CryptoState::incr_byte_array(unsigned char* arr, unsigned int incr) {
	unsigned char a = arr[BLOCK_SIZE - incr - 1];
	if (a == 0xFF) {
		arr[BLOCK_SIZE - incr - 1] = 0x00;
		incr_byte_array(arr, incr + 1);
	}
	else {
		arr[BLOCK_SIZE - incr - 1]++;
	}
}

bool CryptoState::crypt(unsigned char* cipher_text, unsigned char* clear_text) {
	savedata_encr = cipher_text;
	savedata_clear = clear_text;
	if (is_encrypted(savedata_encr)) {
		printf("Detected encrypted %s file. Starting decryption...   \n", file_type == FILE_TYPE::USR ? "NIOHUSR" : "NIOHSYS");
		key_setup(DECRYPTION_TYPE::HEADER);
		decrypt_header();
		//check if steam account is correct
		key_setup(DECRYPTION_TYPE::BODY); //header has to be decrypted before doing the body key setup. Key setup is the same for user and sys files.
		decrypt_body();

#ifndef PRESERVE_BODY_SUB_KEYS
		/*
		Sub keys for the save data body decryption are randomly generated,
		they don't have to be preserved. Setting them all to zero makes comparing
		different saves a bit easier.
		*/
		unsigned char zero[BLOCK_SIZE] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		memcpy_s(savedata_clear + 0x40, BLOCK_SIZE, zero, BLOCK_SIZE);
		memcpy_s(savedata_clear + 0x50, BLOCK_SIZE, zero, BLOCK_SIZE);
		memcpy_s(savedata_clear + 0x60, BLOCK_SIZE, zero, BLOCK_SIZE);
		memcpy_s(savedata_clear + 0x70, BLOCK_SIZE, zero, BLOCK_SIZE);
#endif // PRESERVE_BODY_SUB_KEYS

		return !is_encrypted(savedata_clear);
	} 
	else {
		printf("Detected decrypted %s file. Starting encryption...   \n", file_type == FILE_TYPE::USR ? "NIOHUSR" : "NIOHSYS");
		memcpy_s(savedata_clear, HEADER_SIZE, savedata_encr, HEADER_SIZE);
		key_setup(DECRYPTION_TYPE::BODY);
		decrypt_body();
		key_setup(DECRYPTION_TYPE::HEADER);
		decrypt_header();
		return true;
	}

}

bool CryptoState::is_encrypted(unsigned char* buf) {
	unsigned char str_nioh[] = { 0x4E, 0x49, 0x4F, 0x48 };
	unsigned char s = 0;
	for (size_t i = 0; i < 4; i++) {
		s += buf[i] ^ str_nioh[i];
	}
	if (s == 0x00)
		return false;
	return true;
}

void CryptoState::decrypt_header() {

	unsigned char IV_local[BLOCK_SIZE];
	unsigned char out[BLOCK_SIZE];
	unsigned char tmp_clear[HEADER_SIZE]; //necessary because the block size doesn't divide header size evenly and de/encryption are in different orders

	unsigned int n_rounds = HEADER_SIZE / BLOCK_SIZE + 1;

	memcpy_s(IV_local, BLOCK_SIZE, s_IV_2, BLOCK_SIZE);
		for (int i = 0; i < n_rounds; i++) {
		AES_ECB_encrypt(IV_local, s_key_2, out, BLOCK_SIZE);
		incr_byte_array(IV_local);
		for (int j = 0; j < BLOCK_SIZE; j++) {
			tmp_clear[BLOCK_SIZE * i + j] = savedata_encr[BLOCK_SIZE * i + j] ^ out[j];
		}
	}

	memcpy_s(IV_local, BLOCK_SIZE, s_IV_1, BLOCK_SIZE);
	for (int i = 0; i < n_rounds; i++) {
		AES_ECB_encrypt(IV_local, s_key_1, out, BLOCK_SIZE);
		incr_byte_array(IV_local);
		for (int j = 0; j < BLOCK_SIZE; j++) {
			tmp_clear[BLOCK_SIZE * i + j] ^= out[j];
		}
	}

	memcpy_s(savedata_clear, HEADER_SIZE, tmp_clear, HEADER_SIZE);
}

void CryptoState::decrypt_body() {

	unsigned char IV_local[BLOCK_SIZE];
	unsigned char out[BLOCK_SIZE];

	unsigned int n_rounds;
	if (file_type == FILE_TYPE::USR) {
		n_rounds = USR_BODY_SIZE / BLOCK_SIZE;
	}
	else {
		n_rounds = SYS_BODY_SIZE / BLOCK_SIZE + 1;
	}

	memcpy_s(IV_local, BLOCK_SIZE, s_IV_2, BLOCK_SIZE);
	for (int i = 0; i < n_rounds; i++) {
		AES_ECB_encrypt(IV_local, s_key_2, out, BLOCK_SIZE);
		incr_byte_array(IV_local);
		for (int j = 0; j < BLOCK_SIZE; j++) {
			savedata_clear[BLOCK_SIZE * i + j + HEADER_SIZE] = savedata_encr[BLOCK_SIZE * i + j + HEADER_SIZE] ^ out[j];
		}
	}

	memcpy_s(IV_local, BLOCK_SIZE, s_IV_1, BLOCK_SIZE);
	for (int i = 0; i < n_rounds; i++) {
		AES_ECB_encrypt(IV_local, s_key_1, out, BLOCK_SIZE);
		incr_byte_array(IV_local);
		for (int j = 0; j < BLOCK_SIZE; j++) {
			savedata_clear[BLOCK_SIZE * i + j + HEADER_SIZE] ^= out[j];
		}
	}
}

int CryptoState::get_file_size() {
	switch (CryptoState::file_type) {
	case FILE_TYPE::USR:
		return HEADER_SIZE + USR_BODY_SIZE;
	case FILE_TYPE::SYS:
		return HEADER_SIZE + SYS_BODY_SIZE;
	}
}
