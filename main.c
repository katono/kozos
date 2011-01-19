#include "defines.h"
#include "serial.h"
#include "lib.h"


static int init(void)
{
	serial_init(SERIAL_DEFAULT_DEVICE);
}

int main(void)
{
	init();

	puts("Hello, World!\n");
	while (1);

	return 0;
}
