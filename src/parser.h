#ifndef PARSER_H__HEADER_GUARD__
#define PARSER_H__HEADER_GUARD__

#include <stdbool.h> /* bool, true, false */
#include <stdint.h>  /* uint64_t */
#include <stdlib.h>  /* size_t */

#include "token.h"
#include "list.h"
#include "lexer.h"
#include "node.h"
#include "target.h"

typedef struct {
	lexer_t lexer;

	token_t *tok;
	list_t   toks;

	size_t i;
} parser_t;

extern bool g_insts_have_size_variants[];

bool inst_has_size_variant(token_type_t p_inst);

parser_t parser_new(const char *p_path);
void     parser_free(parser_t *p_parser);

node_t *parser_parse_next(parser_t *p_parser);

node_t *parser_parse_data(parser_t *p_parser);
node_t *parser_parse_inst(parser_t *p_parser);
node_t *parser_parse_label(parser_t *p_parser);

void parser_next_token(parser_t *p_parser);
bool parser_at_end(parser_t *p_parser);

#endif
