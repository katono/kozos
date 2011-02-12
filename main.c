#include "defines.h"
#include "serial.h"
#include "xmodem.h"
#include "lib.h"


static int init(void)
{
	extern int erodata, data_start, edata, bss_start, ebss;

	memcpy(&data_start, &erodata, (long) &edata - (long) &data_start);
	memset(&bss_start, 0, (long) &ebss - (long) &bss_start);

	serial_init(SERIAL_DEFAULT_DEVICE);
}

int global_data = 0x10;
int global_bss;
static int static_data = 0x20;
static int static_bss;


static void printval(void)
{
	printf("global_data = 0x%x\n", global_data);
	printf("global_bss  = 0x%x\n", global_bss);
	printf("static_data = 0x%x\n", static_data);
	printf("static_bss  = 0x%x\n", static_bss);
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
	long i = 0xFFFF;
	init();

	printf("Hello, World!\n");
	printf("%d, 0x%x\n", 100, 100);

	printval();
	puts("overwrite variables.\n");
	global_data = 0x20;
	global_bss = 0x30;
	static_data = 0x40;
	static_bss = 0x50;
	printval();
	printf("%lx, %p, %#lx, %d\n", 0x1234L, main, i, 999);

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
		} else {
			puts("unknown.\n");
		}
	}

	return 0;
}
