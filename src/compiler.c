#include "compiler.h"

inst_func_map_t g_insts_map[] = {
	{.name = "move", .func = compiler_inst_move},

	{.name = "write64", .func = compiler_inst_write64},
	{.name = "write32", .func = compiler_inst_write32},
	{.name = "write16", .func = compiler_inst_write16},
	{.name = "write8",  .func = compiler_inst_write8},

	{.name = "read64", .func = compiler_inst_read64},
	{.name = "read32", .func = compiler_inst_read32},
	{.name = "read16", .func = compiler_inst_read16},
	{.name = "read8",  .func = compiler_inst_read8},

	{.name = "push64", .func = compiler_inst_push64},
	{.name = "push32", .func = compiler_inst_push32},
	{.name = "push16", .func = compiler_inst_push16},
	{.name = "push8",  .func = compiler_inst_push8},
	{.name = "pusha",  .func = compiler_inst_pusha},

	{.name = "pop64", .func = compiler_inst_pop64},
	{.name = "pop32", .func = compiler_inst_pop32},
	{.name = "pop16", .func = compiler_inst_pop16},
	{.name = "pop8",  .func = compiler_inst_pop8},
	{.name = "popa",  .func = compiler_inst_popa},

	{.name = "eq",  .func = compiler_inst_eq},
	{.name = "neq", .func = compiler_inst_neq},
	{.name = "gt",  .func = compiler_inst_gt},
	{.name = "ge",  .func = compiler_inst_ge},
	{.name = "lt",  .func = compiler_inst_lt},
	{.name = "le",  .func = compiler_inst_le},

	{.name = "jump",  .func = compiler_inst_jump},
	{.name = "jumpt", .func = compiler_inst_jumpt},
	{.name = "jumpf", .func = compiler_inst_jumpf},

	{.name = "add", .func = compiler_inst_add},
	{.name = "inc", .func = compiler_inst_inc},

	{.name = "sub", .func = compiler_inst_sub},
	{.name = "dec", .func = compiler_inst_dec},

	{.name = "mult", .func = compiler_inst_mult},
	{.name = "div",  .func = compiler_inst_div},
	{.name = "mod",  .func = compiler_inst_mod},

	{.name = "call",  .func = compiler_inst_call},
	{.name = "callt", .func = compiler_inst_callt},
	{.name = "callf", .func = compiler_inst_callf},
	{.name = "ret",  .func = compiler_inst_ret},

	{.name = "writef", .func = compiler_inst_writef},

	{.name = "memset",  .func = compiler_inst_memset},
	{.name = "memcopy", .func = compiler_inst_memcopy},

	{.name = "debug", .func = compiler_inst_debug},

	{.name = "halt", .func = compiler_inst_halt}
};

void compile(const char *p_source_path, const char *p_out_path) {
	assert(sizeof(g_insts_map) / sizeof(g_insts_map[0]) == IMPLEMENTED_INSTS);

	test();

	lexer_t lexer = lex(p_source_path);

	/* tokens_dump(&tokens); */

	/* if we lexed no tokens */
	if (lexer.tokens.count == 0)
		fatal("'%s' is an empty file", p_source_path);

	compiler_t compiler;
	memset(&compiler, 0, sizeof(compiler_t));

	list_init(&compiler.macros, sizeof(macro_t));
	list_init(&compiler.labels, sizeof(label_t));
	list_init(&compiler.insts,  sizeof(inst_t));

	compiler.path   = p_source_path;
	compiler.tokens = list_copy(&lexer.tokens);

	compiler_macros(&compiler);
	compiler_labels(&compiler);
	compiler_main(&compiler);

	if (compiler.err_count > 0) {
		fputs("\nCompilation aborted.\n", stderr);
		exit(EXIT_FAILURE);
	}

	compiler_write_file(&compiler, p_out_path);

	compiler_free_macros(&compiler);
	list_free(&compiler.labels);
	list_free(&compiler.insts);
	list_free(&compiler.tokens);

	tokens_free(&lexer.tokens);
	lexer_free_lines(&lexer);
}

