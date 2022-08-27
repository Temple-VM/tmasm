#ifndef NODE_H__HEADER_GUARD__
#define NODE_H__HEADER_GUARD__

#include <stdint.h> /* uint64_t */
#include <stdlib.h> /* size_t, strtol */
#include <string.h> /* memset */

#include "utils.h"
#include "token.h"
#include "list.h"
#include "colors.h"

/* TODO: make each node have 2 CHILDREN */

typedef enum {
	NODE_TYPE_INST,
	NODE_TYPE_DATA,
	NODE_TYPE_DEFINE,
	NODE_TYPE_LABEL,

	NODE_TYPE_REG,
	NODE_TYPE_NUM,
	NODE_TYPE_STR,
	NODE_TYPE_ID,

	NODE_TYPE_INT64_TYPE,
	NODE_TYPE_INT32_TYPE,
	NODE_TYPE_INT16_TYPE,
	NODE_TYPE_BYTE_TYPE,
	NODE_TYPE_WORD_TYPE,
} node_type_t;

typedef struct node {
	node_type_t type;

	size_t size; /* size of the data - 8bit, 16bit, 32bit, 64bit */
	union {
		list_t   data; /* for single/multi byte arrays */
		uint64_t num;
	} data;

	struct node *left, *right;

	token_t *tok;
} node_t;

#ifdef TMASM_DEBUG
void node_dump(node_t *p_node);
#endif

const char *node_type_to_str(node_type_t p_type);

node_t *node_new(token_t *p_tok);
void    node_free_tree(node_t *p_node);

#endif
