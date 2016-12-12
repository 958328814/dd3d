#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

bool utils_bin2hex(uint8_t *src, uint32_t src_size, char* dst, uint32_t dst_size);

void utils_get_uuid(uint8_t out[16]);

uint32_t utils_hash(const uint8_t *data, int len, uint32_t hash);

const uint32_t XXTEA_KEY_SIZE = 16;
void utils_xxtea_encrypt(uint8_t *data, size_t data_size, uint8_t key[XXTEA_KEY_SIZE]);
void utils_xxtea_decrypt(uint8_t *data, size_t data_size, uint8_t key[XXTEA_KEY_SIZE]);

template<typename ReturnType, typename PointerType>
ReturnType utils_poniter_calc(PointerType p, int offset) {
	return reinterpret_cast<ReturnType>(
		reinterpret_cast<uint8_t*>(p) + offset		
	);
}

void utils_bin_dump(void* bin, uint32_t buffer_size);

#endif
