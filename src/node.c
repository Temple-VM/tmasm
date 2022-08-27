#include "node.h"

#ifdef TMASM_DEBUG
static void node_dump_indent(node_t *p_node, size_t p_indent) {
	for (size_t i = 0; i < p_indent; ++ i)
		fputs("    ", stdout);

	switch (p_node->type) {
	case NODE_TYPE_NUM:
		fprintclrf(stdout, "[\x1by%s: \x1bn%li\x1bX]\n",
		           node_type_to_str(p_node->type), (long)p_node->data.num);

		break;

	default:
		fprintclrf(stdout, "[\x1by%s: \x1bn%s\x1bX]\n",
		           node_type_to_str(p_node->type), p_node->tok->data);
	}

	++ p_indent;

	if (p_node->left != NULL)
		node_dump_indent(p_node->left, p_indent);

	if (p_node->right != NULL)
		node_dump_indent(p_node->right, p_indent);
}

void node_dump(node_t *p_node) {
	node_dump_indent(p_node, 0);
}
#endif

const char *node_type_to_str(node_type_t p_type) {
	switch (p_type) {
	case NODE_TYPE_INST:   return "instruction";
	case NODE_TYPE_DATA:   return "data";
	case NODE_TYPE_DEFINE: return "define";
	case NODE_TYPE_LABEL:  return "label";

	case NODE_TYPE_REG: return "register";
	case NODE_TYPE_NUM: return "number";
	case NODE_TYPE_STR: return "string";
	case NODE_TYPE_ID:  return "identifier";

	case NODE_TYPE_INT64_TYPE: return "INT64";
	case NODE_TYPE_INT32_TYPE: return "INT32";
	case NODE_TYPE_INT16_TYPE: return "INT16";
	case NODE_TYPE_BYTE_TYPE:  return "BYTE";
	case NODE_TYPE_WORD_TYPE:  return "WORD";

	default: INTERNAL_BUG;
	}
}

node_t *node_new(token_t *p_tok) {
	node_t *node;
	SMEMALLOC(node, 1);

	node->tok   = p_tok;
	node->size  = 0;
	node->left  = NULL;
	node->right = NULL;

	switch (p_tok->type) {
	case TOKEN_TYPE_HEX:
		node->type     = NODE_TYPE_NUM;
		node->data.num = (uint64_t)strtol(p_tok->data, NULL, 16);

		break;

	case TOKEN_TYPE_DEC:
		node->type     = NODE_TYPE_NUM;
		node->data.num = (uint64_t)strtol(p_tok->data, NULL, 10);

		break;

	case TOKEN_TYPE_CH:
		node->type     = NODE_TYPE_NUM;
		node->data.num = (uint64_t)p_tok->data[0];

		break;

	case TOKEN_TYPE_STR: node->type = NODE_TYPE_STR; break;
	case TOKEN_TYPE_ID:  node->type = NODE_TYPE_ID;  break;

	case TOKEN_TYPE_INT64_TYPE: node->type = NODE_TYPE_INT64_TYPE; break;
	case TOKEN_TYPE_INT32_TYPE: node->type = NODE_TYPE_INT32_TYPE; break;
	case TOKEN_TYPE_INT16_TYPE: node->type = NODE_TYPE_INT16_TYPE; break;
	case TOKEN_TYPE_BYTE_TYPE:  node->type = NODE_TYPE_BYTE_TYPE;  break;
	case TOKEN_TYPE_WORD_TYPE:  node->type = NODE_TYPE_WORD_TYPE;  break;

	case TOKEN_TYPE_LABEL:  node->type = NODE_TYPE_LABEL;  break;
	case TOKEN_TYPE_DATA:   node->type = NODE_TYPE_DATA;   break;
	case TOKEN_TYPE_DEFINE: node->type = NODE_TYPE_DEFINE; break;

	default:
		if (token_type_is_inst(p_tok->type))
			node->type = NODE_TYPE_INST;
		else if (token_type_is_reg(p_tok->type)) {
			node->type     = NODE_TYPE_REG;
			node->data.num = p_tok->type - TOKEN_TYPE_REG_1;
		} else
			INTERNAL_BUG;
	}

	return node;
}

void node_free_tree(node_t *p_node) {
	if (p_node->left != NULL)
		node_free_tree(p_node->left);

	if (p_node->right != NULL)
		node_free_tree(p_node->right);

	free(p_node);
}
