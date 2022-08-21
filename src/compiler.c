#include "compiler.h"

inst_func_t g_inst_func_map[] = {
	{.key = TOKEN_TYPE_INST_NONE, .func = compiler_compile_inst_none},

	{.key = TOKEN_TYPE_INST_MOVE, .func = compiler_compile_inst_move},

	{.key = TOKEN_TYPE_INST_WRITE, .func = compiler_compile_inst_write},
	{.key = TOKEN_TYPE_INST_READ,  .func = compiler_compile_inst_read},

	{.key = TOKEN_TYPE_INST_PUSH,   .func = compiler_compile_inst_push},
	{.key = TOKEN_TYPE_INST_PUSH_A, .func = compiler_compile_inst_pusha},

	{.key = TOKEN_TYPE_INST_POP,   .func = compiler_compile_inst_pop},
	{.key = TOKEN_TYPE_INST_POP_A, .func = compiler_compile_inst_popa},

	{.key = TOKEN_TYPE_INST_EQ,  .func = compiler_compile_inst_eq},
	{.key = TOKEN_TYPE_INST_NEQ, .func = compiler_compile_inst_neq},
	{.key = TOKEN_TYPE_INST_GT,  .func = compiler_compile_inst_gt},
	{.key = TOKEN_TYPE_INST_GE,  .func = compiler_compile_inst_ge},
	{.key = TOKEN_TYPE_INST_LT,  .func = compiler_compile_inst_lt},
	{.key = TOKEN_TYPE_INST_LE,  .func = compiler_compile_inst_le},

	{.key = TOKEN_TYPE_INST_JUMP,  .func = compiler_compile_inst_jump},
	{.key = TOKEN_TYPE_INST_JUMPT, .func = compiler_compile_inst_jumpt},
	{.key = TOKEN_TYPE_INST_JUMPF, .func = compiler_compile_inst_jumpf},

	{.key = TOKEN_TYPE_INST_ADD, .func = compiler_compile_inst_add},
	{.key = TOKEN_TYPE_INST_INC, .func = compiler_compile_inst_inc},

	{.key = TOKEN_TYPE_INST_SUB, .func = compiler_compile_inst_sub},
	{.key = TOKEN_TYPE_INST_DEC, .func = compiler_compile_inst_dec},

	{.key = TOKEN_TYPE_INST_MULT, .func = compiler_compile_inst_mult},
	{.key = TOKEN_TYPE_INST_DIV,  .func = compiler_compile_inst_div},
	{.key = TOKEN_TYPE_INST_MOD, .func = compiler_compile_inst_mod},

	{.key = TOKEN_TYPE_INST_RSHIFT, .func = compiler_compile_inst_rshift},
	{.key = TOKEN_TYPE_INST_LSHIFT, .func = compiler_compile_inst_lshift},

	{.key = TOKEN_TYPE_INST_AND, .func = compiler_compile_inst_and},
	{.key = TOKEN_TYPE_INST_OR,  .func = compiler_compile_inst_or},
	{.key = TOKEN_TYPE_INST_NOT, .func = compiler_compile_inst_not},

	{.key = TOKEN_TYPE_INST_BITAND, .func = compiler_compile_inst_bitand},
	{.key = TOKEN_TYPE_INST_BITOR,  .func = compiler_compile_inst_bitor},

	{.key = TOKEN_TYPE_INST_CALL,  .func = compiler_compile_inst_call},
	{.key = TOKEN_TYPE_INST_CALLT, .func = compiler_compile_inst_callt},
	{.key = TOKEN_TYPE_INST_CALLF, .func = compiler_compile_inst_callf},
	{.key = TOKEN_TYPE_INST_RET,   .func = compiler_compile_inst_ret},

	{.key = TOKEN_TYPE_INST_SYSCALL, .func = compiler_compile_inst_syscall},

	{.key = TOKEN_TYPE_INST_HALT, .func = compiler_compile_inst_halt},
};

