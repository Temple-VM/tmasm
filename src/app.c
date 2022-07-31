#include "app.h"

/* TODO: make the arguments work more like in gcc */
int read_args(int p_argc, char **p_argv) {
	if (p_argc > 1) {
		if (strcmp(p_argv[1], "-h") == 0 || strcmp(p_argv[1], "--help") == 0) {
			if (p_argc > 2)
				fatal("Unexpected second argument '%s'", p_argv[2]);

			help();
		} else if (strcmp(p_argv[1], "-v") == 0 || strcmp(p_argv[1], "-version") == 0) {
			if (p_argc > 2)
				fatal("Unexpected second argument '%s'", p_argv[2]);

			version();
		} else {
			if (p_argc > 3)
				fatal("Unexpected third argument '%s'", p_argv[3]);
			if (p_argc > 2)
				assemble(p_argv[1], p_argv[2]);
			else {
				size_t len = strlen(p_argv[1]);
				char  *out_path = (char*)malloc(len + 2); /* 3 extra bytes for the .tm extension */
				if (out_path == NULL)
					assert(0 && "malloc fail");

				strcpy(out_path, p_argv[1]);

				size_t last_dot = len;
				for (size_t i = 1; i < len; ++ i) {
					if (out_path[i] == '.')
						last_dot = i;
				}

				out_path[last_dot]     = '.'; /* in case the file name has no extension */
				out_path[last_dot + 1] = 't';
				out_path[last_dot + 2] = 'm';
				out_path[last_dot + 3] = '\0';

				assemble(p_argv[1], out_path);

				free(out_path);
			}
		}
	} else
		help();

	return 0;
}

void help(void) {
	puts(" ________  ___  ___   ________  ___\n"
         "|_   _|  \\/  | / _ \\ /  ___|  \\/  |\n"
         "  | | | .  . |/ /_\\ \\\\ `--.| .  . |\n"
         "  | | | |\\/| ||  _  | `--. \\ |\\/| |\n"
         "  | | | |  | || | | |/\\__/ / |  | |\n"
         "  \\_/ \\_|  |_/\\_| |_/\\____/\\_|  |_/\n"
	     "\n"
	     "Github: https://github.com/lordoftrident/tmasm\n"
	     "\n"
	     "Usage: tmasm [OPTIONS] [FILE [OUT]]\n"
	     "OPTIONS:\n"
	     "  -h, --help,    - show this message\n"
	     "  -v, --version  - show the version");
}

void version(void) {
	printf("TMASM %i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

void assemble(const char *p_source_path, const char *p_out_path) {
	compile(p_source_path, p_out_path);

	if (chmod(p_out_path, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP |
	                      S_IXGRP | S_IROTH | S_IXOTH) != 0)
		fatal("Could not make '%s' executable", p_out_path);
}
