#ifndef INTERMEDIATE_H__HEADER_GUARD__
#define INTERMEDIATE_H__HEADER_GUARD__

/* an AST is not needed */

#include <stdio.h>   /* stderr, fputs, FILE, fopen, fclose */
#include <stdlib.h>  /* malloc, realloc, free */
#include <string.h>  /* memset, strcmp */
#include <stdarg.h>  /* va_list, va_start, va_end, vsnprintf */
#include <assert.h>  /* assert */
#include <stdbool.h> /* bool, true, false */
#include <stdint.h>  /* uint8_t */

#include "error.h"
#include "list.h"
#include "token.h"
#include "lexer.h"
#include "target.h"

/* TODO: add data segments */

#define MAX_COMPILER_ERRS   8
#define PROGRAM_ENTRY_LABEL "entry"

#define CASE_TOKEN_TYPE_NUM \
	     TOKEN_TYPE_HEX:  case TOKEN_TYPE_DEC: \
	case TOKEN_TYPE_CHAR: case TOKEN_TYPE_ID

/* TODO: prevent name conflicts with labels and macros */

typedef struct {
	const char *name;

	list_t tokens;

	char  *line;
	size_t row, col;
} macro_t;

typedef struct {
	const char *name;

	word_t inst_pos;
} label_t;

typedef struct {
	list_t macros;
	list_t labels;
	list_t insts; /* compiled instructions */

	word_t entry_point;

	const char *path;
	list_t      tokens;

	token_t *token;
	size_t   i;

	size_t err_count;
} compiler_t;

typedef struct {
	const char *name;
	void      (*func)(compiler_t*, size_t);
} inst_func_map_t;

extern inst_func_map_t g_insts_map[];

void compile(const char *p_source_path, const char *p_out_path);

void compiler_free_macros(compiler_t *p_compiler);

void compiler_write_file(compiler_t *p_compiler, const char *p_out_path);
void compiler_write_word_bytes(FILE *p_file, word_t p_data);

void compiler_macros(compiler_t *p_compiler);
void compiler_labels(compiler_t *p_compiler);
void compiler_main(compiler_t *p_compiler);
void compiler_inst(compiler_t *p_compiler);
void compiler_macro_definition(compiler_t *p_compiler);
void compiler_expand_macro(compiler_t *p_compiler);

void compiler_next_token(compiler_t *p_compiler);

void compiler_push_inst(compiler_t *p_compiler, opcode_t p_opcode, reg_t p_reg, word_t p_data);

reg_t  compiler_token_to_reg(compiler_t *p_compiler);
word_t compiler_token_to_num(compiler_t *p_compiler);

void compiler_inst_move(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_write64(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_write32(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_write16(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_write8(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_read64(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_read32(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_read16(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_read8(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_push64(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_push32(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_push16(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_push8(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_pusha(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_pop64(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_pop32(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_pop16(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_pop8(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_popa(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_eq(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_neq(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_gt(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_ge(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_lt(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_le(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_jump(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_jumpt(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_jumpf(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_add(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_inc(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_sub(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_dec(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_mult(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_div(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_mod(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_call(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_callt(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_callf(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_ret(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_writef(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_memset(compiler_t *p_compiler, size_t p_argc);
void compiler_inst_memcopy(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_debug(compiler_t *p_compiler, size_t p_argc);

void compiler_inst_halt(compiler_t *p_compiler, size_t p_argc);

void compiler_error(compiler_t *p_compiler, const char *p_fmt, ...);
void compiler_error_at_prev(compiler_t *p_compiler, token_t *p_prev, const char *p_fmt, ...);
void compiler_next_error_fatal(compiler_t *p_compiler);

#endif
