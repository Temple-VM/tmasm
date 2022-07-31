#ifndef LIST_H__HEADER_GUARD__
#define LIST_H__HEADER_GUARD__

/* no templates :( */

#include <stdlib.h>  /* size_t, malloc, realloc, free */
#include <string.h>  /* memset, memcpy */
#include <stdint.h>  /* uint8_t */
#include <assert.h>  /* assert */
#include <stdbool.h> /* bool, true, false */

#define LIST_CHUNK_SIZE 32

/* macros to make the usage of list_t easier */
#define LIST_AT(p_type, p_list, p_idx) ((p_type*)list_at(p_list, p_idx))

typedef struct {
	void  *buf;
	size_t buf_size, count, memb_size;
} list_t;

void list_init(list_t *p_list, size_t p_memb_size);

list_t list_copy(list_t *p_list);

void  list_add(list_t *p_list, const void *p_memb);
void  list_insert(list_t *p_list, size_t p_idx, void *p_start, size_t p_count);
void  list_remove(list_t *p_list, size_t p_idx, size_t p_count);
void *list_at(list_t *p_list, size_t p_idx);

void list_free(list_t *p_list);

#endif
