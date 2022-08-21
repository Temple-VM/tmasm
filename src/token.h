#ifndef TOKEN_H__HEADER_GUARD__
#define TOKEN_H__HEADER_GUARD__

#include <stdbool.h> /* bool, true, false */

#include "utils.h"
#include "location.h"
#include "colors.h"

typedef enum {
	TOKEN_TYPE_EOF = 0,
	TOKEN_TYPE_NEW_LINE,

	TOKEN_TYPE_COMMA,

	TOKEN_TYPE_HEX,
	TOKEN_TYPE_DEC,
	TOKEN_TYPE_CH,

	TOKEN_TYPE_STR,

	TOKEN_TYPE_LABEL,
	TOKEN_TYPE_ID,

	TOKEN_TYPE_INCLUDE,
	TOKEN_TYPE_DATA,

	TOKEN_TYPE_LPAREN,
	TOKEN_TYPE_RPAREN,

	TOKEN_TYPE_INT64_TYPE,
	TOKEN_TYPE_INT32_TYPE,
	TOKEN_TYPE_INT16_TYPE,
	TOKEN_TYPE_WORD_TYPE,
	TOKEN_TYPE_BYTE_TYPE,

	TOKEN_TYPE_INST_NONE,

	TOKEN_TYPE_INST_MOVE,

	TOKEN_TYPE_INST_WRITE, /* has size variants */
	TOKEN_TYPE_INST_READ, /* has size variants */

	TOKEN_TYPE_INST_PUSH, /* has size variants */
	TOKEN_TYPE_INST_PUSH_A,

	TOKEN_TYPE_INST_POP, /* has size variants */
	TOKEN_TYPE_INST_POP_A,

	TOKEN_TYPE_INST_EQ,
	TOKEN_TYPE_INST_NEQ,
	TOKEN_TYPE_INST_GT,
	TOKEN_TYPE_INST_GE,
	TOKEN_TYPE_INST_LT,
	TOKEN_TYPE_INST_LE,

	TOKEN_TYPE_INST_JUMP,
	TOKEN_TYPE_INST_JUMPT,
	TOKEN_TYPE_INST_JUMPF,

	TOKEN_TYPE_INST_ADD,
	TOKEN_TYPE_INST_INC,

	TOKEN_TYPE_INST_SUB,
	TOKEN_TYPE_INST_DEC,

	TOKEN_TYPE_INST_MULT,
	TOKEN_TYPE_INST_DIV,
	TOKEN_TYPE_INST_MOD,

	TOKEN_TYPE_INST_RSHIFT,
	TOKEN_TYPE_INST_LSHIFT,

	TOKEN_TYPE_INST_AND,
	TOKEN_TYPE_INST_OR,
	TOKEN_TYPE_INST_NOT,

	TOKEN_TYPE_INST_BITAND,
	TOKEN_TYPE_INST_BITOR,

	TOKEN_TYPE_INST_CALL,
	TOKEN_TYPE_INST_CALLT,
	TOKEN_TYPE_INST_CALLF,
	TOKEN_TYPE_INST_RET,

	TOKEN_TYPE_INST_SYSCALL,

	TOKEN_TYPE_INST_HALT,

	TOKEN_TYPE_REG_1,
	TOKEN_TYPE_REG_2,
	TOKEN_TYPE_REG_3,
	TOKEN_TYPE_REG_4,
	TOKEN_TYPE_REG_5,
	TOKEN_TYPE_REG_6,
	TOKEN_TYPE_REG_7,
	TOKEN_TYPE_REG_8,
	TOKEN_TYPE_REG_9,
	TOKEN_TYPE_REG_10,
	TOKEN_TYPE_REG_11,
	TOKEN_TYPE_REG_12,
	TOKEN_TYPE_REG_13,
	TOKEN_TYPE_REG_14,
	TOKEN_TYPE_REG_15,
	TOKEN_TYPE_REG_AC,
	TOKEN_TYPE_REG_IP,
	TOKEN_TYPE_REG_SP,
	TOKEN_TYPE_REG_SB,
	TOKEN_TYPE_REG_CN,
	TOKEN_TYPE_REG_EX
} token_type_t;

typedef struct {
	token_type_t type;
	char *data;

	location_t loc;
} token_t;

#ifdef TMASM_DEBUG
void token_dump(token_t *p_tok);
#endif

const char *token_type_to_str(token_type_t p_type);

bool token_type_is_inst(token_type_t p_type);
bool token_type_is_keyword(token_type_t p_type);
bool token_type_is_reg(token_type_t p_type);
bool token_type_is_type(token_type_t p_type);
bool token_type_is_value(token_type_t p_type);
bool token_type_is_arg(token_type_t p_type);

void token_free(token_t *p_tok);

#endif
