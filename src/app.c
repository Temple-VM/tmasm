#include "app.h"

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

	exit(0);
}

void version(void) {
	printf("TMASM %i.%i.%i\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

	exit(0);
}

void app_read_args(app_t *p_app, int p_argc, char **p_argv) {
	if (p_argc <= 1)
		help();

	p_app->source_path = NULL;
	p_app->output_path = "out.tm";

	for (size_t i = 1; i < (size_t)p_argc; ++ i) {
		if (strcmp(p_argv[i], "-h") == 0 || strcmp(p_argv[i], "--help") == 0)
			help();
		else if (strcmp(p_argv[i], "-v") == 0 || strcmp(p_argv[i], "--version") == 0)
			version();
		else if (p_argv[i][0] == '-' && p_argv[i][1] == 'o') {
			p_app->output_path = p_argv[i] + 2;

			if (strlen(p_app->output_path) == 0)
				fatal("Expected output file path after "QUOTES("%s"), "-o");
		} else
			p_app->source_path = p_argv[i];
	}

	if (p_app->source_path == NULL)
		fatal("No input file");
}

void app_compile(app_t *p_app) {
	compiler_t compiler = compiler_new(p_app->source_path);
	compiler_compile(&compiler, p_app->output_path);

	if (chmod(p_app->output_path, S_IRUSR | S_IWUSR | S_IXUSR |
	          S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0)
		fatal("Could not make "QUOTES("%s")" executable", p_app->output_path);
}