void compiler_free_macros(compiler_t *p_compiler) {
	for (size_t i = 0; i < p_compiler->macros.count; ++ i) {
		macro_t *macro = LIST_AT(macro_t, &p_compiler->macros, i);

		list_free(&macro->tokens);
		list_free(&macro->args);
	}

	list_free(&p_compiler->macros);
}

void compiler_write_file(compiler_t *p_compiler, const char *p_out_path) {
	FILE *file = fopen(p_out_path, "w");
	if (file == NULL)
		fatal("Could not write file '%s'", p_out_path);

	fputs("#!/usr/bin/temple\nTM", file);

	compiler_write_word_bytes(file, p_compiler->entry_point);
	compiler_write_word_bytes(file, (word_t)p_compiler->insts.count);

	for (size_t i = 0; i < p_compiler->insts.count; ++ i) {
		inst_t *inst = LIST_AT(inst_t, &p_compiler->insts, i);

		uint8_t bytes[INST_SIZE] = {
			inst->opcode,
			inst->reg,
        	(inst->data & 0xFF00000000000000) >> 070,
        	(inst->data & 0x00FF000000000000) >> 060,
        	(inst->data & 0x0000FF0000000000) >> 050,
        	(inst->data & 0x000000FF00000000) >> 040,
        	(inst->data & 0x00000000FF000000) >> 030,
        	(inst->data & 0x0000000000FF0000) >> 020,
        	(inst->data & 0x000000000000FF00) >> 010,
        	(inst->data & 0x00000000000000FF)
		};

		fwrite(bytes, 1, sizeof(bytes), file);
	}

	fclose(file);
}

void compiler_write_word_bytes(FILE *p_file, word_t p_data) {
	uint8_t bytes[sizeof(word_t)] = {
        (p_data & 0xFF00000000000000) >> 070,
        (p_data & 0x00FF000000000000) >> 060,
        (p_data & 0x0000FF0000000000) >> 050,
        (p_data & 0x000000FF00000000) >> 040,
        (p_data & 0x00000000FF000000) >> 030,
        (p_data & 0x0000000000FF0000) >> 020,
        (p_data & 0x000000000000FF00) >> 010,
        (p_data & 0x00000000000000FF)
	};

	fwrite(bytes, 1, sizeof(bytes), p_file);
}

void compiler_macros(compiler_t *p_compiler) {
	for (p_compiler->i = 0; p_compiler->i < p_compiler->tokens.count; ++ p_compiler->i) {
		p_compiler->token = LIST_AT(token_t, &p_compiler->tokens, p_compiler->i);

		switch (p_compiler->token->type) {
		case TOKEN_TYPE_KEYWORD:
			if (strcmp("define", p_compiler->token->data) == 0)
				compiler_macro_definition(p_compiler);

			break;

		case TOKEN_TYPE_ID: compiler_expand_macro(p_compiler); break;

		default: break;
		}
	}
}

void compiler_labels(compiler_t *p_compiler) {
	word_t inst_pos  = 0;
	bool   has_entry = false;

	for (p_compiler->i = 0; p_compiler->i < p_compiler->tokens.count; ++ p_compiler->i) {
		p_compiler->token = LIST_AT(token_t, &p_compiler->tokens, p_compiler->i);

		switch (p_compiler->token->type) {
		case TOKEN_TYPE_INST: ++ inst_pos; break;
		case TOKEN_TYPE_LABEL:
			{
				if (strcmp(PROGRAM_ENTRY_LABEL, p_compiler->token->data) == 0) {
					has_entry = true;
					p_compiler->entry_point = inst_pos;
				}

				label_t label = {.name = p_compiler->token->data, .inst_pos = inst_pos};
				list_add(&p_compiler->labels, &label);
			}

			break;

		default: break;
		}
	}

	if (!has_entry)
		fatal("Program has no entry point");
}

