#include "lexer.h"

PAIR_T(keyword) g_keywords[] = {
	{.key = "include", .value = TOKEN_TYPE_INCLUDE},
	{.key = "define",  .value = TOKEN_TYPE_DEFINE},
	{.key = "data",    .value = TOKEN_TYPE_DATA},

	{.key = "INT64", .value = TOKEN_TYPE_INT64_TYPE},
	{.key = "INT32", .value = TOKEN_TYPE_INT32_TYPE},
	{.key = "INT16", .value = TOKEN_TYPE_INT16_TYPE},
	{.key = "WORD",  .value = TOKEN_TYPE_WORD_TYPE},
	{.key = "BYTE",  .value = TOKEN_TYPE_BYTE_TYPE},

	{.key = "none", .value = TOKEN_TYPE_INST_NONE},

	{.key = "move", .value = TOKEN_TYPE_INST_MOVE},

	{.key = "write", .value = TOKEN_TYPE_INST_WRITE},

	{.key = "read",  .value = TOKEN_TYPE_INST_READ},

	{.key = "push",  .value = TOKEN_TYPE_INST_PUSH},
	{.key = "pushA", .value = TOKEN_TYPE_INST_PUSH_A},

	{.key = "pop",  .value = TOKEN_TYPE_INST_POP},
	{.key = "popA", .value = TOKEN_TYPE_INST_POP_A},

	{.key = "eq",  .value = TOKEN_TYPE_INST_EQ},
	{.key = "neq", .value = TOKEN_TYPE_INST_NEQ},
	{.key = "gt",  .value = TOKEN_TYPE_INST_GT},
	{.key = "ge",  .value = TOKEN_TYPE_INST_GE},
	{.key = "lt",  .value = TOKEN_TYPE_INST_LT},
	{.key = "le",  .value = TOKEN_TYPE_INST_LE},

	{.key = "jump",  .value = TOKEN_TYPE_INST_JUMP},
	{.key = "jumpt", .value = TOKEN_TYPE_INST_JUMPT},
	{.key = "jumpf", .value = TOKEN_TYPE_INST_JUMPF},

	{.key = "add", .value = TOKEN_TYPE_INST_ADD},
	{.key = "inc", .value = TOKEN_TYPE_INST_INC},

	{.key = "sub", .value = TOKEN_TYPE_INST_SUB},
	{.key = "dec", .value = TOKEN_TYPE_INST_DEC},

	{.key = "mult", .value = TOKEN_TYPE_INST_MULT},
	{.key = "div",  .value = TOKEN_TYPE_INST_DIV},
	{.key = "mod",  .value = TOKEN_TYPE_INST_MOD},

	{.key = "rshift", .value = TOKEN_TYPE_INST_RSHIFT},
	{.key = "lshift", .value = TOKEN_TYPE_INST_LSHIFT},

	{.key = "and", .value = TOKEN_TYPE_INST_AND},
	{.key = "or",  .value = TOKEN_TYPE_INST_OR},
	{.key = "not", .value = TOKEN_TYPE_INST_NOT},

	{.key = "bitand", .value = TOKEN_TYPE_INST_BITAND},
	{.key = "bitor",  .value = TOKEN_TYPE_INST_BITOR},

	{.key = "call",  .value = TOKEN_TYPE_INST_CALL},
	{.key = "callt", .value = TOKEN_TYPE_INST_CALLT},
	{.key = "callf", .value = TOKEN_TYPE_INST_CALLF},
	{.key = "ret",   .value = TOKEN_TYPE_INST_RET},

	{.key = "syscall", .value = TOKEN_TYPE_INST_SYSCALL},

	{.key = "halt", .value = TOKEN_TYPE_INST_HALT}
};

