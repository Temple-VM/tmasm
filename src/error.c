#include "error.h"

static void print_error_location(location_t *p_loc) {
	/* generating nice gcc-like errors :) */

	/* calculation and stuff */
	char   row_str[32];
	size_t row_str_len = snprintf(row_str, sizeof(row_str), "%lu", (unsigned long)p_loc->row);

	size_t ptr_off = 0; /* character pointer offset */
	for (size_t i = 0; i < p_loc->col; ++ i) {
		if (p_loc->line[i] == '\t')
			ptr_off += 3; /* tabs are always 4 spaces when you output them, right? */
	}

	char ptr_str[strlen(p_loc->line) + sizeof(row_str)];
	memset(ptr_str, ' ', sizeof(ptr_str));

	ptr_off += p_loc->col;
	ptr_str[ptr_off - 1] = '^';
	ptr_str[ptr_off]     = '\0';

	/* printing the error */
	fprintclrf(stderr, "  \x1bW-> At %s:%lu:%lu\x1bX\n", p_loc->file,
	           (unsigned long)p_loc->row, (unsigned long)p_loc->col); /* -> At FILE:ROW:COL */
	fprintclrf(stderr, "    \x1bW%s |\x1bX ", row_str);               /*   ROW | LINE */

	/* outputting all of the chars manually because of tabs */
	for (char *ch = p_loc->line; *ch != '\0'; ++ ch) {
		switch (*ch) {
		case '\n': break;
		case '\t': fputs("    ", stderr); break;

		default: fputc(*ch, stderr); break;
		}
	}

	memset(row_str, ' ', row_str_len);
	fprintclrf(stderr, "\n    \x1bW%s |\x1bM %s", row_str, ptr_str);

	for (size_t i = 0; i < (p_loc->tok_len == 0? 0 : p_loc->tok_len - 1); ++ i)
		fputc('~', stderr);

	fprintclrf(stderr, "\x1bX\n\n");

	fflush(stderr);
}

void fatal(const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	fprintclrf(stderr, "\x1bRError: \x1bX%s\nTry 'tmasm -h'\n", msg);

	exit(EXIT_FAILURE);
}

void error(location_t *p_loc, const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	fprintclrf(stderr, "\x1bRError: \x1bX%s\n", msg);
	print_error_location(p_loc);
}

void note(location_t *p_loc, const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	fprintclrf(stderr, "\x1bNNote: \x1bX%s\n", msg);
	print_error_location(p_loc);
}

void warning(location_t *p_loc, const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	fprintclrf(stderr, "\x1bYWarning: \x1bX%s\n", msg);
	print_error_location(p_loc);
}

void aborted(void) {
	fprintclrf(stderr, "\x1bWCompilation aborted.\x1bX\n");

	exit(EXIT_FAILURE);
}