void compiler_main(compiler_t *p_compiler) {
	for (p_compiler->i = 0; p_compiler->i < p_compiler->tokens.count; ++ p_compiler->i) {
		p_compiler->token = LIST_AT(token_t, &p_compiler->tokens, p_compiler->i);

		switch (p_compiler->token->type) {
		case TOKEN_TYPE_INST:    compiler_inst(p_compiler); break;
		case TOKEN_TYPE_KEYWORD: assert(0 && "'data' keyword not implemented");

		case TOKEN_TYPE_END:
		case TOKEN_TYPE_LABEL: break;

		case TOKEN_TYPE_ID:
			compiler_error(p_compiler, "Undefined identifier '%s'", p_compiler->token->data);

			break;

		default: compiler_error(p_compiler, "Unexpected token '%s'", p_compiler->token->data);
		}
	}
}

void compiler_inst(compiler_t *p_compiler) {
	for (size_t i = 0; i < IMPLEMENTED_INSTS; ++ i) {
		if (strcmp(g_insts_map[i].name, p_compiler->token->data) == 0) {
			size_t j = p_compiler->i + 1;
			while (LIST_AT(token_t, &p_compiler->tokens, j)->type != TOKEN_TYPE_END)
				++ j;

			g_insts_map[i].func(p_compiler, j - p_compiler->i - 1);

			while (p_compiler->token->type != TOKEN_TYPE_END)
				compiler_next_token(p_compiler);

			break;
		}
	}
}

void compiler_macro_definition(compiler_t *p_compiler) {
	macro_t macro = {
		.line = p_compiler->token->line,
		.row  = p_compiler->token->row,
		.col  = p_compiler->token->col
	};

	list_init(&macro.args,   sizeof(char*));
	list_init(&macro.tokens, sizeof(token_t));

	token_t *macro_declaration   = p_compiler->token;
	size_t   macro_declaration_i = p_compiler->i;

	compiler_next_token(p_compiler);
	if (p_compiler->token->type != TOKEN_TYPE_ID) {
		if (p_compiler->token->type == TOKEN_TYPE_INST)
			compiler_error_fatal(p_compiler, "Expected macro identifier, got %s",
			                     token_type_to_str(p_compiler->token));

		return;
	} else
		macro.name = p_compiler->token->data;

	compiler_next_token(p_compiler);
	if (p_compiler->token->type == TOKEN_TYPE_END || p_compiler->token->type == TOKEN_TYPE_LPAREN) {
		if (p_compiler->token->type == TOKEN_TYPE_LPAREN) {
			compiler_next_token(p_compiler);

			while (p_compiler->token->type != TOKEN_TYPE_RPAREN) {
				if (p_compiler->token->type == TOKEN_TYPE_END)
					compiler_error_fatal(p_compiler, "Argument list not ended");
				else if (p_compiler->token->type != TOKEN_TYPE_ID)
					compiler_error_fatal(p_compiler, "Expected token type identifier, got %s",
					                     token_type_to_str(p_compiler->token));

				list_add(&macro.args, &p_compiler->token->data);

				compiler_next_token(p_compiler);
			}

			if (macro.args.count == 0)
				compiler_error_fatal(p_compiler, "Empty argument lists are forbidden");

			compiler_next_token(p_compiler);
			if (p_compiler->token->type != TOKEN_TYPE_END)
				compiler_error_fatal(p_compiler, "Expected statement end, got %s",
				                     token_type_to_str(p_compiler->token));
		}

		compiler_next_token(p_compiler);

		while (true) {
			if (p_compiler->token->type == TOKEN_TYPE_KEYWORD) {
				if (strcmp("end", p_compiler->token->data) == 0)
					break;
			}

			list_add(&macro.tokens, p_compiler->token);

			++ p_compiler->i;
			if (p_compiler->i >= p_compiler->tokens.count)
				compiler_error_at_prev(p_compiler, macro_declaration,
				                       "Macro definition not ended");

			-- p_compiler->i;
			compiler_next_token(p_compiler);
		}

		list_remove(&p_compiler->tokens, macro_declaration_i,
		            macro.tokens.count + 4 + (macro.args.count > 0? macro.args.count + 2 : 0));

		p_compiler->i = macro_declaration_i;
	} else {
		while (p_compiler->token->type != TOKEN_TYPE_END) {
			list_add(&macro.tokens, p_compiler->token);

			++ p_compiler->i;
			assert(p_compiler->i < p_compiler->tokens.count && "This should probably never show");

			-- p_compiler->i;
			compiler_next_token(p_compiler);
		}

		list_remove(&p_compiler->tokens, macro_declaration_i, macro.tokens.count + 3);
		p_compiler->i -= macro.tokens.count + 3;
	}

	if (macro.tokens.count == 0)
		compiler_error(p_compiler, "Empty macros are forbidden");

	list_add(&p_compiler->macros, &macro);
}