PAIR_T(keyword) g_regs[] = {
	{.key = "R1",  .value = TOKEN_TYPE_REG_1},
	{.key = "R2",  .value = TOKEN_TYPE_REG_2},
	{.key = "R3",  .value = TOKEN_TYPE_REG_3},
	{.key = "R4",  .value = TOKEN_TYPE_REG_4},
	{.key = "R5",  .value = TOKEN_TYPE_REG_5},
	{.key = "R6",  .value = TOKEN_TYPE_REG_6},
	{.key = "R7",  .value = TOKEN_TYPE_REG_7},
	{.key = "R8",  .value = TOKEN_TYPE_REG_8},
	{.key = "R9",  .value = TOKEN_TYPE_REG_9},
	{.key = "R10", .value = TOKEN_TYPE_REG_10},
	{.key = "R11", .value = TOKEN_TYPE_REG_11},
	{.key = "R12", .value = TOKEN_TYPE_REG_12},
	{.key = "R13", .value = TOKEN_TYPE_REG_13},
	{.key = "R14", .value = TOKEN_TYPE_REG_14},
	{.key = "R15", .value = TOKEN_TYPE_REG_15},

	{.key = "AC", .value = TOKEN_TYPE_REG_AC},
	{.key = "IP", .value = TOKEN_TYPE_REG_IP},
	{.key = "SP", .value = TOKEN_TYPE_REG_SP},
	{.key = "SB", .value = TOKEN_TYPE_REG_SB},
	{.key = "CN", .value = TOKEN_TYPE_REG_CN},
	{.key = "EX", .value = TOKEN_TYPE_REG_EX}
};

lexer_t lexer_new(const char *p_path) {
	lexer_t lexer = {0};
	lexer.toks     = list_new(sizeof(token_t), LIST_FREE_FUNC(token_free));
	lexer.path     = p_path;
	lexer.loc.line = NULL;
	lexer.loc.file = (char*)p_path;

	lexer.stream = fopen(p_path, "r");
	if (lexer.stream == NULL)
		fatal("Could not open file "QUOTES("%s"), p_path);

	return lexer;
}

list_t lexer_lex(lexer_t *p_lexer) {
	/* skip the shebang if there is one */
	p_lexer->ch = fgetc(p_lexer->stream);
	if (p_lexer->ch == '#') {
		while (fgetc(p_lexer->stream) != '\n');

		++ p_lexer->loc.row;
	} else
		ungetc(p_lexer->ch, p_lexer->stream);

	bool at_end = false;
	while (!at_end) {
		at_end = lexer_next_line(p_lexer);
		lexer_lex_line(p_lexer);
	}

	lexer_token_start_here(p_lexer);
	p_lexer->data[0] = EOF;
	lexer_push_token(p_lexer, TOKEN_TYPE_EOF);

	if (p_lexer->loc.line != NULL)
		free(p_lexer->loc.line);

	fclose(p_lexer->stream);

	if (p_lexer->toks.count == 0)
		fatal("at %s: Empty source files are forbidden", p_lexer->path);

	return p_lexer->toks;
}

bool lexer_next_line(lexer_t *p_lexer) {
	bool at_end = false;

	++ p_lexer->loc.row;
	p_lexer->loc.col = 1;

	p_lexer->line_len = 0;
	if (p_lexer->loc.line != NULL)
		free(p_lexer->loc.line);

	memset(p_lexer->line, 0, sizeof(p_lexer->line));

	/* read the entire next line into a buffer (the error messages need an entire line) */
	char ch;
	while ((ch = fgetc(p_lexer->stream)) != '\n') {
		if (ch == EOF) {
			at_end = true;

			break;
		}

		p_lexer->line[p_lexer->line_len ++] = ch;

		if (p_lexer->line_len + 1 >= sizeof(p_lexer->line)) {
			fatal("Line at %s:%lu exceeds max line length %i",
			      p_lexer->path, (unsigned long)p_lexer->tok.loc.row, sizeof(p_lexer->line));
		}
	}
	p_lexer->line[p_lexer->line_len] = '\0';

	/* copy the line for errors and replace \n/EOF with a NULL terminator */
	p_lexer->loc.line = copy_str(p_lexer->line);
	p_lexer->ch       = *p_lexer->line; /* jump to the first char */

	return at_end;
}

