#ifndef LEXER_H__HEADER_GUARD__
#define LEXER_H__HEADER_GUARD__

#include <stdlib.h>  /* size_t, malloc, realloc, free */
#include <stdio.h>   /* FILE, fopen, fclose, fgetc */
#include <stdarg.h>  /* va_list, va_start, va_end, vsnprintf */
#include <string.h>  /* strncpy, strcmp */
#include <ctype.h>   /* isalpha, isalnum, isdigit */
#include <assert.h>  /* assert */
#include <stdbool.h> /* bool, true, false */

#include "error.h"
#include "token.h"
#include "target.h"

#define CASE_SEPARATORS \
	     '(': case ')': -- p_lexer->col; /* fall through */  \
	case ' ': case '\t': case '\v': case '\f': case '\r' \


typedef struct {
	const char *path;
	FILE       *file;

	list_t tokens;

	list_t lines;
	char  *line;
	size_t line_len, row, col;

	token_t token;
	char   *token_str;
	size_t  token_buf_size, token_len;
} lexer_t;

extern const char *g_keywords[];

lexer_t lex(const char *p_path);

void lexer_free_lines(lexer_t *p_lexer);

bool lexer_line(lexer_t *p_lexer);
bool lexer_token(lexer_t *p_lexer);

void lexer_skip_comment(lexer_t *p_lexer);
void lexer_str(lexer_t *p_lexer);
void lexer_char(lexer_t *p_lexer);
void lexer_dec(lexer_t *p_lexer);
void lexer_hex(lexer_t *p_lexer);
void lexer_word(lexer_t *p_lexer);

void lexer_add_to_token(lexer_t *p_lexer, char p_ch);

void lexer_fatal(lexer_t *p_lexer, const char *p_fmt, ...);

#endif