void write_word_to_file(FILE *p_file, word_t p_data) {
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

compiler_t compiler_new(const char *p_path) {
	compiler_t compiler = {0};
	compiler.parser = parser_new(p_path);

#ifdef TMASM_DEBUG
	for (size_t i = 0; i < compiler.parser.toks.count; ++ i)
		token_dump(LIST_AT(token_t, &compiler.parser.toks, i));

	puts("\n");
#endif

	compiler.path = p_path;

	compiler.nodes = list_new(sizeof(node_t), LIST_FREE_FUNC(node_free_tree));

	compiler.data        = list_new(sizeof(uint8_t),      NULL);
	compiler.insts       = list_new(sizeof(inst_t),       NULL);
	compiler.labels      = list_new(sizeof(label_t),      LIST_FREE_FUNC(label_free));
	compiler.data_consts = list_new(sizeof(data_const_t), LIST_FREE_FUNC(data_const_free));

	return compiler;
}

void compiler_compile(compiler_t *p_compiler, const char *p_output_path) {
	/* first, compile all of the label declarations and store all other nodes inside a list
	   to inspect later */
	while ((p_compiler->node = parser_parse_next(&p_compiler->parser)) != NULL) {
#ifdef TMASM_DEBUG
		node_dump(p_compiler->node);
#endif

		switch (p_compiler->node->type) {
		case NODE_TYPE_DATA:
		case NODE_TYPE_INST:
			list_push(&p_compiler->nodes, p_compiler->node);
			free(p_compiler->node);

			break;

		case NODE_TYPE_LABEL:
			compiler_compile_label(p_compiler);
			node_free_tree(p_compiler->node);

			break;

		default: INTERNAL_BUG;
		}
	}

	/* after all the label definitions were compiled, compile the rest of the nodes */
	for (size_t i = 0; i < p_compiler->nodes.count; ++ i) {
		p_compiler->node = LIST_AT(node_t, &p_compiler->nodes, i);

		switch (p_compiler->node->type) {
		case NODE_TYPE_DATA: compiler_compile_data(p_compiler); break;
		case NODE_TYPE_INST: compiler_compile_inst(p_compiler); break;

		default: INTERNAL_BUG;
		}
	}

	compiler_generate_tm_file(p_compiler, p_output_path);

	parser_free(&p_compiler->parser);

	list_free(&p_compiler->data);
	list_free(&p_compiler->insts);
	list_free(&p_compiler->labels);
	list_free(&p_compiler->data_consts);
}

void compiler_generate_tm_file(compiler_t *p_compiler, const char *p_path) {
	label_t *entry_point = NULL;

	for (size_t i = 0; i < p_compiler->labels.count; ++ i) {
		label_t *label = LIST_AT(label_t, &p_compiler->labels, i);

		if (strcmp(label->name, ENTRY_LABEL) == 0) {
			entry_point = label;

			break;
		}
	}

	if (entry_point == NULL)
		fatal("%s: Entry label "QUOTES("%s")" not found", p_compiler->path, ENTRY_LABEL);

	FILE *file = fopen(p_path, "w");
	if (file == NULL)
		fatal("Could not write file "QUOTES("%s"), p_path);

	fputs("#!/usr/bin/temple\nTM", file);

	write_word_to_file(file, entry_point->addr);
	//write_word_to_file(file, p_compiler->data.count);
	write_word_to_file(file, p_compiler->insts.count);

	//fwrite(p_compiler->data.buf, 1, p_compiler->data.count, file);

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

void compiler_compile_data(compiler_t *p_compiler) {
	data_const_t data_const = {
		.name = copy_str(p_compiler->node->right->tok->data),
		.addr = p_compiler->data.count
	};

	switch (p_compiler->node->left->type) {
	case NODE_TYPE_NUM:
		{
			uint64_t data = p_compiler->node->left->data.num;

			switch (p_compiler->node->size) {
			case 8: LIST_PUSH(uint8_t, &p_compiler->data, data); break;
			case 16:
				LIST_PUSH(uint8_t, &p_compiler->data, (data & 0xFF00) >> 010);
				LIST_PUSH(uint8_t, &p_compiler->data,  data & 0x00FF);

				break;

			case 32:
				LIST_PUSH(uint8_t, &p_compiler->data, (data & 0xFF000000) >> 030);
				LIST_PUSH(uint8_t, &p_compiler->data, (data & 0x00FF0000) >> 020);
				LIST_PUSH(uint8_t, &p_compiler->data, (data & 0x0000FF00) >> 010);
				LIST_PUSH(uint8_t, &p_compiler->data,  data & 0x000000FF);

				break;

			case 64:
				LIST_PUSH(uint8_t, &p_compiler->data, (data & 0xFF00000000000000) >> 070);
				LIST_PUSH(uint8_t, &p_compiler->data, (data & 0x00FF000000000000) >> 060);
				LIST_PUSH(uint8_t, &p_compiler->data, (data & 0x0000FF0000000000) >> 050);
				LIST_PUSH(uint8_t, &p_compiler->data, (data & 0x000000FF00000000) >> 040);
				LIST_PUSH(uint8_t, &p_compiler->data, (data & 0x00000000FF000000) >> 030);
				LIST_PUSH(uint8_t, &p_compiler->data, (data & 0x0000000000FF0000) >> 020);
				LIST_PUSH(uint8_t, &p_compiler->data, (data & 0x000000000000FF00) >> 010);
				LIST_PUSH(uint8_t, &p_compiler->data,  data & 0x00000000000000ff);

				break;

			default: INTERNAL_BUG;
			}
		}

		break;

	case NODE_TYPE_STR:
		{
			char *data = p_compiler->node->left->tok->data;

			/* use <= to also include the null terminator */
			for (size_t i = 0; i <= strlen(data); ++ i) {
				for (size_t j = 8; j < p_compiler->node->size; j += j)
					LIST_PUSH(uint8_t, &p_compiler->data, 0);

				LIST_PUSH(uint8_t, &p_compiler->data, (uint8_t)data[i]);
			}
		}

		break;

	default: INTERNAL_BUG; /* the parser should not allow any unexpected tokens through */
	}

	list_push(&p_compiler->data_consts, &data_const);
}

void compiler_compile_inst(compiler_t *p_compiler) {
	size_t argc = 0;
	if (p_compiler->node->right != NULL)
		argc = 1;

	if (p_compiler->node->left != NULL) {
		argc = 2;

		if (p_compiler->node->right == NULL)
			INTERNAL_BUG;
	}

	for (size_t i = 0; i < SIZE_OF(g_inst_func_map); ++ i) {
		if (g_inst_func_map[i].key == p_compiler->node->tok->type) {
			LIST_PUSH(inst_t, &p_compiler->insts, g_inst_func_map[i].func(p_compiler, argc));

			return;
		}
	}
}

void compiler_compile_label(compiler_t *p_compiler) {
	label_t label = {
		.name = copy_str(p_compiler->node->tok->data),
		.addr = p_compiler->insts.count
	};

	list_push(&p_compiler->labels, &label);
}

inst_t compiler_compile_inst_none(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "none";
	inst_t inst = {.opcode = OPCODE_NONE};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	return inst;
}

inst_t compiler_compile_inst_move(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "move";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_MOVE_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_MOVE;
	}

	return inst;
}

inst_t compiler_compile_inst_write(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "write";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data = arg.value.reg;

		switch (p_compiler->node->size) {
		case 8:  inst.opcode = OPCODE_WRITE_R_8;  break;
		case 16: inst.opcode = OPCODE_WRITE_R_16; break;
		case 32: inst.opcode = OPCODE_WRITE_R_32; break;
		case 64: inst.opcode = OPCODE_WRITE_R_64; break;

		default: INTERNAL_BUG;
		}
	} else {
		inst.data = arg.value.num;

		switch (p_compiler->node->size) {
		case 8:  inst.opcode = OPCODE_WRITE_8;  break;
		case 16: inst.opcode = OPCODE_WRITE_16; break;
		case 32: inst.opcode = OPCODE_WRITE_32; break;
		case 64: inst.opcode = OPCODE_WRITE_64; break;

		default: INTERNAL_BUG;
		}
	}

	return inst;
}

