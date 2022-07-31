#ifndef TOKEN_H__HEADER_GUARD__
#define TOKEN_H__HEADER_GUARD__

#include <stdlib.h>  /* size_t, malloc, realloc, free */
#include <stdio.h>   /* stdout, putchar, printf, fputs */
#include <assert.h>  /* assert */
#include <stdbool.h> /* bool, true, false */
#include <string.h>  /* memset */

#include "list.h"

typedef enum {
	TOKEN_TYPE_END = 0,

	TOKEN_TYPE_HEX,
	TOKEN_TYPE_DEC,
	TOKEN_TYPE_CHAR,
	TOKEN_TYPE_STR,
	TOKEN_TYPE_REG,

	TOKEN_TYPE_LABEL,
	TOKEN_TYPE_KEYWORD,
	TOKEN_TYPE_INST,
	TOKEN_TYPE_ID
} token_type_t;

typedef struct {
	token_type_t type;
	char        *data;

	char  *line;
	size_t row, col;
} token_t;

token_t     token_new(token_type_t p_type, char *p_data, char *p_line, size_t p_row, size_t p_col);
const char *token_type_to_str(token_t *p_token);

void tokens_free(list_t *p_list);
void tokens_dump(list_t *p_list); /* debug */

#endif
