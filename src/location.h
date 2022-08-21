#ifndef LOCATION_H__HEADER_GUARD__
#define LOCATION_H__HEADER_GUARD__

#include <stdlib.h>

typedef struct {
	size_t row, col, tok_len;
	char  *line, *file;
} location_t;

#endif
