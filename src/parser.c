#include "parser.h"

bool g_insts_have_size_variants[] = {
	false, /* TOKEN_TYPE_INST_NONE */

	false, /* TOKEN_TYPE_INST_MOVE */

	true, /* TOKEN_TYPE_INST_WRITE */
	true, /* TOKEN_TYPE_INST_READ */

	true,  /* TOKEN_TYPE_INST_PUSH */
	false, /* TOKEN_TYPE_INST_PUSH_A */

	true,  /* TOKEN_TYPE_INST_POP */
	false, /* TOKEN_TYPE_INST_POP_A */

	false, /* TOKEN_TYPE_INST_EQ */
	false, /* TOKEN_TYPE_INST_NEQ */
	false, /* TOKEN_TYPE_INST_GT */
	false, /* TOKEN_TYPE_INST_GE */
	false, /* TOKEN_TYPE_INST_LT */
	false, /* TOKEN_TYPE_INST_LE */

	false, /* TOKEN_TYPE_INST_JUMP */

	false, /* TOKEN_TYPE_INST_ADD */
	false, /* TOKEN_TYPE_INST_INC */

	false, /* TOKEN_TYPE_INST_SUB */
	false, /* TOKEN_TYPE_INST_DEC */

	false, /* TOKEN_TYPE_INST_MULT */
	false, /* TOKEN_TYPE_INST_DIV */
	false, /* TOKEN_TYPE_INST_MOD */

	false, /* TOKEN_TYPE_INST_RSHIFT */
	false, /* TOKEN_TYPE_INST_LSHIFT */

	false, /* TOKEN_TYPE_INST_AND */
	false, /* TOKEN_TYPE_INST_OR */
	false, /* TOKEN_TYPE_INST_NOT */

	false, /* TOKEN_TYPE_INST_BITAND */
	false, /* TOKEN_TYPE_INST_BITOR */

	false, /* TOKEN_TYPE_INST_CALL */
	false, /* TOKEN_TYPE_INST_RET */

	false, /* TOKEN_TYPE_INST_SYSCALL */

	false /* TOKEN_TYPE_INST_HALT */
};

bool inst_has_size_variant(token_type_t p_inst) {
	return g_insts_have_size_variants[p_inst - TOKEN_TYPE_INST_NONE];
}

parser_t parser_new(const char *p_path) {
	parser_t parser = {0};
	parser.lexer = lexer_new(p_path);

	parser.toks = lexer_lex(&parser.lexer);
	parser.tok  = parser.toks.buf;

	return parser;
}

void parser_free(parser_t *p_parser) {
	list_free(&p_parser->toks);
}

node_t *parser_parse_next(parser_t *p_parser) {
	if (p_parser->tok->type == TOKEN_TYPE_EOF)
		return NULL;

	switch (p_parser->tok->type) {
	case TOKEN_TYPE_LABEL:  return parser_parse_label(p_parser);
	case TOKEN_TYPE_DATA:   return parser_parse_data(p_parser);
	case TOKEN_TYPE_DEFINE: return parser_parse_define(p_parser);

	default:
		if (token_type_is_inst(p_parser->tok->type))
			return parser_parse_inst(p_parser);
	}

	error(&p_parser->tok->loc, "Unexpected token "QUOTES("%s"),
	      token_type_to_str(p_parser->tok->type));

	aborted();

	return NULL;
}

node_t *parser_parse_data(parser_t *p_parser) {
	node_t *node = node_new(p_parser->tok);

	parser_next_token(p_parser);

	if (p_parser->tok->type != TOKEN_TYPE_ID) {
		error(&p_parser->tok->loc, "Expected "QUOTES("%s")", got "QUOTES("%s"),
		      token_type_to_str(TOKEN_TYPE_ID), token_type_to_str(p_parser->tok->type));

		aborted();
	}

	node->left = node_new(p_parser->tok);

	parser_next_token(p_parser);

	if (!token_type_is_type(p_parser->tok->type)) {
		error(&p_parser->tok->loc, "Expected "QUOTES("%s")", got "QUOTES("%s"),
		      token_type_to_str(TOKEN_TYPE_WORD_TYPE), token_type_to_str(p_parser->tok->type));

		aborted();
	}

	switch (p_parser->tok->type) {
	case TOKEN_TYPE_INT64_TYPE: node->size = 64; break;
	case TOKEN_TYPE_INT32_TYPE: node->size = 32; break;
	case TOKEN_TYPE_INT16_TYPE: node->size = 16; break;
	case TOKEN_TYPE_BYTE_TYPE:  node->size = 8;  break;
	case TOKEN_TYPE_WORD_TYPE:  node->size = sizeof(word_t) * 8; break;

	default: INTERNAL_BUG;
	}

	parser_next_token(p_parser);

	if (!token_type_is_value(p_parser->tok->type)) {
		error(&p_parser->tok->loc, "Expected a value "
		      "("QUOTES("%s")", "QUOTES("%s")", "QUOTES("%s")", "QUOTES("%s")")"
		      ", got "QUOTES("%s"),
		      token_type_to_str(TOKEN_TYPE_HEX), token_type_to_str(TOKEN_TYPE_DEC),
		      token_type_to_str(TOKEN_TYPE_CH),token_type_to_str(TOKEN_TYPE_STR),
		      token_type_to_str(p_parser->tok->type));

		aborted();
	}

	node->right = node_new(p_parser->tok);

	parser_next_token(p_parser);

	if (!parser_at_end(p_parser)) {
		error(&p_parser->tok->loc, "Expected "QUOTES("%s")", got "QUOTES("%s"),
		      token_type_to_str(TOKEN_TYPE_NEW_LINE), token_type_to_str(p_parser->tok->type));

		aborted();
	}

	parser_next_token(p_parser);

	return node;
}

