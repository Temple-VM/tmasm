#include "token.h"

#ifdef TMASM_DEBUG
void token_dump(token_t *p_tok) {
	fprintclrf(stdout, "[\x1by%s: \x1bn%s\x1bX], ", token_type_to_str(p_tok->type), p_tok->data);
}
#endif

const char *token_type_to_str(token_type_t p_type) {
	switch (p_type) {
	case TOKEN_TYPE_EOF:      return "end of file";
	case TOKEN_TYPE_NEW_LINE: return "new line";

	case TOKEN_TYPE_COMMA: return ",";

	case TOKEN_TYPE_HEX: return "hexadecimal";
	case TOKEN_TYPE_DEC: return "decimal";
	case TOKEN_TYPE_CH:  return "character";

	case TOKEN_TYPE_STR: return "string";

	case TOKEN_TYPE_LABEL: return "label";
	case TOKEN_TYPE_ID:    return "identifier";

	case TOKEN_TYPE_INCLUDE: return "include";
	case TOKEN_TYPE_DATA:    return "data";

	case TOKEN_TYPE_LPAREN: return "(";
	case TOKEN_TYPE_RPAREN: return ")";

	case TOKEN_TYPE_INT64_TYPE:
	case TOKEN_TYPE_INT32_TYPE:
	case TOKEN_TYPE_INT16_TYPE:
	case TOKEN_TYPE_WORD_TYPE:
	case TOKEN_TYPE_BYTE_TYPE: return "type";

	case TOKEN_TYPE_INST_NONE:
	case TOKEN_TYPE_INST_MOVE:
	case TOKEN_TYPE_INST_WRITE:
	case TOKEN_TYPE_INST_READ:
	case TOKEN_TYPE_INST_PUSH:
	case TOKEN_TYPE_INST_PUSH_A:
	case TOKEN_TYPE_INST_POP:
	case TOKEN_TYPE_INST_POP_A:
	case TOKEN_TYPE_INST_EQ:
	case TOKEN_TYPE_INST_NEQ:
	case TOKEN_TYPE_INST_GT:
	case TOKEN_TYPE_INST_GE:
	case TOKEN_TYPE_INST_LT:
	case TOKEN_TYPE_INST_LE:
	case TOKEN_TYPE_INST_JUMP:
	case TOKEN_TYPE_INST_JUMPT:
	case TOKEN_TYPE_INST_JUMPF:
	case TOKEN_TYPE_INST_ADD:
	case TOKEN_TYPE_INST_INC:
	case TOKEN_TYPE_INST_SUB:
	case TOKEN_TYPE_INST_DEC:
	case TOKEN_TYPE_INST_MULT:
	case TOKEN_TYPE_INST_DIV:
	case TOKEN_TYPE_INST_MOD:
	case TOKEN_TYPE_INST_RSHIFT:
	case TOKEN_TYPE_INST_LSHIFT:
	case TOKEN_TYPE_INST_AND:
	case TOKEN_TYPE_INST_OR:
	case TOKEN_TYPE_INST_NOT:
	case TOKEN_TYPE_INST_BITAND:
	case TOKEN_TYPE_INST_BITOR:
	case TOKEN_TYPE_INST_CALL:
	case TOKEN_TYPE_INST_CALLT:
	case TOKEN_TYPE_INST_CALLF:
	case TOKEN_TYPE_INST_RET:
	case TOKEN_TYPE_INST_SYSCALL:
	case TOKEN_TYPE_INST_HALT: return "instruction";

	case TOKEN_TYPE_REG_1:
	case TOKEN_TYPE_REG_2:
	case TOKEN_TYPE_REG_3:
	case TOKEN_TYPE_REG_4:
	case TOKEN_TYPE_REG_5:
	case TOKEN_TYPE_REG_6:
	case TOKEN_TYPE_REG_7:
	case TOKEN_TYPE_REG_8:
	case TOKEN_TYPE_REG_9:
	case TOKEN_TYPE_REG_10:
	case TOKEN_TYPE_REG_11:
	case TOKEN_TYPE_REG_12:
	case TOKEN_TYPE_REG_13:
	case TOKEN_TYPE_REG_14:
	case TOKEN_TYPE_REG_15:
	case TOKEN_TYPE_REG_AC:
	case TOKEN_TYPE_REG_IP:
	case TOKEN_TYPE_REG_SP:
	case TOKEN_TYPE_REG_SB:
	case TOKEN_TYPE_REG_CN:
	case TOKEN_TYPE_REG_EX: return "register";

	default: INTERNAL_BUG;
	}
}

