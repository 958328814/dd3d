#include "dd3d.h"
#include <openssl/rsa.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include "utils.h"
#include "config.h"

#include "crypt.h"

RSA* Rsa = NULL;

void crypt_init() {
	const char* private_key_file;
	if (CONFIG_TRUE == config_lookup_string(config_get(), "dd3d.private_key_file", &private_key_file)) {
		//Read private key
		FILE* fp_p = fopen(private_key_file, "r");
		if (fp_p) {
			Rsa = RSA_new();
			if (PEM_read_RSAPrivateKey(fp_p, &Rsa, NULL, NULL)) {

			} else {
				throw "Read private key failed.\n";
			}
		} else {
			throw "Open private key file failed.\n";
		}
	} else
		throw "Private key file path is missing..\n";
}

void crypt_cleanup() {
	RSA_free(Rsa);
}

int crypt_rsa_private_encrypt(uint8_t* in, uint32_t in_size, uint8_t out[RSA_SIZE]) {
	return RSA_private_encrypt(in_size, in, out, Rsa, RSA_PKCS1_PADDING);	
}

int crypt_rsa_private_decrypt(uint8_t* in, uint32_t in_size, uint8_t out[RSA_SIZE]) {
	return RSA_private_decrypt(in_size, in, out, Rsa, RSA_PKCS1_PADDING);	
}
