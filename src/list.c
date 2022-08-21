#include "list.h"

list_t list_new(size_t p_memb_size, void (*p_free_func)(void*)) {
	return (list_t){
		.buf       = memalloc(LIST_CHUNK_SIZE * p_memb_size),
		.buf_size  = LIST_CHUNK_SIZE,
		.count     = 0,
		.memb_size = p_memb_size,
		.free_func = p_free_func
	};
}

list_t list_copy(list_t *p_list) {
	list_t list = {
		.buf_size  = p_list->buf_size,
		.count     = p_list->count,
		.memb_size = p_list->memb_size
	};

	list.buf = memalloc(list.buf_size * list.memb_size);

	memcpy(list.buf, p_list->buf, list.count * list.memb_size);

	return list;
}

void list_push(list_t *p_list, const void *p_memb) {
	/* copy the entire element to the end of the buffer */
	memcpy(&((uint8_t*)p_list->buf)[p_list->count * p_list->memb_size], p_memb, p_list->memb_size);
	++ p_list->count;

	/* resize the buffer if needed */
	if (p_list->count >= p_list->buf_size) {
		p_list->buf_size *= 2;

		p_list->buf = memrealloc(p_list->buf, p_list->buf_size * p_list->memb_size);
	}
}

void list_pop(list_t *p_list) {
	if (p_list->free_func != NULL)
		p_list->free_func(list_at(p_list, p_list->count - 1));

	-- p_list->count;
}

void list_insert(list_t *p_list, size_t p_idx, void *p_start, size_t p_count) {
	assert(p_count != 0);
	assert(p_idx <= p_list->count);

	size_t prev_count = p_list->count;
	p_list->count += p_count;

	/* add chunks until it is enough */
	size_t prev_buf_size = p_list->buf_size;
	while (p_list->buf_size < p_list->count)
		p_list->buf_size *= 2;

	if (prev_buf_size != p_list->buf_size)
		p_list->buf = memrealloc(p_list->buf, p_list->buf_size * p_list->memb_size);

	/* for shorter code */
	uint8_t *byte_buf = (uint8_t*)p_list->buf;

	for (size_t i = prev_count - (prev_count == 0? 0 : 1); i >= p_idx; -- i) {
		/* since we do not know the type of a single element, we have to move everything
		   byte by byte */
		for (size_t j = 0; j < p_list->memb_size; ++ j)
			byte_buf[(i + p_count) *
			         p_list->memb_size + j] = byte_buf[i * p_list->memb_size + j];

		if (i == 0) /* prevent i wrapping around */
			break;
	}

	/* paste the new elements in */
	for (size_t i = 0; i < p_count * p_list->memb_size; ++ i)
		byte_buf[i + p_idx * p_list->memb_size] = ((uint8_t*)p_start)[i];
}

void list_remove(list_t *p_list, size_t p_idx, size_t p_count) {
	assert(p_count != 0);
	assert(p_idx < p_list->count);
	assert(p_idx + p_count <= p_list->count);

	uint8_t *byte_buf = (uint8_t*)p_list->buf;

	if (p_list->free_func != NULL) {
		for (size_t i = p_idx; i < p_idx + p_count; ++ i)
			p_list->free_func(list_at(p_list, i));
	}

	size_t prev_count = p_list->count;
	p_list->count -= p_count;

	for (size_t i = p_idx + p_count; i < prev_count; ++ i) {
		for (size_t j = 0; j < p_list->memb_size; ++ j)
			byte_buf[(i - p_count) * p_list->memb_size + j] = byte_buf[i * p_list->memb_size + j];
	}
}

void *list_at(list_t *p_list, size_t p_idx) {
	assert(p_idx < p_list->count);

	return &((uint8_t*)p_list->buf)[p_idx * p_list->memb_size];
}

void *list_end(list_t *p_list) {
	if (p_list->count == 0)
		return NULL;
	else
		return list_at(p_list, p_list->count - 1);
}

void list_free(list_t *p_list) {
	if (p_list->free_func != NULL) {
		for (size_t i = 0; i < p_list->count; ++ i)
			p_list->free_func(list_at(p_list, i));
	}

	free(p_list->buf);
}
