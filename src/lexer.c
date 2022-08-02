#include "lexer.h"

const char *g_keywords[] = {
	"define",
	"data",
	"end",
	"include"
};

lexer_t lex(const char *p_path) {
	lexer_t lexer;
	memset(&lexer, 0, sizeof(lexer_t));

	list_init(&lexer.tokens, sizeof(token_t));
	list_init(&lexer.lines, sizeof(char*));

	lexer.path = p_path;
	lexer.file = fopen(p_path, "r");
	if (lexer.file == NULL)
		fatal("Could not open file '%s'", lexer.path);

	char ch = fgetc(lexer.file);
	if (ch == '#') {
		while (fgetc(lexer.file) != '\n'); /* skip the line */

		++ lexer.row;
	} else
		ungetc(ch, lexer.file);

	while (lexer_line(&lexer));

	fclose(lexer.file);

	return lexer;
}

void lexer_free_lines(lexer_t *p_lexer) {
	for (size_t i = 0; i < p_lexer->lines.count; ++ i)
		free(*LIST_AT(char*, &p_lexer->lines, i));

	list_free(&p_lexer->lines);
}

bool lexer_line(lexer_t *p_lexer) {
	bool not_at_end = true;

	size_t line_buf_size = 64;
	p_lexer->line_len    = 0;
	p_lexer->line        = (char*)malloc(line_buf_size);
	if (p_lexer->line == NULL)
		assert(0 && "malloc fail");

	list_add(&p_lexer->lines, &p_lexer->line);

	p_lexer->line[0] = '\0';

	/* read the line into the line bufer */
	char ch = 0;
	while ((ch = fgetc(p_lexer->file)) != '\n') {
		if (ch == EOF) { /* if we are at the end, let the main lex function know */
			not_at_end = false;

			break;
		}

		/* increase the buffer if needed */
		p_lexer->line[p_lexer->line_len ++] = ch;
		p_lexer->line[p_lexer->line_len]    = '\0';

		if (p_lexer->line_len + 1 >= line_buf_size) {
			line_buf_size *= 2;

			void *tmp = realloc(p_lexer->line, line_buf_size);
			if (tmp == NULL) {
				tokens_free(&p_lexer->tokens);

				assert(0 && "realloc fail");
			} else
				p_lexer->line = (char*)tmp;
		}
	}

	bool token_lexed = false;
	for (p_lexer->col = 0; p_lexer->col < p_lexer->line_len; ++ p_lexer->col) {
		if (lexer_token(p_lexer) && !token_lexed)
			token_lexed = true;
	}

	/* we dont need bloat new line tokens */
	if (token_lexed) {
		token_t token = token_new(TOKEN_TYPE_END, NULL, p_lexer->line, p_lexer->row, p_lexer->col);
		list_add(&p_lexer->tokens, &token);
	}

	++ p_lexer->row;

	return not_at_end;
}

bool lexer_token(lexer_t *p_lexer) {
	p_lexer->token_buf_size = 64;
	p_lexer->token_len      = 0;
	p_lexer->token_str      = (char*)malloc(p_lexer->token_buf_size);
	if (p_lexer->token_str == NULL)
		assert(0 && "malloc fail");

	p_lexer->token_str[0] = '\0';
	p_lexer->token        = token_new(TOKEN_TYPE_ID, p_lexer->token_str,
	                                  p_lexer->line, p_lexer->row, p_lexer->col);

	for (; p_lexer->col < p_lexer->line_len; ++ p_lexer->col) {
		char ch = p_lexer->line[p_lexer->col]; /* for shorter and more readable code */

		switch (ch) {
		case ' ': case '\t': case '\v': case '\f': case '\r':
			++ p_lexer->token.col;

			continue;

		case ';': lexer_skip_comment(p_lexer); goto no_token;
		case '|':
			lexer_add_to_token(p_lexer, ch);
			p_lexer->token.type = TOKEN_TYPE_END;

			break;

		case '"':
			p_lexer->token.type = TOKEN_TYPE_STR;

			++ p_lexer->col;
			lexer_str(p_lexer);

			break;

		case '(':
			lexer_add_to_token(p_lexer, ch);
			p_lexer->token.type = TOKEN_TYPE_LPAREN;

			break;

		case ')':
			lexer_add_to_token(p_lexer, ch);
			p_lexer->token.type = TOKEN_TYPE_RPAREN;

			break;

		case '\'':
			p_lexer->token.type = TOKEN_TYPE_CHAR;

			++ p_lexer->col;
			lexer_char(p_lexer);

			break;

		case '&':
			p_lexer->token.type = TOKEN_TYPE_REG;

			++ p_lexer->col;
			lexer_word(p_lexer);

			if (p_lexer->token_len == 0)
				lexer_fatal(p_lexer, "Expected a register");

			break;

		case '$':
			p_lexer->token.type = TOKEN_TYPE_HEX;

			++ p_lexer->col;
			lexer_hex(p_lexer);

			if (p_lexer->token_len == 0)
				lexer_fatal(p_lexer, "Expected a hexadecimal number");

			break;

		default:
			if (isalpha(ch) || ch == '_' || ch == '-')
				lexer_word(p_lexer);
			else if (isdigit(ch)) {
				p_lexer->token.type = TOKEN_TYPE_DEC;
				lexer_dec(p_lexer);
			} else
				lexer_fatal(p_lexer, "Unexpected character '%c'", ch);
		}

		/* this function will only be reached after a token was collected */
		list_add(&p_lexer->tokens, &p_lexer->token);

		return true; /* a token was lexed */
	}

no_token:
	free(p_lexer->token_str);

	return false;
}

void lexer_skip_comment(lexer_t *p_lexer) {
	while (p_lexer->col < p_lexer->line_len)
		++ p_lexer->col;
}