void compiler_expand_macro(compiler_t *p_compiler) {
	for (size_t i = 0; i < p_compiler->macros.count; ++ i) {
		macro_t *macro = LIST_AT(macro_t, &p_compiler->macros, i);

		if (strcmp(macro->name, p_compiler->token->data) == 0) {
			for (size_t i = 0; i < macro->tokens.count; ++ i) {
				token_t *token = LIST_AT(token_t, &macro->tokens, i);

				token->line = p_compiler->token->line;
				token->row  = p_compiler->token->row;
				token->col  = p_compiler->token->col;
			}

			list_t args;
			list_init(&args, sizeof(token_t));

			size_t prev_i = p_compiler->i;

			compiler_next_token(p_compiler);
			if (p_compiler->token->type == TOKEN_TYPE_LPAREN) {
				compiler_next_token(p_compiler);

				while (p_compiler->token->type != TOKEN_TYPE_RPAREN) {
					if (p_compiler->token->type == TOKEN_TYPE_END)
						compiler_error_fatal(p_compiler, "Argument list not ended");

					list_add(&args, p_compiler->token);

					compiler_next_token(p_compiler);
				}

				if (args.count == 0)
					compiler_error_fatal(p_compiler, "Empty argument lists are forbidden");

				if (macro->args.count != args.count)
					compiler_error_fatal(p_compiler,
					                     "Macro '%s' expected %li arguments, but got %li",
					                     macro->name, (long)macro->args.count, (long)args.count);
			} else {
				compiler_prev_token(p_compiler);

				if (macro->args.count != 0)
					compiler_error_fatal(p_compiler, "Macro '%s' expects %li arguments",
					                     macro->name, macro->args.count);
			}

			p_compiler->i = prev_i;

			list_remove(&p_compiler->tokens, p_compiler->i,
			            1 + (args.count > 0? args.count + 2 : 0));

			list_insert(&p_compiler->tokens, p_compiler->i,
			            macro->tokens.buf, macro->tokens.count);

			if (macro->args.count > 0) {
				for (size_t i = p_compiler->i; i < p_compiler->i + macro->tokens.count; ++ i) {
					token_t *token = LIST_AT(token_t, &p_compiler->tokens, i);

					if (token->type == TOKEN_TYPE_ID) {
						bool   found   = false;
						size_t arg_idx = 0;
						for (; arg_idx < macro->args.count; ++ arg_idx) {
							char *arg_name = *LIST_AT(char*, &macro->args, arg_idx);
							if (strcmp(arg_name, token->data) == 0) {
								found = true;

								break;
							}
						}

						if (found)
							*token = *LIST_AT(token_t, &args, arg_idx);
					}
				}
			}

			break;
		}
	}
}

void compiler_next_token(compiler_t *p_compiler) {
	++ p_compiler->i;

	if (p_compiler->i >= p_compiler->tokens.count)
		compiler_error_fatal(p_compiler, "Unexpected end of file");
	else
		p_compiler->token = LIST_AT(token_t, &p_compiler->tokens, p_compiler->i);
}

