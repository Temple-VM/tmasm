#ifndef ERROR_H__HEADER_GUARD__
#define ERROR_H__HEADER_GUARD__

#include <stdlib.h> /* exit, EXIT_FAILURE */
#include <stdio.h>  /* fprintf, stderr, fputc, fputs, snprintf, fflush */
#include <stdarg.h> /* va_list, va_start, va_end, vsnprintf, fputs */

#include "colors.h"
#include "location.h"

#define QUOTES(p_fmt) "\x1bW'"p_fmt"'\x1bX"

void fatal(const char *p_fmt, ...);

void error(location_t *p_loc, const char *p_fmt, ...);
void note(location_t *p_loc, const char *p_fmt, ...);
void warning(location_t *p_loc, const char *p_fmt, ...);

void aborted(void);

#endif