void lexer_lex_line(lexer_t *p_lexer) {
	for (; !lexer_at_line_end(p_lexer); lexer_next_ch(p_lexer)) {
		switch (p_lexer->ch) {
		case ',':
			lexer_token_start_here(p_lexer);
			lexer_add_ch(p_lexer);
			lexer_push_token(p_lexer, TOKEN_TYPE_COMMA);

			break;

		case '(':
			lexer_token_start_here(p_lexer);
			lexer_add_ch(p_lexer);
			lexer_push_token(p_lexer, TOKEN_TYPE_LPAREN);

			break;

		case ')':

			lexer_token_start_here(p_lexer);
			lexer_add_ch(p_lexer);
			lexer_push_token(p_lexer, TOKEN_TYPE_RPAREN);

			break;

		case ';':  lexer_lex_skip_comment(p_lexer); break;
		case '"':  lexer_lex_str(p_lexer); break;
		case '\'': lexer_lex_ch(p_lexer);  break;
		case '&':  lexer_lex_reg(p_lexer); break;

		case '0':
			lexer_next_ch(p_lexer);
			if (lexer_at_line_end(p_lexer)) {
				lexer_prev_ch(p_lexer);
				lexer_lex_dec(p_lexer);

				break;
			}

			if (tolower(p_lexer->ch) == 'x')
				lexer_lex_hex(p_lexer);
			else {
				lexer_prev_ch(p_lexer);
				lexer_lex_dec(p_lexer);
			}

			break;

		default:
			if (isspace(p_lexer->ch))
				break;
			else if (isdigit(p_lexer->ch))
				lexer_lex_dec(p_lexer);
			else if (isalpha(p_lexer->ch) || p_lexer->ch == '_')
				lexer_lex_id(p_lexer);
			else {
				error(&p_lexer->loc, "Unexpected character "QUOTES("%c"), p_lexer->ch);

				aborted();
			}
		}
	}

	if (p_lexer->toks.count > 0) {
		if (LIST_END(token_t, &p_lexer->toks)->type != TOKEN_TYPE_NEW_LINE) {
			lexer_token_start_here(p_lexer);
			p_lexer->data[0] = '\n';
			lexer_push_token(p_lexer, TOKEN_TYPE_NEW_LINE);
		}
	}
}

void lexer_lex_hex(lexer_t *p_lexer) {
	lexer_token_start_here(p_lexer);
	lexer_next_ch(p_lexer);

	for (; !lexer_at_line_end(p_lexer); lexer_next_ch(p_lexer)) {
		if (isspace(p_lexer->ch))
			break;
		else if (IS_SEPARATOR(p_lexer->ch)) {
			lexer_prev_ch(p_lexer);
			break;
		} else if (!isxdigit(p_lexer->ch)) {
			if (isalpha(p_lexer->ch)) {
				error(&p_lexer->loc, "Expected a hexadecimal digit, got "QUOTES("%c"), p_lexer->ch);

				aborted();
			} else {
				lexer_prev_ch(p_lexer);

				break;
			}
		}

		lexer_add_ch(p_lexer);
	}

	if (p_lexer->data_len == 0) {
		error(&p_lexer->loc, "Expected a hexadecimal digit");

		aborted();
	}

	lexer_push_token(p_lexer, TOKEN_TYPE_HEX);
}

void lexer_lex_dec(lexer_t *p_lexer) {
	lexer_token_start_here(p_lexer);

	for (; !lexer_at_line_end(p_lexer); lexer_next_ch(p_lexer)) {
		if (isspace(p_lexer->ch))
			break;
		else if (IS_SEPARATOR(p_lexer->ch)) {
			lexer_prev_ch(p_lexer);
			break;
		} else if (!isdigit(p_lexer->ch)) {
			if (isalpha(p_lexer->ch)) {
				error(&p_lexer->loc, "Expected a decimal digit, got "QUOTES("%c"), p_lexer->ch);

				aborted();
			} else {
				lexer_prev_ch(p_lexer);

				break;
			}
		}

		lexer_add_ch(p_lexer);
	}

	lexer_push_token(p_lexer, TOKEN_TYPE_DEC);
}

