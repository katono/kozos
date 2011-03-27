#include "defines.h"
#include "serial.h"
#include "lib.h"
#include "dram.h"


int main(void)
{
	char buf[32];

	printf("Hello World!\n");

	while (1) {
		puts("> ");
		gets(buf, sizeof buf);

		if (!strncmp(buf, "echo", 4)) {
			printf("%s\n", &buf[4]);
		} else if (!strcmp(buf, "exit")) {
			break;
		} else if (!strcmp(buf, "ramchk")) {
			dram_check();
		} else {
			puts("unknown.\n");
		}
	}

	return 0;
}
