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

#define __MAKE_PAIR(p_id, p_key, p_value) \
	typedef struct { \
		p_key; \
		p_value; \
	} pair_##p_id##_t

#define MAKE_PAIR_T(p_id, p_key_type, p_value_type) \
	__MAKE_PAIR(p_id, p_key_type key, p_value_type value)

#define MAKE_PAIR_T_FUNC(p_id, p_key_type, p_return_type, ...) \
	__MAKE_PAIR(p_id, p_key_type key, p_return_type (*value)(__VA_ARGS__))

#define PAIR_T(p_id) pair_##p_id##_t

void *memalloc(size_t p_size);
void *memrealloc(void *p_ptr, size_t p_size);
void  memfree(void **p_ptr);

void *memalloccopy(const void *p_ptr, size_t p_size);

char *copy_str(const char *p_str);

void fwrite64_little_endian(uint64_t p_data, FILE *p_file);

#endif
