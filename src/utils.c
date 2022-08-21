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
