#ifndef APP_H__HEADER_GUARD__
#define APP_H__HEADER_GUARD__

#include <stdlib.h>   /* malloc, free */
#include <stdio.h>    /* puts, printf */
#include <string.h>   /* strcmp, strcpy */
#include <assert.h>   /* assert */
#include <stdint.h>   /* uint64_t */
#include <sys/stat.h> /* chmod */

#include "error.h"
#include "compiler.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 1
#define VERSION_PATCH 0

/* TM + program size */
#define HEADER_SIZE 2 + sizeof(uint64_t)

int read_args(int p_argc, char **p_argv);

void help(void);
void version(void);

void assemble(const char *p_path, const char *p_out_path);

#endif