inst_t compiler_compile_inst_read(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "read";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data = arg.value.reg;

		switch (p_compiler->node->size) {
		case 8:  inst.opcode = OPCODE_READ_R_8;  break;
		case 16: inst.opcode = OPCODE_READ_R_16; break;
		case 32: inst.opcode = OPCODE_READ_R_32; break;
		case 64: inst.opcode = OPCODE_READ_R_64; break;

		default: INTERNAL_BUG;
		}
	} else {
		inst.data = arg.value.num;

		switch (p_compiler->node->size) {
		case 8:  inst.opcode = OPCODE_READ_8;  break;
		case 16: inst.opcode = OPCODE_READ_16; break;
		case 32: inst.opcode = OPCODE_READ_32; break;
		case 64: inst.opcode = OPCODE_READ_64; break;

		default: INTERNAL_BUG;
		}
	}

	return inst;
}

inst_t compiler_compile_inst_push(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "push";
	inst_t inst = {0};

	if (p_argc != 1) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->left, name);
	if (arg.is_reg) {
		inst.reg = arg.value.reg;

		switch (p_compiler->node->size) {
		case 8:  inst.opcode = OPCODE_PUSH_R_8;  break;
		case 16: inst.opcode = OPCODE_PUSH_R_16; break;
		case 32: inst.opcode = OPCODE_PUSH_R_32; break;
		case 64: inst.opcode = OPCODE_PUSH_R_64; break;

		default: INTERNAL_BUG;
		}
	} else {
		inst.data = arg.value.num;

		switch (p_compiler->node->size) {
		case 8:  inst.opcode = OPCODE_PUSH_8;  break;
		case 16: inst.opcode = OPCODE_PUSH_16; break;
		case 32: inst.opcode = OPCODE_PUSH_32; break;
		case 64: inst.opcode = OPCODE_PUSH_64; break;

		default: INTERNAL_BUG;
		}
	}

	return inst;
}

