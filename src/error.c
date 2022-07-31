#include "error.h"

void fatal(const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	fprintf(stderr, "Error: %s\nTry 'tmasm -h'\n", msg);

	exit(EXIT_FAILURE);
}

void error_at(size_t p_line_num, size_t p_ch_num, const char *p_line,
              const char *p_path, const char *p_title, const char *p_msg) {
	/* generating nice gcc-like errors :) */
	fprintf(stderr, "%s: %s\n  -> at %s:%lu:%lu\n", p_title, p_msg,
	        p_path, (unsigned long)p_line_num, (unsigned long)p_ch_num);

	char   line_num_str[32];
	size_t line_num_str_len = snprintf(line_num_str, sizeof(line_num_str),
	                                   "%lu", (unsigned long)p_line_num);

	size_t ptr_off = 0; /* character pointer offset */
	for (size_t i = 0; i < p_ch_num; ++ i) {
		if (p_line[i] == '\t')
			ptr_off += 3; /* tabs are always 4 spaces when you output them, right? */
	}

	char err_ptr[strlen(p_line) + sizeof(line_num_str)];
	memset(err_ptr, ' ', sizeof(err_ptr));

	ptr_off += line_num_str_len + p_ch_num + 2;

	err_ptr[line_num_str_len + 1] = '|';
	err_ptr[ptr_off]     = '^';
	err_ptr[ptr_off + 1] = '\0';

	fprintf(stderr, "    %s | ", line_num_str);

	/* outputting all of the chars manually because of tabs */
	for (const char *ch = p_line; *ch != '\0'; ++ ch) {
		if (*ch == '\t')
			fputs("    ", stderr);
		else
			fputc(*ch, stderr);
	}


	fprintf(stderr, "\n    %s\n", err_ptr);

	fflush(stderr);
}