void lexer_str(lexer_t *p_lexer) {
	bool escape = false;

	for (; p_lexer->col < p_lexer->line_len; ++ p_lexer->col) {
		char ch = p_lexer->line[p_lexer->col];

		switch (ch) {
		case '"':
			if (escape)
				lexer_fatal(p_lexer, "Unfinished escape sequence");

			return;

		case '\\':
			if (escape)
				lexer_add_to_token(p_lexer, '\\');

			escape = !escape;

			break;

		default:
			if (escape) {
				switch (ch) {
				case '0': ch = '\0'; break;
				case 'a': ch = '\a'; break;
				case 'b': ch = '\b'; break;
				case 'f': ch = '\f'; break;
				case 'n': ch = '\n'; break;
				case 'r': ch = '\r'; break;
				case 't': ch = '\t'; break;
				case 'v': ch = '\v'; break;
				case 'e': ch = 27;   break;

				default: lexer_fatal(p_lexer, "Unknown escape sequence '\\%c'", ch);
				}
			}

			lexer_add_to_token(p_lexer, ch);
		}
	}

	lexer_fatal(p_lexer, "Expected a string end");
}

void lexer_char(lexer_t *p_lexer) {
	bool escape    = false;
	bool read_char = false;

	for (; p_lexer->col < p_lexer->line_len; ++ p_lexer->col) {
		char ch = p_lexer->line[p_lexer->col];

		switch (ch) {
		case '\'':
			if (!read_char)
				lexer_fatal(p_lexer, "Character expected a single char");

			return;

		case '\\':
			if (read_char)
				lexer_fatal(p_lexer, "Character expected a single char");

			if (escape) {
				lexer_add_to_token(p_lexer, '\\');
				read_char = true;
			}

			escape = !escape;

			break;

		default:
			if (read_char)
				lexer_fatal(p_lexer, "Character expected a single char");

			if (escape) {
				switch (ch) {
				case '0': ch = '\0'; break;
				case 'a': ch = '\a'; break;
				case 'b': ch = '\b'; break;
				case 'f': ch = '\f'; break;
				case 'n': ch = '\n'; break;
				case 'r': ch = '\r'; break;
				case 't': ch = '\t'; break;
				case 'v': ch = '\v'; break;
				case 'e': ch = 27;   break;

				default: lexer_fatal(p_lexer, "Unknown escape sequence '\\%c'", ch);
				}
			}

			lexer_add_to_token(p_lexer, ch);
			read_char = true;
		}
	}

	lexer_fatal(p_lexer, "Expected a string end");
}

void lexer_dec(lexer_t *p_lexer) {
	for (; p_lexer->col < p_lexer->line_len; ++ p_lexer->col) {
		char ch = p_lexer->line[p_lexer->col];

		switch (ch) {
		case CASE_SEPARATORS: return;
		default:
			if (!isdigit(ch))
				lexer_fatal(p_lexer, "Unexpected character '%c' in decimal", ch);

			lexer_add_to_token(p_lexer, ch);
		}
	}
}

void lexer_hex(lexer_t *p_lexer) {
	for (; p_lexer->col < p_lexer->line_len; ++ p_lexer->col) {
		char ch = p_lexer->line[p_lexer->col];

		switch (ch) {
		case CASE_SEPARATORS: return;
		default:
			if (!isdigit(ch) && ch < 'a' && ch > 'f' && ch < 'A' && ch > 'F')
				lexer_fatal(p_lexer, "Unexpected character '%c' in hexadecimal", ch);

			lexer_add_to_token(p_lexer, ch);
		}
	}
}

void lexer_word(lexer_t *p_lexer) {
	for (; p_lexer->col < p_lexer->line_len; ++ p_lexer->col) {
		char ch = p_lexer->line[p_lexer->col];

		switch (ch) {
		case CASE_SEPARATORS:
			goto finish;

		case ':':
			p_lexer->token.type = TOKEN_TYPE_LABEL;

			return;

		default:
			if (!isalnum(ch))
				lexer_fatal(p_lexer, "Unexpected character '%c' in word", ch);

			/* fall through */

		case '_': case '-': lexer_add_to_token(p_lexer, ch);
		}
	}

finish:
	for (size_t i = 0; i < sizeof(g_keywords) / sizeof(g_keywords[0]); ++ i) {
		if (strcmp(g_keywords[i], p_lexer->token_str) == 0) {
			p_lexer->token.type = TOKEN_TYPE_KEYWORD;

			return;
		}
	}

	for (size_t i = 0; i < IMPLEMENTED_INSTS; ++ i) {
		if (strcmp(g_insts[i], p_lexer->token_str) == 0) {
			p_lexer->token.type = TOKEN_TYPE_INST;

			break;
		}
	}
}

void lexer_add_to_token(lexer_t *p_lexer, char p_ch) {
	p_lexer->token_str[p_lexer->token_len ++] = p_ch;
	p_lexer->token_str[p_lexer->token_len]    = '\0';

	if (p_lexer->token_len + 1 >= p_lexer->token_buf_size) {
		p_lexer->token_buf_size *= 2;

		void *tmp = realloc(p_lexer->token_str, p_lexer->token_buf_size);
		if (tmp == NULL) {
			free(&p_lexer->tokens);
			free(p_lexer->token_str);

			assert(0 && "realloc fail");
		} else
			p_lexer->token_str = (char*)tmp;
	}
}

void lexer_fatal(lexer_t *p_lexer, const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	error_at(p_lexer->row + 1, p_lexer->col + 1, p_lexer->line,
	         p_lexer->path, "Lexer error", msg);

	exit(EXIT_FAILURE);
}