inst_t compiler_compile_inst_pusha(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "pusha";
	inst_t inst = {.opcode = OPCODE_PUSH_A};

	if (p_argc != 0) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	return inst;
}

inst_t compiler_compile_inst_pop(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "pop";
	inst_t inst = {0};

	if (p_argc == 0) {
		switch (p_compiler->node->size) {
		case 8:  inst.opcode = OPCODE_POP_8;  break;
		case 16: inst.opcode = OPCODE_POP_16; break;
		case 32: inst.opcode = OPCODE_POP_32; break;
		case 64: inst.opcode = OPCODE_POP_64; break;

		default: INTERNAL_BUG;
		}
	} else if (p_argc == 1) {
		inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

		switch (p_compiler->node->size) {
		case 8:  inst.opcode = OPCODE_POP_R_8;  break;
		case 16: inst.opcode = OPCODE_POP_R_16; break;
		case 32: inst.opcode = OPCODE_POP_R_32; break;
		case 64: inst.opcode = OPCODE_POP_R_64; break;

		default: INTERNAL_BUG;
		}
	} else {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 1 or 0 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	return inst;
}

inst_t compiler_compile_inst_popa(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "popa";
	inst_t inst = {.opcode = OPCODE_POP_A};

	if (p_argc != 0) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	return inst;
}

inst_t compiler_compile_inst_eq(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "eq";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_EQ_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_EQ;
	}

	return inst;
}

inst_t compiler_compile_inst_neq(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "neq";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_NEQ_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_NEQ;
	}

	return inst;
}

inst_t compiler_compile_inst_gt(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "gt";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_GT_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_GT;
	}

	return inst;
}

inst_t compiler_compile_inst_ge(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "ge";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_GE_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_GE;
	}

	return inst;
}

inst_t compiler_compile_inst_lt(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "lt";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_LT_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_LT;
	}

	return inst;
}

inst_t compiler_compile_inst_le(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "le";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_LE_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_LE;
	}

	return inst;
}

inst_t compiler_compile_inst_jump(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "jump";
	inst_t inst = {0};

	if (p_argc != 1) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->left, name);
	if (arg.is_reg) {
		inst.reg    = arg.value.reg;
		inst.opcode = OPCODE_JUMP_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_JUMP;
	}

	return inst;
}

