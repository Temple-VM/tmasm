#ifndef ERROR_H__HEADER_GUARD__
#define ERROR_H__HEADER_GUARD__

#include <stdlib.h> /* exit, EXIT_FAILURE */
#include <stdio.h>  /* fprintf, snprintf, fflush, stderr, fputs, fputc */
#include <stdarg.h> /* va_list, va_start, va_end, vsnprintf */
#include <string.h> /* strlen */

void fatal(const char *p_fmt, ...);
void error_at(size_t p_line_num, size_t p_ch_num, const char *p_line,
              const char *p_path, const char *p_title, const char *p_msg);

#endif
