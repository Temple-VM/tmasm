#ifndef UTILS_H__HEADER_GUARD__
#define UTILS_H__HEADER_GUARD__

#include <assert.h> /* assert */
#include <stdlib.h> /* size_t, malloc, realloc, free */
#include <string.h> /* memcpy, strlen */
#include <stdint.h> /* uint64_t */
#include <stdio.h>  /* FILE, fwrite */

#define INTERNAL_BUG \
	assert(0 && "If this shows, it is an internal bug, please report it")

#define SMEMALLOC(p_ptr, p_size)   (p_ptr = memalloc(sizeof(p_ptr[0]) * p_size))
#define SMEMREALLOC(p_ptr, p_size) (p_ptr = memrealloc(p_ptr, sizeof(p_ptr[0]) * p_size))
#define SMEMFREE(p_ptr)            (memfree((void**)&p_ptr))

#define SIZE_OF(p_ptr) (sizeof(p_ptr) / sizeof(p_ptr[0]))

typedef struct {
	void *key, *value;
} pair_t;

void *memalloc(size_t p_size);
void *memrealloc(void *p_ptr, size_t p_size);
void  memfree(void **p_ptr);

void *memalloccopy(const void *p_ptr, size_t p_size);

char *copy_str(const char *p_str);

void fwrite64_little_endian(uint64_t p_data, FILE *p_file);

#endif
