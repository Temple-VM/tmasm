#include "list.h"

void list_init(list_t *p_list, size_t p_memb_size) {
	memset(p_list, 0, sizeof(list_t));

	p_list->memb_size = p_memb_size;
	p_list->buf_size  = LIST_CHUNK_SIZE;
	p_list->buf       = malloc(p_list->buf_size * p_list->memb_size);
	if (p_list->buf == NULL)
		assert(0 && "malloc fail");
}

list_t list_copy(list_t *p_list) {
	list_t list = {
		.buf_size  = p_list->buf_size,
		.count     = p_list->count,
		.memb_size = p_list->memb_size
	};

	list.buf = malloc(list.buf_size * list.memb_size);
	if (list.buf == NULL)
		assert(0 && "malloc fail");

	memcpy(list.buf, p_list->buf, list.count * list.memb_size);

	return list;
}

void list_add(list_t *p_list, const void *p_element) {
	/* copy the entire element at the end of the buffer */
	memcpy(&((uint8_t*)p_list->buf)[p_list->count * p_list->memb_size],
	       p_element, p_list->memb_size);
	++ p_list->count;

	/* resize the buffer if needed */
	if (p_list->count >= p_list->buf_size) {
		p_list->buf_size *= 2;

		void *tmp = realloc(p_list->buf, p_list->buf_size * p_list->memb_size);
		if (tmp == NULL) {
			free(p_list->buf);

			assert(0 && "realloc fail");
		} else
			p_list->buf = tmp;
	}
}

void list_insert(list_t *p_list, size_t p_idx, void *p_start, size_t p_count) {
	assert(p_count != 0);
	assert(p_idx < p_list->count);

	size_t prev_count = p_list->count;
	p_list->count += p_count;

	/* add chunks until it is enough */
	while (p_list->buf_size < p_list->count)
		p_list->buf_size *= 2;

	/* for shorter code */
	uint8_t *byte_buf = (uint8_t*)p_list->buf;

	for (size_t i = prev_count - 1; i >= p_idx; -- i) {
		/* since we do not know the type of a single element, we have to move everything
		   byte by byte */
		for (size_t j = 0; j < p_list->memb_size; ++ j)
			byte_buf[(i + p_count) *
			         p_list->memb_size + j] = byte_buf[i * p_list->memb_size + j];
	}

	/* paste the new elements in */
	for (size_t i = 0; i < p_count * p_list->memb_size; ++ i)
		byte_buf[i + p_idx * p_list->memb_size] = ((uint8_t*)p_start)[i];
}

void list_remove(list_t *p_list, size_t p_idx, size_t p_count) {
	assert(p_count != 0);
	assert(p_idx < p_list->count);
	assert(p_idx + p_count <= p_list->count);

	size_t prev_count = p_list->count;
	p_list->count -= p_count;

	uint8_t *byte_buf = (uint8_t*)p_list->buf;

	for (size_t i = p_idx + p_count; i < prev_count; ++ i) {
		for (size_t j = 0; j < p_list->memb_size; ++ j)
			byte_buf[(i - p_count) * p_list->memb_size + j] = byte_buf[i * p_list->memb_size + j];
	}
}

void *list_at(list_t *p_list, size_t p_idx) {
	assert(p_idx < p_list->count);

	return &((uint8_t*)p_list->buf)[p_idx * p_list->memb_size];
}

void list_free(list_t *p_list) {
	free(p_list->buf);
}