void compiler_prev_token(compiler_t *p_compiler) {
	assert(p_compiler->i > 0 && "This should never fail");

	-- p_compiler->i;
	p_compiler->token = LIST_AT(token_t, &p_compiler->tokens, p_compiler->i);
}

void compiler_push_inst(compiler_t *p_compiler, opcode_t p_opcode, reg_t p_reg, word_t p_data) {
	inst_t inst = {.opcode = p_opcode, .reg = p_reg, .data = p_data};

	list_add(&p_compiler->insts, &inst);
}

reg_t compiler_token_to_reg(compiler_t *p_compiler) {
	if (p_compiler->token->type != TOKEN_TYPE_REG)
		assert(0 && "Token is not convertable to a register (this message should never show)");

	for (size_t i = 0; i < IMPLEMENTED_REGS; ++ i) {
		if (strcmp(g_regs[i], p_compiler->token->data) == 0)
			return (reg_t)i;
	}

	compiler_error(p_compiler, "Unknown register '%s'", p_compiler->token->data);

	return -1;
}

word_t compiler_token_to_num(compiler_t *p_compiler) {
	switch (p_compiler->token->type) {
	case TOKEN_TYPE_ID:
		for (size_t i = 0; i < p_compiler->labels.count; ++ i) {
			label_t *label = LIST_AT(label_t, &p_compiler->labels, i);

			if (strcmp(label->name, p_compiler->token->data) == 0)
				return label->inst_pos;
		}

		compiler_error(p_compiler, "Undefined identifier '%s'", p_compiler->token->data);

		return -1;

	case TOKEN_TYPE_CHAR: return p_compiler->token->data[0];

	case TOKEN_TYPE_HEX: case TOKEN_TYPE_DEC:
		return strtol(p_compiler->token->data, NULL,
		              p_compiler->token->type == TOKEN_TYPE_HEX? 16 : 10);

	default: assert(0 && "Token is not convertable to a number (this message should never show)");
	}
}