inst_t compiler_compile_inst_jumpt(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "jumpt";
	inst_t inst = {0};

	if (p_argc != 1) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->left, name);
	if (arg.is_reg) {
		inst.reg    = arg.value.reg;
		inst.opcode = OPCODE_JUMP_T_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_JUMP_T;
	}

	return inst;
}

inst_t compiler_compile_inst_jumpf(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "jumpf";
	inst_t inst = {0};

	if (p_argc != 1) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->left, name);
	if (arg.is_reg) {
		inst.reg    = arg.value.reg;
		inst.opcode = OPCODE_JUMP_F_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_JUMP_F;
	}

	return inst;
}

inst_t compiler_compile_inst_add(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "add";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_ADD_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_ADD;
	}

	return inst;
}

inst_t compiler_compile_inst_inc(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "inc";
	inst_t inst = {.opcode = OPCODE_INC};

	if (p_argc != 1) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	return inst;
}

inst_t compiler_compile_inst_sub(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "sub";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_SUB_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_SUB;
	}

	return inst;
}

inst_t compiler_compile_inst_dec(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "dec";
	inst_t inst = {.opcode = OPCODE_DEC};

	if (p_argc != 1) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	return inst;
}

inst_t compiler_compile_inst_mult(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "mult";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_MULT_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_MULT;
	}

	return inst;
}

inst_t compiler_compile_inst_div(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "div";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_DIV_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_DIV;
	}

	return inst;
}

inst_t compiler_compile_inst_mod(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "mod";
	inst_t inst = {0};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->right, name);
	if (arg.is_reg) {
		inst.data   = arg.value.reg;
		inst.opcode = OPCODE_MOD_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_MOD;
	}

	return inst;
}

inst_t compiler_compile_inst_rshift(compiler_t *p_compiler, size_t p_argc) {
	(void)p_compiler; (void)p_argc;

	assert(0 && "Instruction unimplemented");
}

inst_t compiler_compile_inst_lshift(compiler_t *p_compiler, size_t p_argc) {
	(void)p_compiler; (void)p_argc;

	assert(0 && "Instruction unimplemented");
}

inst_t compiler_compile_inst_and(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "and";
	inst_t inst = {.opcode = OPCODE_AND};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg  = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);
	inst.data = compiler_get_reg_arg(p_compiler, p_compiler->node->right, name);

	return inst;
}

inst_t compiler_compile_inst_or(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "or";
	inst_t inst = {.opcode = OPCODE_OR};

	if (p_argc != 2) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg  = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);
	inst.data = compiler_get_reg_arg(p_compiler, p_compiler->node->right, name);

	return inst;
}

inst_t compiler_compile_inst_not(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "not";
	inst_t inst = {.opcode = OPCODE_NOT};

	if (p_argc != 1) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	inst.reg = compiler_get_reg_arg(p_compiler, p_compiler->node->left, name);

	return inst;
}

inst_t compiler_compile_inst_bitand(compiler_t *p_compiler, size_t p_argc) {
	(void)p_compiler; (void)p_argc;

	assert(0 && "Instruction unimplemented");
}

inst_t compiler_compile_inst_bitor(compiler_t *p_compiler, size_t p_argc) {
	(void)p_compiler; (void)p_argc;

	assert(0 && "Instruction unimplemented");
}

inst_t compiler_compile_inst_call(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "call";
	inst_t inst = {0};

	if (p_argc != 1) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->left, name);
	if (arg.is_reg) {
		inst.reg    = arg.value.reg;
		inst.opcode = OPCODE_CALL_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_CALL;
	}

	return inst;
}

inst_t compiler_compile_inst_callt(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "callt";
	inst_t inst = {0};

	if (p_argc != 1) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->left, name);
	if (arg.is_reg) {
		inst.reg    = arg.value.reg;
		inst.opcode = OPCODE_CALL_T_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_CALL_T;
	}

	return inst;
}

inst_t compiler_compile_inst_callf(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "callf";
	inst_t inst = {0};

	if (p_argc != 1) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->left, name);
	if (arg.is_reg) {
		inst.reg    = arg.value.reg;
		inst.opcode = OPCODE_CALL_F_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_CALL_F;
	}

	return inst;
}

