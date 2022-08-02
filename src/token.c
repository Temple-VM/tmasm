#include "token.h"

token_t token_new(token_type_t p_type, char *p_data, char *p_line, size_t p_row, size_t p_col) {
	return (token_t){
		.type = p_type,
		.data = p_data,
		.line = p_line,
		.row  = p_row,
		.col  = p_col
	};
}

const char *token_type_to_str(token_t *p_token) {
	switch (p_token->type) {
	case TOKEN_TYPE_END: return "statement end"; break;

	case TOKEN_TYPE_HEX:  return "hexadecimal"; break;
	case TOKEN_TYPE_DEC:  return "decimal";     break;
	case TOKEN_TYPE_CHAR: return "character";   break;
	case TOKEN_TYPE_STR:  return "string";      break;
	case TOKEN_TYPE_REG:  return "register";    break;

	case TOKEN_TYPE_LPAREN: return "("; break;
	case TOKEN_TYPE_RPAREN: return ")"; break;

	case TOKEN_TYPE_LABEL:   return "label";       break;
	case TOKEN_TYPE_KEYWORD: return "keyword";     break;
	case TOKEN_TYPE_INST:    return "instruction"; break;
	case TOKEN_TYPE_ID:      return "identifier";  break;

	default: return "?";
	}
}

void tokens_free(list_t *p_list) {
	for (size_t i = 0; i < p_list->count; ++ i) {
		token_t *token = LIST_AT(token_t, p_list, i);

		if (token->data != NULL)
			free(token->data);
	}

	free(p_list->buf);
}

void tokens_dump(list_t *p_list) {
	for (size_t i = 0; i < p_list->count; ++ i) {
		token_t *token = LIST_AT(token_t, p_list, i);

		if (i > 0 && i % 8 == 0)
			putchar('\n');

		putchar('[');

		fputs(token_type_to_str(token), stdout);

		if (token->data != NULL) {
			if (token->type == TOKEN_TYPE_END)
				printf(", \n%s\n", token->data);
			else
				printf(", %s", token->data);
		}

		fputs("], ", stdout);
	}

	putchar('\n');
}
