#include "lexer.h"

pair_t g_keywords[] = {
	{.key = (void*)"include", .value = (void*)TOKEN_TYPE_INCLUDE},
	{.key = (void*)"data",    .value = (void*)TOKEN_TYPE_DATA},

	{.key = (void*)"INT64", .value = (void*)TOKEN_TYPE_INT64_TYPE},
	{.key = (void*)"INT32", .value = (void*)TOKEN_TYPE_INT32_TYPE},
	{.key = (void*)"INT16", .value = (void*)TOKEN_TYPE_INT16_TYPE},
	{.key = (void*)"WORD",  .value = (void*)TOKEN_TYPE_WORD_TYPE},
	{.key = (void*)"BYTE",  .value = (void*)TOKEN_TYPE_BYTE_TYPE},

	{.key = (void*)"none", .value = (void*)TOKEN_TYPE_INST_NONE},

	{.key = (void*)"move", .value = (void*)TOKEN_TYPE_INST_MOVE},

	{.key = (void*)"write", .value = (void*)TOKEN_TYPE_INST_WRITE},

	{.key = (void*)"read",  .value = (void*)TOKEN_TYPE_INST_READ},

	{.key = (void*)"push",  .value = (void*)TOKEN_TYPE_INST_PUSH},
	{.key = (void*)"pushA", .value = (void*)TOKEN_TYPE_INST_PUSH_A},

	{.key = (void*)"pop",  .value = (void*)TOKEN_TYPE_INST_POP},
	{.key = (void*)"popA", .value = (void*)TOKEN_TYPE_INST_POP_A},

	{.key = (void*)"eq",  .value = (void*)TOKEN_TYPE_INST_EQ},
	{.key = (void*)"neq", .value = (void*)TOKEN_TYPE_INST_NEQ},
	{.key = (void*)"gt",  .value = (void*)TOKEN_TYPE_INST_GT},
	{.key = (void*)"ge",  .value = (void*)TOKEN_TYPE_INST_GE},
	{.key = (void*)"lt",  .value = (void*)TOKEN_TYPE_INST_LT},
	{.key = (void*)"le",  .value = (void*)TOKEN_TYPE_INST_LE},

	{.key = (void*)"jump",  .value = (void*)TOKEN_TYPE_INST_JUMP},
	{.key = (void*)"jumpt", .value = (void*)TOKEN_TYPE_INST_JUMPT},
	{.key = (void*)"jumpf", .value = (void*)TOKEN_TYPE_INST_JUMPF},

	{.key = (void*)"add", .value = (void*)TOKEN_TYPE_INST_ADD},
	{.key = (void*)"inc", .value = (void*)TOKEN_TYPE_INST_INC},

	{.key = (void*)"sub", .value = (void*)TOKEN_TYPE_INST_SUB},
	{.key = (void*)"dec", .value = (void*)TOKEN_TYPE_INST_DEC},

	{.key = (void*)"mult", .value = (void*)TOKEN_TYPE_INST_MULT},
	{.key = (void*)"div",  .value = (void*)TOKEN_TYPE_INST_DIV},
	{.key = (void*)"mod",  .value = (void*)TOKEN_TYPE_INST_MOD},

	{.key = (void*)"rshift", .value = (void*)TOKEN_TYPE_INST_RSHIFT},
	{.key = (void*)"lshift", .value = (void*)TOKEN_TYPE_INST_LSHIFT},

	{.key = (void*)"and", .value = (void*)TOKEN_TYPE_INST_AND},
	{.key = (void*)"or",  .value = (void*)TOKEN_TYPE_INST_OR},
	{.key = (void*)"not", .value = (void*)TOKEN_TYPE_INST_NOT},

	{.key = (void*)"bitand", .value = (void*)TOKEN_TYPE_INST_BITAND},
	{.key = (void*)"bitor",  .value = (void*)TOKEN_TYPE_INST_BITOR},

	{.key = (void*)"call",  .value = (void*)TOKEN_TYPE_INST_CALL},
	{.key = (void*)"callt", .value = (void*)TOKEN_TYPE_INST_CALLT},
	{.key = (void*)"callf", .value = (void*)TOKEN_TYPE_INST_CALLF},
	{.key = (void*)"ret",   .value = (void*)TOKEN_TYPE_INST_RET},

	{.key = (void*)"syscall", .value = (void*)TOKEN_TYPE_INST_SYSCALL},

	{.key = (void*)"halt", .value = (void*)TOKEN_TYPE_INST_HALT}
};

pair_t g_regs[] = {
	{.key = (void*)"R1",  .value = (void*)TOKEN_TYPE_REG_1},
	{.key = (void*)"R2",  .value = (void*)TOKEN_TYPE_REG_2},
	{.key = (void*)"R3",  .value = (void*)TOKEN_TYPE_REG_3},
	{.key = (void*)"R4",  .value = (void*)TOKEN_TYPE_REG_4},
	{.key = (void*)"R5",  .value = (void*)TOKEN_TYPE_REG_5},
	{.key = (void*)"R6",  .value = (void*)TOKEN_TYPE_REG_6},
	{.key = (void*)"R7",  .value = (void*)TOKEN_TYPE_REG_7},
	{.key = (void*)"R8",  .value = (void*)TOKEN_TYPE_REG_8},
	{.key = (void*)"R9",  .value = (void*)TOKEN_TYPE_REG_9},
	{.key = (void*)"R10", .value = (void*)TOKEN_TYPE_REG_10},
	{.key = (void*)"R11", .value = (void*)TOKEN_TYPE_REG_11},
	{.key = (void*)"R12", .value = (void*)TOKEN_TYPE_REG_12},
	{.key = (void*)"R13", .value = (void*)TOKEN_TYPE_REG_13},
	{.key = (void*)"R14", .value = (void*)TOKEN_TYPE_REG_14},
	{.key = (void*)"R15", .value = (void*)TOKEN_TYPE_REG_15},

	{.key = (void*)"AC", .value = (void*)TOKEN_TYPE_REG_AC},
	{.key = (void*)"IP", .value = (void*)TOKEN_TYPE_REG_IP},
	{.key = (void*)"SP", .value = (void*)TOKEN_TYPE_REG_SP},
	{.key = (void*)"SB", .value = (void*)TOKEN_TYPE_REG_SB},
	{.key = (void*)"CN", .value = (void*)TOKEN_TYPE_REG_CN},
	{.key = (void*)"EX", .value = (void*)TOKEN_TYPE_REG_EX}
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