inst_t compiler_compile_inst_ret(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "ret";
	inst_t inst = {.opcode = OPCODE_RET};

	if (p_argc != 0) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	return inst;
}

inst_t compiler_compile_inst_syscall(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "syscall";
	inst_t inst = {0};

	if (p_argc != 1) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	arg_any_t arg = compiler_get_any_arg(p_compiler, p_compiler->node->left, name);
	if (arg.is_reg) {
		inst.reg    = arg.value.reg;
		inst.opcode = OPCODE_SYSCALL_R;
	} else {
		inst.data   = arg.value.num;
		inst.opcode = OPCODE_SYSCALL;
	}

	return inst;
}

inst_t compiler_compile_inst_halt(compiler_t *p_compiler, size_t p_argc) {
	const char *name = "halt";
	inst_t inst = {.opcode = OPCODE_HALT};

	if (p_argc != 0) {
		error(&p_compiler->node->tok->loc, QUOTES("%s")" expects 2 arguments, got %li",
		      name, (long)p_argc);

		aborted();
	}

	return inst;
}

word_t compiler_get_num_arg(compiler_t *p_compiler, node_t *p_node, const char *p_inst) {
	switch (p_node->type) {
	case NODE_TYPE_NUM: return p_node->data.num;
	case NODE_TYPE_ID:
		/* is the identifier a const? */
		for (size_t i = 0; i < p_compiler->data_consts.count; ++ i) {
			data_const_t *data_const = LIST_AT(data_const_t, &p_compiler->data_consts, i);

			if (strcmp(data_const->name, p_node->tok->data) == 0)
				return data_const->addr;
		}

		/* is the identifier a label? */
		for (size_t i = 0; i < p_compiler->labels.count; ++ i) {
			label_t *label = LIST_AT(label_t, &p_compiler->labels, i);

			if (strcmp(label->name, p_node->tok->data) == 0)
				return label->addr;
		}

		error(&p_node->tok->loc, "Identifier "QUOTES("%s")" not defined",
		      p_node->tok->data);

		aborted();

		break;

	default:
		error(&p_node->tok->loc, QUOTES("%s")" arg expected to be "QUOTES("%s")", got "QUOTES("%S"),
		      p_inst, node_type_to_str(NODE_TYPE_NUM), node_type_to_str(p_node->type));

		aborted();
	}

	return 0; /* for the compiler so it will not complain */
}

reg_t compiler_get_reg_arg(compiler_t *p_compiler, node_t *p_node, const char *p_inst) {
	(void)p_compiler; /* we dont use p_compiler, but we still require it for consistency :) */

	switch (p_node->type) {
	case NODE_TYPE_REG: return (reg_t)p_node->data.num; break;

	default:
		error(&p_node->tok->loc, QUOTES("%s")" arg expected to be "QUOTES("%s")", got "QUOTES("%s"),
		      p_inst, node_type_to_str(NODE_TYPE_REG), node_type_to_str(p_node->type));

		aborted();
	}

	return 0;
}

arg_any_t compiler_get_any_arg(compiler_t *p_compiler, node_t *p_node, const char *p_inst) {
	(void)p_compiler;

	arg_any_t arg = {0};

	switch (p_node->type) {
	case NODE_TYPE_REG:
		arg.is_reg    = true;
		arg.value.reg = compiler_get_reg_arg(p_compiler, p_node, p_inst);

		break;

	case NODE_TYPE_NUM: case NODE_TYPE_ID:
		arg.is_reg    = false;
		arg.value.num = compiler_get_num_arg(p_compiler, p_node, p_inst);

		break;

	default:
		error(&p_node->tok->loc, QUOTES("%s")" arg expected to be "QUOTES("%s")
		      " or "QUOTES("%s")", got "QUOTES("%S"),
		      p_inst, node_type_to_str(NODE_TYPE_NUM), node_type_to_str(NODE_TYPE_REG),
		      node_type_to_str(p_node->type));

		aborted();
	}

	return arg;
}

void label_free(label_t *p_label) {
	free(p_label->name);
}

void data_const_free(data_const_t *p_data_const) {
	free(p_data_const->name);
}
