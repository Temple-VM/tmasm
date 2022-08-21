#ifndef LIST_H__HEADER_GUARD__
#define LIST_H__HEADER_GUARD__

/* no templates :( */

#include <stdlib.h>  /* size_t, malloc, realloc, free */
#include <string.h>  /* memset, memcpy */
#include <stdint.h>  /* uint8_t */
#include <assert.h>  /* assert */
#include <stdbool.h> /* bool, true, false */

#include "utils.h"

#define LIST_CHUNK_SIZE 32

/* macros to make the usage of list_t easier */
#define LIST_AT(p_type, p_list, p_idx) ((p_type*)list_at(p_list, p_idx))
#define LIST_END(p_type, p_list)       ((p_type*)list_end(p_list))
#define LIST_PUSH(p_type, p_list, p_val) \
	{ \
		p_type to_push = p_val; \
		list_push(p_list, &to_push); \
	}

#define LIST_FREE_FUNC(p_func) ((void (*)(void*))(p_func))

typedef struct {
	void  *buf;
	size_t buf_size, count, memb_size;

	void (*free_func)(void*);
} list_t;

list_t list_new(size_t p_memb_size, void (*p_free_func)(void*));
list_t list_copy(list_t *p_list);

void list_push(list_t *p_list, const void *p_memb);
void list_pop(list_t *p_list);

void  list_insert(list_t *p_list, size_t p_idx, void *p_start, size_t p_count);
void  list_remove(list_t *p_list, size_t p_idx, size_t p_count);
void *list_at(list_t *p_list, size_t p_idx);
void *list_end(list_t *p_list);

void list_free(list_t *p_list);

#endif
