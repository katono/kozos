#include "defines.h"
#include "serial.h"
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
	PRINTF1("global_data = 0x%x\n", global_data);
	PRINTF1("global_bss  = 0x%x\n", global_bss);
	PRINTF1("static_data = 0x%x\n", static_data);
	PRINTF1("static_bss  = 0x%x\n", static_bss);
}

int main(void)
{
	init();

	PRINTF0("Hello, World!\n");
	PRINTF2("%d, 0x%x\n", 100, 100);

	printval();
	puts("overwrite variables.\n");
	global_data = 0x20;
	global_bss = 0x30;
	static_data = 0x40;
	static_bss = 0x50;
	printval();

	while (1);

	return 0;
}
