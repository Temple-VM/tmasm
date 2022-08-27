#ifndef APP_H__HEADER_GUARD__
#define APP_H__HEADER_GUARD__

/* TODO: make an atexit function to free all resources at exit */

#include <stdlib.h>   /* size_t, malloc, free */
#include <string.h>   /* strcmp, strcpy */
#include <assert.h>   /* assert */
#include <stdint.h>   /* uint64_t */
#include <sys/stat.h> /* chmod, S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP,
                         S_IWGRP, S_IXGRP, S_IROTH, S_IXOTH */

#include "error.h"
#include "compiler.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 2
#define VERSION_PATCH 1

typedef struct {
	const char *source_path, *output_path;
} app_t;

void help(void);
void version(void);

void app_read_args(app_t *p_app, int p_argc, char **p_argv);
void app_compile(app_t *p_app);

#endif
