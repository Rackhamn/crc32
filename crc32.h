#ifndef CRC32_H
#define CRC32_H

#include "crc32_poly_table.h"
#include "endian.h"
#include "reverse_bits.h"

#define CRC32_SEED		0xFFFFFFFF
#define CRC32_POLY		0xEDB88320
// TODO: test against 	0x1EDC6F41

static
unsigned int crc32_table[256] = { 
	// GCC specific - otherwise 0
	#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) 
	CRC32_POLY_TABLE_256_LE
	#elif defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
	CRC32_POLY_TABLE_256_BE
	#else
	0
	#endif
};

// if we want to use the CRC32_POLY, then we dont need to call crc32_init
void crc32_init(unsigned int poly) {
	// if we already know the polynomial then just copy the table over
	if(poly == CRC32_POLY) {
		const unsigned int * table = is_little_endian() ? crc32_poly_table_256_le : crc32_poly_table_256_be;
		memcpy(crc32_table, table, sizeof(unsigned int) * 256);
		return;
	}

	// otherwise build the real table
	unsigned int entry;
	for(int i = 0; i < 256; i++) {
		entry = (unsigned int)i;
		
		for(int j = 0; j < 8; j++) {
			if((entry & 1) == 1) { entry = (entry >> 1) ^ poly; }
			else { entry >>= 1; }
			crc32_table[i] = entry;
		}
	}
	
	// since we generate the table in little endian order, 
	// we might have to reorder and reverse the table bits
	if(is_big_endian()) {
		// 1) reverse array order
		for(int i = 0; i < 128; i++) {
			// swap3
			unsigned int t = crc32_table[i];
			crc32_table[i] = crc32_table[256 - i];
			crc32_table[256 - i] = t;
		}
		
		// 2) reverse bits
		for(int i = 0; i < 256; i++) {
			crc32_table[i] = reverse_u32(crc32_table[i]);
		}
	}
}

unsigned int crc32_hash(void * s) {
	unsigned int hash = CRC32_SEED;
	
	const unsigned char * ss = (unsigned char *)s;
	while(*ss) {
		hash = (hash >> 8) ^ crc32_table[*ss++ ^ (hash & 0xFF)];
	}
	
	return ~hash;
}

// size is in bytes
unsigned int crc32_hash_s(void * s, size_t size) {
	unsigned int hash = CRC32_SEED;
	
	const unsigned char * ss = (unsigned char *)s;
	while(size--)
		hash = (hash >> 8) ^ crc32_table[*ss++ ^ (hash & 0xFF)];
	
	return ~hash;
}

// start and size is in bytes
unsigned int crc32_hash_ss(void * s, size_t start, size_t size) {
	unsigned int hash = CRC32_SEED;
	
	const unsigned char * ss = (unsigned char *)s + start;
	while(size--)
		hash = (hash >> 8) ^ crc32_table[*ss++ ^ (hash & 0xFF)];
	
	return ~hash;
}

#endif /* CRC32_H */