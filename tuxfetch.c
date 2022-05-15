#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <proc/sysinfo.h>

const char *tux_ascii =
	    " ------------------------------\n"
	    "      \\\n"
	    "       \\\n"
	    "           .--.\n"
	    "          |o_o |\n"
	    "          |:_/ |\n"
	    "         //   \\ \\\n"
	    "        (|     | )\n"
	    "       /'\\_   _/`\\\n"
	    "       \\___)=(___/\n\n";

static char *
get_kernel_release(void) {
	struct utsname utsname;

	if (uname(&utsname) == -1) {
		fprintf(stderr, "tuxfetch: uname failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	return strdup(utsname.release);
}

static char *
get_shell(void) {
	uid_t uid;
	struct passwd *pw;

	uid = geteuid();
	pw = getpwuid(uid);

	return pw->pw_shell;
}

static char *
get_os_name(void) {
	struct utsname utsname;
	FILE *fp;
	char *line;
	char *os_name, *machine, *os_name_and_machine;
	size_t buff_size;

	if (uname(&utsname) == -1) {
		fprintf(stderr, "tuxfetch: uname failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	machine = utsname.machine;

	if (NULL == (fp = fopen("/etc/os-release", "r"))) {
		fprintf(
			stderr,
			"tuxfetch: failed to open /etc/os-release: %s\n",
			strerror(errno)
		);

		exit(EXIT_FAILURE);
	}

	os_name = line = NULL;
	buff_size = 0;

	while (getline(&line, &buff_size, fp) != -1) {
		if (strncmp(line, "PRETTY_NAME=\"", sizeof("PRETTY_NAME=\"") - 1) == 0) {
			char *start, *end;

			/* search for the opening quote */
			if (NULL == (start = strchr(line, '"'))) {
				fputs("tuxfetch: invalid format /etc/os-release\n", stderr);
				exit(EXIT_FAILURE);
			}

			/* skip quote char */
			++start;

			/* search for the closing quote */
			if (NULL == (end = strchr(start, '"'))) {
				fputs("tuxfetch: invalid format /etc/os-release\n", stderr);
				exit(EXIT_FAILURE);
			}

			if (NULL == (os_name = malloc((end - start) + 1))) {
				fputs("tuxfetch: malloc failed\n", stderr);
				exit(EXIT_FAILURE);
			}

			memcpy(os_name, start, end - start);
			os_name[end - start] = '\0';

			break;
		}
	}

	fclose(fp);
	free(line);

	if (NULL == os_name) {
		fputs("tuxfetch: PRETTY_NAME key not found in /etc/os-release\n", stderr);
		exit(EXIT_FAILURE);
	}

	os_name_and_machine = malloc(strlen(os_name) + strlen(machine) + 2);

	if (NULL == os_name_and_machine) {
		fputs("tuxfetch: malloc failed\n", stderr);
		exit(EXIT_FAILURE);
	}

	sprintf(os_name_and_machine, "%s %s", os_name, machine);

	free(os_name);

	return os_name_and_machine;
}

static char *
get_uptime(void) {
	struct sysinfo s_info;
	long hours, minutes, seconds;
	char *uptime;

	if (sysinfo(&s_info) == -1) {
		fprintf(stderr, "tuxfetch: sysinfo failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	hours = s_info.uptime / 3600;
	minutes = (s_info.uptime - hours * 3600) / 60;
	seconds = s_info.uptime - minutes * 60 - hours * 3600;

	if (NULL == (uptime = malloc(32))) {
		fputs("tuxfetch: malloc failed\n", stderr);
		exit(EXIT_FAILURE);
	}

	sprintf(uptime, "%dh %dm %ds", hours, minutes, seconds);

	return uptime;
}

static char *
get_ram_usage(void) {
	long total_ram, used_ram;
	char *ram_usage, *unit;

	meminfo();

#ifdef MEMORY_UNIT_MIB
	unit = "MiB";
	total_ram = (kb_main_total * 1024) / (1000 * 1000);
	used_ram = (kb_main_used * 1024) / (1000 * 1000);
#else
	unit = "MB";
	total_ram = kb_main_total / 1024;
	used_ram = kb_main_used / 1024;
#endif

	if (NULL == (ram_usage = malloc(32))) {
		fputs("tuxfetch: malloc failed\n", stderr);
		exit(EXIT_FAILURE);
	}

	sprintf(ram_usage, "%d / %d %s", used_ram, total_ram, unit);

	return ram_usage;
}

int
main(void) {
	printf("\n ------------------------------\n");

	printf("     \x1B[35mos\x1B[0m %s \n", get_os_name());
	printf("     \x1B[35mkernel\x1B[0m %s \n", get_kernel_release());
	printf("     \x1B[35muptime\x1B[0m %s \n", get_uptime());
	printf("     \x1B[35mshell\x1B[0m %s \n", get_shell());
	printf("     \x1B[35mmemory\x1B[0m %s \n", get_ram_usage());

	puts(tux_ascii);

	return EXIT_SUCCESS;
}