void lexer_lex_ch(lexer_t *p_lexer) {
	lexer_token_start_here(p_lexer);
	lexer_next_ch(p_lexer);

	switch (p_lexer->ch) {
	case '\\':
		lexer_next_ch(p_lexer);

		switch(p_lexer->ch) {
		case '\\': p_lexer->data[0] = '\\'; break;
		case '0':  p_lexer->data[0] = '\0'; break;
		case 'a':  p_lexer->data[0] = '\a'; break;
		case 'b':  p_lexer->data[0] = '\b'; break;
		case 'f':  p_lexer->data[0] = '\f'; break;
		case 'n':  p_lexer->data[0] = '\n'; break;
		case 'r':  p_lexer->data[0] = '\r'; break;
		case 't':  p_lexer->data[0] = '\t'; break;
		case 'v':  p_lexer->data[0] = '\v'; break;
		case 'e':  p_lexer->data[0] = 27;   break;

		default:
			error(&p_lexer->loc, "Unknown escape sequence "QUOTES("\\%c"), p_lexer->ch);

			aborted();
		}

		lexer_push_token(p_lexer, TOKEN_TYPE_CH);

		break;

	case '\'':
		error(&p_lexer->loc, "Empty character literals are forbidden");

		aborted();

		break;
	default:
		lexer_add_ch(p_lexer);
		lexer_push_token(p_lexer, TOKEN_TYPE_CH);
	}

	lexer_next_ch(p_lexer);
	if (p_lexer->ch != '\'') {
		error(&p_lexer->loc, "Character literal length has to be equal to 1");

		aborted();
	}
}

void lexer_lex_str(lexer_t *p_lexer) {
	lexer_token_start_here(p_lexer);
	lexer_next_ch(p_lexer);

	bool escape = false;
	for (; !lexer_at_line_end(p_lexer); lexer_next_ch(p_lexer)) {
		switch (p_lexer->ch) {
		case '"':
			if (escape)
				lexer_add_ch(p_lexer);
			else {
				lexer_push_token(p_lexer, TOKEN_TYPE_STR);

				return;
			}

			break;

		case '\\':
			if (escape)
				lexer_add_ch(p_lexer);

			escape = !escape;

			break;

		default:
			if (escape) {
				switch (p_lexer->ch) {
				case '\\': p_lexer->ch = '\\'; break;
				case '0':  p_lexer->ch = '\0'; break;
				case 'a':  p_lexer->ch = '\a'; break;
				case 'b':  p_lexer->ch = '\b'; break;
				case 'f':  p_lexer->ch = '\f'; break;
				case 'n':  p_lexer->ch = '\n'; break;
				case 'r':  p_lexer->ch = '\r'; break;
				case 't':  p_lexer->ch = '\t'; break;
				case 'v':  p_lexer->ch = '\v'; break;
				case 'e':  p_lexer->ch = 27;   break;

				default:
					error(&p_lexer->loc, "Unknown escape sequence "QUOTES("\\%c"), p_lexer->ch);

					aborted();
				}

				escape = false;
			}

			lexer_add_ch(p_lexer);
		}
	}

	error(&p_lexer->loc,    "String exceeds line, expected matching "QUOTES("%c"), '"');
	note(&p_lexer->tok.loc, "Opened here");

	aborted();
}