void compiler_inst_move(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'move' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_MOVE_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_MOVE, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'move' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'move' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_write64(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'write64' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_WRITE_R_64,
			                   reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_WRITE_64, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'write64' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'write64' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_write32(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'write32' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_WRITE_R_32,
			                   reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_WRITE_32, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'write32' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'write32' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_write16(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'write16' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_WRITE_R_16,
			                   reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_WRITE_16, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'write16' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'write16' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_write8(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'write8' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_WRITE_R_8,
			                   reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_WRITE_8, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'write8' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'write8' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_read64(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'read64' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_READ_R_64,
			                   reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_READ_64, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'read64' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'read64' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_read32(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'read32' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_READ_R_32,
			                   reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_READ_32, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'read32' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'read32' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_read16(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'read16' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_READ_R_16,
			                   reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_READ_16, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'read16' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'read16' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_read8(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'read8' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_READ_R_8, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_READ_8, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'read8' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'read8' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_push64(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'push64' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	switch (p_compiler->token->type) {
	case TOKEN_TYPE_REG:
		compiler_push_inst(p_compiler, OPCODE_PUSH_R_64, compiler_token_to_reg(p_compiler), 0);

		break;

	case CASE_TOKEN_TYPE_NUM:
		compiler_push_inst(p_compiler, OPCODE_PUSH_64, 0, compiler_token_to_num(p_compiler));

		break;

	default:
		compiler_error(p_compiler, "'push64' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_push32(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'push32' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	switch (p_compiler->token->type) {
	case TOKEN_TYPE_REG:
		compiler_push_inst(p_compiler, OPCODE_PUSH_R_32, compiler_token_to_reg(p_compiler), 0);

		break;

	case CASE_TOKEN_TYPE_NUM:
		compiler_push_inst(p_compiler, OPCODE_PUSH_32, 0, compiler_token_to_num(p_compiler));

		break;

	default:
		compiler_error(p_compiler, "'push32' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_push16(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'push16' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	switch (p_compiler->token->type) {
	case TOKEN_TYPE_REG:
		compiler_push_inst(p_compiler, OPCODE_PUSH_R_16, compiler_token_to_reg(p_compiler), 0);

		break;

	case CASE_TOKEN_TYPE_NUM:
		compiler_push_inst(p_compiler, OPCODE_PUSH_16, 0, compiler_token_to_num(p_compiler));

		break;

	default:
		compiler_error(p_compiler, "'push16' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_push8(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'push8' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	switch (p_compiler->token->type) {
	case TOKEN_TYPE_REG:
		compiler_push_inst(p_compiler, OPCODE_PUSH_R_8, compiler_token_to_reg(p_compiler), 0);

		break;

	case CASE_TOKEN_TYPE_NUM:
		compiler_push_inst(p_compiler, OPCODE_PUSH_8, 0, compiler_token_to_num(p_compiler));

		break;

	default:
		compiler_error(p_compiler, "'push8' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_pusha(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 0) {
		compiler_error(p_compiler, "'pusha' expects no arguments, got %li", (long)p_argc);

		return;
	}

	compiler_push_inst(p_compiler, OPCODE_PUSH_A, 0, 0);
}

void compiler_inst_pop64(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc > 1) {
		compiler_error(p_compiler, "'pop64' expects 1 or no arguments, got %li", (long)p_argc);

		return;
	}

	if (p_argc == 0)
		compiler_push_inst(p_compiler, OPCODE_POP_64, 0, 0);
	else {
		compiler_next_token(p_compiler);

		if (p_compiler->token->type == TOKEN_TYPE_REG)
			compiler_push_inst(p_compiler, OPCODE_POP_R_64, compiler_token_to_reg(p_compiler), 0);
		else
			compiler_error(p_compiler, "'pop64' incorrect argument 1 type '%s'",
			               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_pop32(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc > 1) {
		compiler_error(p_compiler, "'pop32' expects 1 or no arguments, got %li", (long)p_argc);

		return;
	}

	if (p_argc == 0)
		compiler_push_inst(p_compiler, OPCODE_POP_32, 0, 0);
	else {
		compiler_next_token(p_compiler);

		if (p_compiler->token->type == TOKEN_TYPE_REG)
			compiler_push_inst(p_compiler, OPCODE_POP_R_32, compiler_token_to_reg(p_compiler), 0);
		else
			compiler_error(p_compiler, "'pop32' incorrect argument 1 type '%s'",
			               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_pop16(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc > 1) {
		compiler_error(p_compiler, "'pop16' expects 1 or no arguments, got %li", (long)p_argc);

		return;
	}

	if (p_argc == 0)
		compiler_push_inst(p_compiler, OPCODE_POP_16, 0, 0);
	else {
		compiler_next_token(p_compiler);

		if (p_compiler->token->type == TOKEN_TYPE_REG)
			compiler_push_inst(p_compiler, OPCODE_POP_R_16, compiler_token_to_reg(p_compiler), 0);
		else
			compiler_error(p_compiler, "'pop16' incorrect argument 1 type '%s'",
			               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_pop8(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc > 1) {
		compiler_error(p_compiler, "'pop8' expects 1 or no arguments, got %li", (long)p_argc);

		return;
	}

	if (p_argc == 0)
		compiler_push_inst(p_compiler, OPCODE_POP_8, 0, 0);
	else {
		compiler_next_token(p_compiler);

		if (p_compiler->token->type == TOKEN_TYPE_REG)
			compiler_push_inst(p_compiler, OPCODE_POP_R_8, compiler_token_to_reg(p_compiler), 0);
		else
			compiler_error(p_compiler, "'pop8' incorrect argument 1 type '%s'",
			               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_popa(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 0) {
		compiler_error(p_compiler, "'popa' expects no arguments, got %li", (long)p_argc);

		return;
	}

	compiler_push_inst(p_compiler, OPCODE_POP_A, 0, 0);
}

void compiler_inst_eq(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'eq' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_EQ_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_EQ, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'eq' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'eq' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_neq(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'neq' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_NEQ_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_NEQ, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'neq' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'neq' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_gt(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'gt' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_GT_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_GT, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'gt' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'gt' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_ge(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'ge' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_GE_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_GE, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'ge' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'ge' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_lt(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'lt' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_LT_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_LT, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'lt' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'lt' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_le(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'le' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_LE_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_LE, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'le' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'le' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_jump(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'jump' expects 1 argument, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	switch (p_compiler->token->type) {
	case TOKEN_TYPE_REG:
		compiler_push_inst(p_compiler, OPCODE_JUMP_R, compiler_token_to_reg(p_compiler), 0);

		break;

	case CASE_TOKEN_TYPE_NUM:
		compiler_push_inst(p_compiler, OPCODE_JUMP, 0, compiler_token_to_num(p_compiler));

		break;

	default:
		compiler_error(p_compiler, "'jump' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_jumpt(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'jumpt' expects 1 argument, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	switch (p_compiler->token->type) {
	case TOKEN_TYPE_REG:
		compiler_push_inst(p_compiler, OPCODE_JUMP_T_R, compiler_token_to_reg(p_compiler), 0);

		break;

	case CASE_TOKEN_TYPE_NUM:
		compiler_push_inst(p_compiler, OPCODE_JUMP_T, 0, compiler_token_to_num(p_compiler));

		break;

	default:
		compiler_error(p_compiler, "'jumpt' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_jumpf(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'jumpf' expects 1 argument, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	switch (p_compiler->token->type) {
	case TOKEN_TYPE_REG:
		compiler_push_inst(p_compiler, OPCODE_JUMP_F_R, compiler_token_to_reg(p_compiler), 0);

		break;

	case CASE_TOKEN_TYPE_NUM:
		compiler_push_inst(p_compiler, OPCODE_JUMP_F, 0, compiler_token_to_num(p_compiler));

		break;

	default:
		compiler_error(p_compiler, "'jumpf' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_add(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'add' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_ADD_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_ADD, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'add' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'add' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_inc(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'inc' expects 1 argument, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG)
		compiler_push_inst(p_compiler, OPCODE_INC, compiler_token_to_reg(p_compiler), 0);
	else
		compiler_error(p_compiler, "'inc' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_sub(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'sub' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_SUB_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_SUB, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'sub' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'sub' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_dec(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'dec' expects 1 argument, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG)
		compiler_push_inst(p_compiler, OPCODE_DEC, compiler_token_to_reg(p_compiler), 0);
	else
		compiler_error(p_compiler, "'dec' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_mult(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'mult' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_MULT_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_MULT, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'mult' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'mult' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_div(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'div' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_DIV_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_DIV, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'div' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'div' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_mod(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 2) {
		compiler_error(p_compiler, "'mod' expects 2 arguments, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	if (p_compiler->token->type == TOKEN_TYPE_REG) {
		reg_t reg = compiler_token_to_reg(p_compiler);

		compiler_next_token(p_compiler);
		switch (p_compiler->token->type) {
		case TOKEN_TYPE_REG:
			compiler_push_inst(p_compiler, OPCODE_MOD_R, reg, compiler_token_to_reg(p_compiler));

			break;

		case CASE_TOKEN_TYPE_NUM:
			compiler_push_inst(p_compiler, OPCODE_MOD, reg, compiler_token_to_num(p_compiler));

			break;

		default:
			compiler_error(p_compiler, "'mod' incorrect argument 2 type '%s'",
			               token_type_to_str(p_compiler->token));
		}
	} else
		compiler_error(p_compiler, "'mod' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
}

void compiler_inst_call(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'call' expects 1 argument, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	switch (p_compiler->token->type) {
	case TOKEN_TYPE_REG:
		compiler_push_inst(p_compiler, OPCODE_CALL_R, compiler_token_to_reg(p_compiler), 0);

		break;

	case CASE_TOKEN_TYPE_NUM:
		compiler_push_inst(p_compiler, OPCODE_CALL, 0, compiler_token_to_num(p_compiler));

		break;

	default:
		compiler_error(p_compiler, "'call' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_callt(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'callt' expects 1 argument, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	switch (p_compiler->token->type) {
	case TOKEN_TYPE_REG:
		compiler_push_inst(p_compiler, OPCODE_CALL_T_R, compiler_token_to_reg(p_compiler), 0);

		break;

	case CASE_TOKEN_TYPE_NUM:
		compiler_push_inst(p_compiler, OPCODE_CALL_T, 0, compiler_token_to_num(p_compiler));

		break;

	default:
		compiler_error(p_compiler, "'callt' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_callf(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 1) {
		compiler_error(p_compiler, "'callf' expects 1 argument, got %li", (long)p_argc);

		return;
	} else
		compiler_next_token(p_compiler);

	switch (p_compiler->token->type) {
	case TOKEN_TYPE_REG:
		compiler_push_inst(p_compiler, OPCODE_CALL_F_R, compiler_token_to_reg(p_compiler), 0);

		break;

	case CASE_TOKEN_TYPE_NUM:
		compiler_push_inst(p_compiler, OPCODE_CALL_F, 0, compiler_token_to_num(p_compiler));

		break;

	default:
		compiler_error(p_compiler, "'callf' incorrect argument 1 type '%s'",
		               token_type_to_str(p_compiler->token));
	}
}

void compiler_inst_ret(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 0) {
		compiler_error(p_compiler, "'ret' expects no arguments, got %li", (long)p_argc);

		return;
	}

	compiler_push_inst(p_compiler, OPCODE_RET, 0, 0);
}

void compiler_inst_writef(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 0) {
		compiler_error(p_compiler, "'writef' expects no arguments, got %li", (long)p_argc);

		return;
	}

	compiler_push_inst(p_compiler, OPCODE_WRITEF, 0, 0);
}

void compiler_inst_memset(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 0) {
		compiler_error(p_compiler, "'memset' expects no arguments, got %li", (long)p_argc);

		return;
	}

	compiler_push_inst(p_compiler, OPCODE_MEMSET, 0, 0);
}

void compiler_inst_memcopy(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 0) {
		compiler_error(p_compiler, "'memcopy' expects no arguments, got %li", (long)p_argc);

		return;
	}

	compiler_push_inst(p_compiler, OPCODE_MEMCOPY, 0, 0);
}

void compiler_inst_debug(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 0) {
		compiler_error(p_compiler, "'debug' expects no arguments, got %li", (long)p_argc);

		return;
	}

	compiler_push_inst(p_compiler, OPCODE_DEBUG, 0, 0);
}

void compiler_inst_halt(compiler_t *p_compiler, size_t p_argc) {
	if (p_argc != 0) {
		compiler_error(p_compiler, "'halt' expects no arguments, got %li", (long)p_argc);

		return;
	}

	compiler_push_inst(p_compiler, OPCODE_HALT, 0, 0);
}

void compiler_error(compiler_t *p_compiler, const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	if (p_compiler->err_count > 0)
		putchar('\n');

	error_at(p_compiler->token->row + 1, p_compiler->token->col + 1, p_compiler->token->line,
	         p_compiler->path, "Compiler error", msg);

	++ p_compiler->err_count;
	if (p_compiler->err_count >= MAX_COMPILER_ERRS) {
		fputs("\nCompilation aborted.\n", stderr);
		exit(EXIT_FAILURE);
	}
}

void compiler_error_fatal(compiler_t *p_compiler, const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	if (p_compiler->err_count > 0)
		putchar('\n');

	error_at(p_compiler->token->row + 1, p_compiler->token->col + 1, p_compiler->token->line,
	         p_compiler->path, "Compiler error", msg);

	fputs("\nCompilation aborted.\n", stderr);
	exit(EXIT_FAILURE);
}

void compiler_error_at_prev(compiler_t *p_compiler, token_t *p_prev, const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	token_t *current_token = p_compiler->token;
	p_compiler->token      = p_prev;

	compiler_error(p_compiler, msg);

	p_compiler->token = current_token;
}
