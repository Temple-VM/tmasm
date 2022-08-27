#ifndef LEXER_H__HEADER_GUARD__
#define LEXER_H__HEADER_GUARD__

#include <stdlib.h>  /* size_t */
#include <stdio.h>   /* FILE, fopen, fclose */
#include <stdbool.h> /* bool, true, false */
#include <string.h>  /* strcmp, strlen */
#include <ctype.h>   /* isdigit, isxdigit, isalpha, isspace, tolower */

#include "error.h"
#include "list.h"
#include "utils.h"
#include "token.h"

#define IS_SEPARATOR(p_ch) (p_ch == ',' || p_ch == '(' || p_ch == ')')

typedef struct {
	const char *path;
	FILE       *stream;

	char   ch;
	char   line[512];
	size_t line_len;

	char   data[256];
	size_t data_len;

	location_t loc;
	token_t    tok;
	   /* token_t */
	list_t toks;
} lexer_t;

MAKE_PAIR_T(keyword, const char*, token_type_t);

extern PAIR_T(keyword) g_keywords[];
extern PAIR_T(keyword) g_regs[];

lexer_t lexer_new(const char *p_path);
list_t  lexer_lex(lexer_t *p_lexer);

bool lexer_next_line(lexer_t *p_lexer);
void lexer_lex_line(lexer_t *p_lexer);

void lexer_lex_hex(lexer_t *p_lexer);
void lexer_lex_dec(lexer_t *p_lexer);
void lexer_lex_ch(lexer_t *p_lexer);

void lexer_lex_str(lexer_t *p_lexer);

void lexer_lex_id(lexer_t *p_lexer);
void lexer_lex_reg(lexer_t *p_lexer);

void lexer_lex_skip_comment(lexer_t *p_lexer);

void lexer_push_token(lexer_t *p_lexer, token_type_t p_type);

void lexer_add_ch(lexer_t *p_lexer);
void lexer_token_start_here(lexer_t *p_lexer);

bool lexer_at_line_end(lexer_t *p_lexer);
void lexer_next_ch(lexer_t *p_lexer);
void lexer_prev_ch(lexer_t *p_lexer);

#endif