node_t *parser_parse_define(parser_t *p_parser) {
	node_t *node = node_new(p_parser->tok);

	parser_next_token(p_parser);

	if (p_parser->tok->type != TOKEN_TYPE_ID) {
		error(&p_parser->tok->loc, "Expected "QUOTES("%s")", got "QUOTES("%s"),
		      token_type_to_str(TOKEN_TYPE_ID), token_type_to_str(p_parser->tok->type));

		aborted();
	}

	node->left = node_new(p_parser->tok);

	parser_next_token(p_parser);

	if (!token_type_is_arg(p_parser->tok->type)) {
		error(&p_parser->tok->loc, "Expected a value "
		      "("QUOTES("%s")", "QUOTES("%s")", "QUOTES("%s")", "QUOTES("%s")")"
		      ", got "QUOTES("%s"),
		      token_type_to_str(TOKEN_TYPE_HEX), token_type_to_str(TOKEN_TYPE_DEC),
		      token_type_to_str(TOKEN_TYPE_CH),token_type_to_str(TOKEN_TYPE_REG_1),
		      token_type_to_str(p_parser->tok->type));

		aborted();
	}

	node->right = node_new(p_parser->tok);

	parser_next_token(p_parser);

	if (!parser_at_end(p_parser)) {
		error(&p_parser->tok->loc, "Expected "QUOTES("%s")", got "QUOTES("%s"),
		      token_type_to_str(TOKEN_TYPE_NEW_LINE), token_type_to_str(p_parser->tok->type));

		aborted();
	}

	parser_next_token(p_parser);

	return node;
}

node_t *parser_parse_inst(parser_t *p_parser) {
	node_t *node = node_new(p_parser->tok);

	parser_next_token(p_parser);

	if (token_type_is_type(p_parser->tok->type)) {
		if (inst_has_size_variant(node->tok->type)) {
			switch (p_parser->tok->type) {
			case TOKEN_TYPE_INT64_TYPE: node->size = 64; break;
			case TOKEN_TYPE_INT32_TYPE: node->size = 32; break;
			case TOKEN_TYPE_INT16_TYPE: node->size = 16; break;
			case TOKEN_TYPE_BYTE_TYPE:  node->size = 8;  break;
			case TOKEN_TYPE_WORD_TYPE:  node->size = sizeof(word_t) * 8; break;

			default: INTERNAL_BUG;
			}

			parser_next_token(p_parser);
		} else {
			error(&p_parser->tok->loc, "Unexpected "QUOTES("%s")" for instruction",
			      token_type_to_str(p_parser->tok->type));

			aborted();
		}
	}

	if (!parser_at_end(p_parser)) {
		if (!token_type_is_arg(p_parser->tok->type)) {
			error(&p_parser->tok->loc, "Unexpected token "QUOTES("%s"),
			      token_type_to_str(p_parser->tok->type));

			aborted();
		}

		node->left = node_new(p_parser->tok);

		parser_next_token(p_parser);

		if (p_parser->tok->type == TOKEN_TYPE_COMMA) {
			parser_next_token(p_parser);

			if (!token_type_is_arg(p_parser->tok->type)) {
				error(&p_parser->tok->loc, "Unexpected token "QUOTES("%s"),
				      token_type_to_str(p_parser->tok->type));

				aborted();
			}

			node->right = node_new(p_parser->tok);

			parser_next_token(p_parser);
		}
	}

	if (!parser_at_end(p_parser)) {
		error(&p_parser->tok->loc, "Expected "QUOTES("%s")", got "QUOTES("%s"),
		      token_type_to_str(TOKEN_TYPE_NEW_LINE), token_type_to_str(p_parser->tok->type));

		aborted();
	}

	parser_next_token(p_parser);

	return node;
}

node_t *parser_parse_label(parser_t *p_parser) {
	node_t *node = node_new(p_parser->tok);

	parser_next_token(p_parser);

	if (!parser_at_end(p_parser)) {
		error(&p_parser->tok->loc, "Expected "QUOTES("%s")", got "QUOTES("%s"),
		      token_type_to_str(TOKEN_TYPE_NEW_LINE), token_type_to_str(p_parser->tok->type));

		aborted();
	}

	parser_next_token(p_parser);

	return node;
}

void parser_next_token(parser_t *p_parser) {
	if (p_parser->tok->type == TOKEN_TYPE_EOF)
		return;

	++ p_parser->i;
	p_parser->tok = LIST_AT(token_t, &p_parser->toks, p_parser->i);
}

bool parser_at_end(parser_t *p_parser) {
	switch (p_parser->tok->type) {
	case TOKEN_TYPE_EOF:
	case TOKEN_TYPE_NEW_LINE: return true;

	default: return false;
	}
}