bool token_type_is_inst(token_type_t p_type) {
	switch (p_type) {
	case TOKEN_TYPE_INST_NONE:
	case TOKEN_TYPE_INST_MOVE:
	case TOKEN_TYPE_INST_WRITE:
	case TOKEN_TYPE_INST_READ:
	case TOKEN_TYPE_INST_PUSH:
	case TOKEN_TYPE_INST_PUSH_A:
	case TOKEN_TYPE_INST_POP:
	case TOKEN_TYPE_INST_POP_A:
	case TOKEN_TYPE_INST_EQ:
	case TOKEN_TYPE_INST_NEQ:
	case TOKEN_TYPE_INST_GT:
	case TOKEN_TYPE_INST_GE:
	case TOKEN_TYPE_INST_LT:
	case TOKEN_TYPE_INST_LE:
	case TOKEN_TYPE_INST_JUMP:
	case TOKEN_TYPE_INST_JUMPT:
	case TOKEN_TYPE_INST_JUMPF:
	case TOKEN_TYPE_INST_ADD:
	case TOKEN_TYPE_INST_INC:
	case TOKEN_TYPE_INST_SUB:
	case TOKEN_TYPE_INST_DEC:
	case TOKEN_TYPE_INST_MULT:
	case TOKEN_TYPE_INST_DIV:
	case TOKEN_TYPE_INST_MOD:
	case TOKEN_TYPE_INST_RSHIFT:
	case TOKEN_TYPE_INST_LSHIFT:
	case TOKEN_TYPE_INST_AND:
	case TOKEN_TYPE_INST_OR:
	case TOKEN_TYPE_INST_NOT:
	case TOKEN_TYPE_INST_BITAND:
	case TOKEN_TYPE_INST_BITOR:
	case TOKEN_TYPE_INST_CALL:
	case TOKEN_TYPE_INST_CALLT:
	case TOKEN_TYPE_INST_CALLF:
	case TOKEN_TYPE_INST_RET:
	case TOKEN_TYPE_INST_SYSCALL:
	case TOKEN_TYPE_INST_HALT: return true;

	default: return false;
	}
}

bool token_type_is_keyword(token_type_t p_type) {
	switch (p_type) {
	case TOKEN_TYPE_INCLUDE:
	case TOKEN_TYPE_DATA:
	case TOKEN_TYPE_LPAREN:
	case TOKEN_TYPE_RPAREN: return true;

	default: return false;
	}
}

bool token_type_is_reg(token_type_t p_type) {
	switch (p_type) {
	case TOKEN_TYPE_REG_1:
	case TOKEN_TYPE_REG_2:
	case TOKEN_TYPE_REG_3:
	case TOKEN_TYPE_REG_4:
	case TOKEN_TYPE_REG_5:
	case TOKEN_TYPE_REG_6:
	case TOKEN_TYPE_REG_7:
	case TOKEN_TYPE_REG_8:
	case TOKEN_TYPE_REG_9:
	case TOKEN_TYPE_REG_10:
	case TOKEN_TYPE_REG_11:
	case TOKEN_TYPE_REG_12:
	case TOKEN_TYPE_REG_13:
	case TOKEN_TYPE_REG_14:
	case TOKEN_TYPE_REG_15:
	case TOKEN_TYPE_REG_AC:
	case TOKEN_TYPE_REG_IP:
	case TOKEN_TYPE_REG_SP:
	case TOKEN_TYPE_REG_SB:
	case TOKEN_TYPE_REG_CN:
	case TOKEN_TYPE_REG_EX: return true;

	default: return false;
	}
}
bool token_type_is_type(token_type_t p_type) {
	switch (p_type) {
	case TOKEN_TYPE_INT64_TYPE:
	case TOKEN_TYPE_INT32_TYPE:
	case TOKEN_TYPE_INT16_TYPE:
	case TOKEN_TYPE_WORD_TYPE:
	case TOKEN_TYPE_BYTE_TYPE: return true;

	default: return false;
	}
}

bool token_type_is_value(token_type_t p_type) {
	switch (p_type) {
	case TOKEN_TYPE_HEX:
	case TOKEN_TYPE_DEC:
	case TOKEN_TYPE_CH:
	case TOKEN_TYPE_STR: return true;

	default: return false;
	}
}

bool token_type_is_arg(token_type_t p_type) {
	switch (p_type) {
	case TOKEN_TYPE_HEX:
	case TOKEN_TYPE_DEC:
	case TOKEN_TYPE_CH:
	case TOKEN_TYPE_ID: return true;

	default: return token_type_is_reg(p_type);
	}
}

void token_free(token_t *p_tok) {
	free(p_tok->data);
	free(p_tok->loc.line);
	free(p_tok->loc.file);
}
