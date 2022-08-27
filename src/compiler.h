#ifndef COMPILER_H__HEADER_GUARD__
#define COMPILER_H__HEADER_GUARD__

/* TODO: there is a lot of code repetition, a function for each instruction. maybe shorten it
         by making a function for each operand case and reuse that function

	for example:
		jump &R1
		call &R1

	these two instructions have the same operands, so instead make a function to handle a single
	register operand and run it for these two instructions
 */

#include <stdint.h> /* uint8_t */
#include <string.h> /* strcmp */

#ifdef TMASM_DEBUG
#	include <stdio.h> /* puts */
#endif

#include "utils.h"
#include "list.h"
#include "parser.h"

#define ENTRY_LABEL "entry"

typedef struct {
	char  *name;
	size_t addr;
} label_t;

typedef struct {
	char  *name;
	size_t addr;
} data_const_t;

typedef struct {
	char   *name;
	node_t *node;
} macro_t;

typedef struct {
	const char *path;

	parser_t parser;

	node_t *node;

	word_t addr;

	    /* node_t */
	list_t nodes;

	    /* inst_fix_t */
	list_t insts_fix;

	  /* uint8_t inst_t*/
	list_t data, insts;

	    /* label_t data_const_t macro_t*/
	list_t labels, data_consts, macros;
} compiler_t;

typedef struct {
	bool is_reg;

	union {
		word_t num;
		reg_t  reg;
	} value;
} arg_any_t;

MAKE_PAIR_T_FUNC(inst_func, token_type_t, inst_t, compiler_t*, size_t);

extern PAIR_T(inst_func) g_inst_func_map[];

compiler_t compiler_new(const char *p_path);

void compiler_compile(compiler_t *p_compiler, const char *p_output_path);

void compiler_generate_tm_file(compiler_t *p_compiler, const char *p_path);

void compiler_compile_data(compiler_t *p_compiler);
void compiler_compile_define(compiler_t *p_compiler);
void compiler_compile_inst(compiler_t *p_compiler);
void compiler_compile_label(compiler_t *p_compiler);

inst_t compiler_compile_inst_none(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_move(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_write(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_read(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_push(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_pusha(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_pop(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_popa(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_eq(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_neq(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_gt(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_ge(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_lt(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_le(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_jump(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_jumpt(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_jumpf(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_add(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_inc(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_sub(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_dec(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_mult(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_div(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_mod(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_rshift(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_lshift(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_and(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_or(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_not(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_bitand(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_bitor(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_call(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_callt(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_callf(compiler_t *p_compiler, size_t p_argc);
inst_t compiler_compile_inst_ret(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_syscall(compiler_t *p_compiler, size_t p_argc);

inst_t compiler_compile_inst_halt(compiler_t *p_compiler, size_t p_argc);

word_t    compiler_get_num_arg(compiler_t *p_compiler, node_t *p_node, const char *p_inst);
reg_t     compiler_get_reg_arg(compiler_t *p_compiler, node_t *p_node, const char *p_inst);
arg_any_t compiler_get_any_arg(compiler_t *p_compiler, node_t *p_node, const char *p_inst);

void label_free(label_t *p_label);
void data_const_free(data_const_t *p_data_const);
void macro_free(macro_t *p_macro);

#endif
