#ifndef CRYPT_H_INCLUDED
#define CRYPT_H_INCLUDED

const uint32_t RSA_SIZE = 128;

void crypt_init();
void crypt_cleanup();
int crypt_rsa_private_encrypt(uint8_t* in, uint32_t in_size, uint8_t out[RSA_SIZE]);
int crypt_rsa_private_decrypt(uint8_t* in, uint32_t in_size, uint8_t out[RSA_SIZE]);

#endif