void lexer_lex_id(lexer_t *p_lexer) {
	lexer_token_start_here(p_lexer);

	for (; !lexer_at_line_end(p_lexer); lexer_next_ch(p_lexer)) {
		switch (p_lexer->ch) {
		case ':': lexer_push_token(p_lexer, TOKEN_TYPE_LABEL); return;

		case '(':
		case ')': lexer_prev_ch(p_lexer); goto finish;

		default:
			if (isspace(p_lexer->ch))
				goto finish;
			else if (IS_SEPARATOR(p_lexer->ch)) {
				lexer_prev_ch(p_lexer);
				goto finish;
			} else if (!isalnum(p_lexer->ch) && p_lexer->ch != '_') {
				error(&p_lexer->loc, "Unexpected character "QUOTES("%c")" in identifier",
				      p_lexer->ch);

				aborted();
			}
		}

		lexer_add_ch(p_lexer);
	}

finish:
	for (size_t i = 0; i < sizeof(g_keywords) / sizeof(g_keywords[0]); ++ i) {
		if (strcmp((char*)g_keywords[i].key, p_lexer->data) == 0) {
			lexer_push_token(p_lexer, (token_type_t)g_keywords[i].value);

			return;
		}
	}

	lexer_push_token(p_lexer, TOKEN_TYPE_ID);
}

void lexer_lex_reg(lexer_t *p_lexer) {
	lexer_token_start_here(p_lexer);

	for (lexer_next_ch(p_lexer); !lexer_at_line_end(p_lexer); lexer_next_ch(p_lexer)) {
		switch (p_lexer->ch) {
		case '(':
		case ')': lexer_prev_ch(p_lexer); goto finish;

		default:
			if (isspace(p_lexer->ch))
				goto finish;
			else if (IS_SEPARATOR(p_lexer->ch)) {
				lexer_prev_ch(p_lexer);
				goto finish;
			} else if (!isalnum(p_lexer->ch) && p_lexer->ch != '_') {
				error(&p_lexer->loc, "Unexpected character "QUOTES("%c")" in identifier",
				      p_lexer->ch);

				aborted();
			}
		}

		lexer_add_ch(p_lexer);
	}

finish:
	for (size_t i = 0; i < sizeof(g_regs) / sizeof(g_regs[0]); ++ i) {
		if (strcmp((char*)g_regs[i].key, p_lexer->data) == 0) {
			lexer_push_token(p_lexer, (token_type_t)g_regs[i].value);

			return;
		}
	}

	p_lexer->tok.loc.tok_len = strlen(p_lexer->data) + 1;
	error(&p_lexer->tok.loc, "Unknown register "QUOTES("%s"), p_lexer->data);

	aborted();
}

void lexer_lex_skip_comment(lexer_t *p_lexer) {
	while (!lexer_at_line_end(p_lexer))
		lexer_next_ch(p_lexer);

	lexer_prev_ch(p_lexer);
}

void lexer_push_token(lexer_t *p_lexer, token_type_t p_type) {
	p_lexer->tok.data        = copy_str(p_lexer->data);
	p_lexer->tok.loc.line    = copy_str(p_lexer->line);
	p_lexer->tok.loc.file    = copy_str(p_lexer->path);
	p_lexer->tok.type        = p_type;
	p_lexer->tok.loc.tok_len = strlen(p_lexer->tok.data);

	list_push(&p_lexer->toks, &p_lexer->tok);

	p_lexer->data_len = 0;
	memset(p_lexer->data, 0, sizeof(p_lexer->data));
}

void lexer_add_ch(lexer_t *p_lexer) {
	p_lexer->data[p_lexer->data_len ++] = p_lexer->ch;

	if (p_lexer->data_len + 1 >= sizeof(p_lexer->data)) {
		fatal("Token at %s:%lu:%lu exceeds max token length %i",
		      p_lexer->path, (unsigned long)p_lexer->tok.loc.row,
		      (unsigned long)p_lexer->tok.loc.col, sizeof(p_lexer->data));
	}
}

void lexer_token_start_here(lexer_t *p_lexer) {
	p_lexer->tok.loc = p_lexer->loc;
}

bool lexer_at_line_end(lexer_t *p_lexer) {
	return p_lexer->ch == '\0';
}

void lexer_next_ch(lexer_t *p_lexer) {
	++ p_lexer->loc.col;
	p_lexer->ch = p_lexer->line[p_lexer->loc.col - 1];
}

void lexer_prev_ch(lexer_t *p_lexer) {
	-- p_lexer->loc.col;
	p_lexer->ch = p_lexer->line[p_lexer->loc.col - 1];
}
