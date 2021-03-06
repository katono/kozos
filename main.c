#include "defines.h"
#include "interrupt.h"
#include "serial.h"
#include "xmodem.h"
#include "elf.h"
#include "lib.h"
#include "dram.h"


static int init(void)
{
	extern int erodata, data_start, edata, bss_start, ebss;

	memcpy(&data_start, &erodata, (long) &edata - (long) &data_start);
	memset(&bss_start, 0, (long) &ebss - (long) &bss_start);

	softvec_init();

	serial_init(SERIAL_DEFAULT_DEVICE);
	dram_init();
}


static void wait(void)
{
	volatile long i;
	for (i = 0; i < 300000; i++) ;
}

int main(void)
{
	char buf[16];
	long size = -1;
	unsigned char *loadbuf = NULL;
	extern int buffer_start;
	char *entry_point;
	void (*f)(void);

	INTR_DISABLE;

	init();

	printf("kzload (kozos boot loader) started.\n");

	while (1) {
		puts("kzload> ");
		gets(buf, sizeof buf);

		if (!strcmp(buf, "load")) {
			loadbuf = (char *) &buffer_start;
			size = xmodem_recv(loadbuf);
			wait();
			if (size < 0) {
				puts("\nXMODEM receive error!\n");
			} else {
				puts("\nXMODEM receive succeeded.\n");
			}
		} else if (!strcmp(buf, "dump")) {
			printf("size: %d(%#x) bytes\n", size, size);
			hexdump(loadbuf, size);
		} else if (!strcmp(buf, "run")) {
			entry_point = elf_load(loadbuf);
			if (!entry_point) {
				printf("run error!\n");
			} else {
				printf("starting from entry point: %p\n", entry_point);
				f = (void (*)(void)) entry_point;
				f();
			}
		} else if (!strcmp(buf, "ramchk")) {
			dram_check();
		} else {
			puts("unknown.\n");
		}
	}

	return 0;
}
