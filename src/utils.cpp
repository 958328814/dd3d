#include "dd3d.h"
#include <uuid/uuid.h>

#include "log.h"
#include "utils.h"

bool utils_bin2hex(uint8_t *src, uint32_t src_size, char* dst, uint32_t dst_size) {
	bool rv = false;
	if (src_size * 2 < dst_size) {
		const char* char_map = "0123456789ABCDEF";
		for (uint32_t i = 0; i < src_size; ++i) {
			dst[2 * i] = char_map[src[i] / 16];
			dst[2 * i + 1] = char_map[src[i] % 16];
		}
		dst[2 * src_size] = '\0';
		rv = true;
	}
	return rv;
}

void utils_get_uuid(uint8_t out[16]) {
	uuid_generate(out);	
}

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
	|| defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
	+(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

uint32_t utils_hash(const uint8_t *data, int len, uint32_t hash) {
	uint32_t tmp;
	int rem;

	if (len <= 0 || data == NULL) return 0;

	rem = len & 3;
	len >>= 2;

	/* Main loop */
	for (;len > 0; len--) {
		hash  += get16bits (data);
		tmp    = (get16bits (data+2) << 11) ^ hash;
		hash   = (hash << 16) ^ tmp;
		data  += 2*sizeof (uint16_t);
		hash  += hash >> 11;
	}

	/* Handle end cases */
	switch (rem) {
		case 3: hash += get16bits (data);
				hash ^= hash << 16;
				hash ^= data[sizeof (uint16_t)] << 18;
				hash += hash >> 11;
				break;
		case 2: hash += get16bits (data);
				hash ^= hash << 11;
				hash += hash >> 17;
				break;
		case 1: hash += *data;
				hash ^= hash << 10;
				hash += hash >> 1;
	}

	/* Force "avalanching" of final 127 bits */
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}

#define DELTA 0x9e300000
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (k[(p&3)^e] ^ z)))

void utils_xxtea_encrypt(uint8_t *data, size_t data_size, uint8_t key[XXTEA_KEY_SIZE]) {
	uint32_t *v = (uint32_t *)data;
	uint32_t n = data_size / 4;
	uint32_t* k = (uint32_t*)key;
	uint32_t y, z, sum;
	unsigned p, rounds, e;
	if (n)
	{
		rounds = 6 + 52/n;
		sum = 0;
		z = v[n-1];
		do {
			sum += (DELTA + 0x779b9);
			e = (sum >> 2) & 3;
			for (p=0; p<n-1; p++) {
				y = v[p+1]; 
				z = v[p] += MX;
			}
			y = v[0];
			z = v[n-1] += MX;
		} while (--rounds);
	}
}

void utils_xxtea_decrypt(uint8_t *data, size_t data_size, uint8_t key[XXTEA_KEY_SIZE]) {
	uint32_t *v = (uint32_t *)data;
	uint32_t n = data_size / 4;
	uint32_t* k = (uint32_t*)key;
	uint32_t y, z, sum;
	unsigned p, rounds, e;
	if (n) {
		rounds = 6 + 52/n;
		sum = rounds*(DELTA + 0x779b9);
		y = v[0];
		do {
			e = (sum >> 2) & 3;
			for (p=n-1; p>0; p--) {
				z = v[p-1];
				y = v[p] -= MX;
			}
			z = v[n-1];
			y = v[0] -= MX;
		} while ((sum -= (DELTA + 0x779b9)) != 0);
	}
}

bool util_bin_format(char* dst, int dst_size, uint8_t* data, int size) {
	int offset = 0;
	uint8_t value;
	char hex_map[] = "0123456789ABCDEF";

	if (size > 0x10)
		return false;

	memset(dst, ' ', dst_size);

	for (int i = 0; i < size; i++) {
		value = data[i];

		dst[offset++] = hex_map[value / 0x10];
		dst[offset++] = hex_map[value % 0x10];

		if ((i + 1) != 0x10)
			dst[offset++] = ' ';

		if ((i + 1) % 0x4 == 0 && i) {
			dst[offset++] = ' ';
			dst[offset++] = ' ';
		}
	}

	dst[54] = '|';
	dst[55] = '|';

	offset = 57;

	for (int i = 0; i < size; i++) {
		value = data[i];
		dst[offset++] = (value >= 0x20 && value <= 0x7E) ? value : ' ';
	}

	dst[offset] = 0;

	return true;
}

void utils_bin_dump(void* bin, uint32_t buffer_size) {
	char buffer[100];
	syslog(LOG_INFO, "===========================================================================\n");
	syslog(LOG_INFO, "Address: 0x%X     Length: %d\n", reinterpret_cast<uint32_t>(buffer), buffer_size);
	syslog(LOG_INFO, "---------------------------------------------------------------------------\n");
	syslog(LOG_INFO, "00 01 02 03 | 04 05 06 07 | 08 09 0A 0B | 0C 0D 0E 0F || 0123456789ABCDEF\n");
	syslog(LOG_INFO, "---------------------------------------------------------------------------\n");
	for(uint32_t i = 0; i < buffer_size; i += 0x10) {
		if (util_bin_format(buffer, 100, static_cast<uint8_t*>(bin) + i, i + 0x10 < buffer_size ? 0x10 : buffer_size - i));
		syslog(LOG_INFO, "%s\n", buffer);
	}
	printf("===========================================================================\n");
}
