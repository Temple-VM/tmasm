#include "utils.h"

void *memalloc(size_t p_size) {
	void *ptr = malloc(p_size);
	if (ptr == NULL)
		assert(0 && "malloc fail");

	return ptr;
}

void *memrealloc(void *p_ptr, size_t p_size) {
	void *ptr = realloc(p_ptr, p_size);
	if (ptr == NULL) {
		free(p_ptr);

		assert(0 && "realloc fail");
	} else
		return ptr;
}

void memfree(void **p_ptr) {
	free(*p_ptr);
	*p_ptr = NULL;
}

void *memalloccopy(const void *p_ptr, size_t p_size) {
	void *copy = memalloc(p_size);
	memcpy(copy, p_ptr, p_size);

	return copy;
}

char *copy_str(const char *p_str) {
	return (char*)memalloccopy(p_str, strlen(p_str) + 1);
}

void fwrite64_little_endian(uint64_t p_data, FILE *p_file) {
	uint8_t bytes[sizeof(uint64_t)] = {
        (p_data & 0xFF00000000000000) >> 070,
        (p_data & 0x00FF000000000000) >> 060,
        (p_data & 0x0000FF0000000000) >> 050,
        (p_data & 0x000000FF00000000) >> 040,
        (p_data & 0x00000000FF000000) >> 030,
        (p_data & 0x0000000000FF0000) >> 020,
        (p_data & 0x000000000000FF00) >> 010,
        (p_data & 0x00000000000000FF)
	};

	fwrite(bytes, 1, sizeof(bytes), p_file);
}
