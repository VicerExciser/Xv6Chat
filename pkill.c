#include "types.h"
#include "stat.h"
#include "user.h"

#define NAME_LEN 16

int
main(int argc, char **argv)
{
	int i, len = 0;
	if (argc < 2) {
		printf(2, "usage: kill <proc_name>\n");
		exit();
	}

	char *pname = (char*)malloc(NAME_LEN*sizeof(char));

	for (i = 1; i < argc; i++) {
		len += strlen(argv[i]);
		if (len < NAME_LEN) {
			strcat(pname, argv[i]);
			if (i != argc-1) {
				pname[len++] = ' ';
				// len++;
			}
		}
		else {
			printf(2, "pkill err: process name too long!\n");
			exit();
		}
	}

	// printf(1, "killing off '%s'\n", pname);
	pkill(pname);
	free(pname);
	exit();
}
