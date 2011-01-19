#include "defines.h"

void start(void);
void int_sci_rxi0(void);
void int_sci_rxi1(void);
void int_sci_txi0(void);
void int_sci_txi1(void);

void (*vectors[])(void) = {
	start,	/*  0 */
	NULL, 	/*  1 */
	NULL, 	/*  2 */
	NULL, 	/*  3 */
	NULL, 	/*  4 */
	NULL, 	/*  5 */
	NULL, 	/*  6 */
	NULL, 	/*  7 */
	NULL, 	/*  8 */
	NULL, 	/*  9 */
	NULL, 	/* 10 */
	NULL, 	/* 11 */
	NULL, 	/* 12 */
	NULL, 	/* 13 */
	NULL, 	/* 14 */
	NULL, 	/* 15 */
	NULL, 	/* 16 */
	NULL, 	/* 17 */
	NULL, 	/* 18 */
	NULL, 	/* 19 */
	NULL, 	/* 20 */
	NULL, 	/* 21 */
	NULL, 	/* 22 */
	NULL, 	/* 23 */
	NULL, 	/* 24 */
	NULL, 	/* 25 */
	NULL, 	/* 26 */
	NULL, 	/* 27 */
	NULL, 	/* 28 */
	NULL, 	/* 29 */
	NULL, 	/* 30 */
	NULL, 	/* 31 */
	NULL, 	/* 32 */
	NULL, 	/* 33 */
	NULL, 	/* 34 */
	NULL, 	/* 35 */
	NULL, 	/* 36 */
	NULL, 	/* 37 */
	NULL, 	/* 38 */
	NULL, 	/* 39 */
	NULL, 	/* 40 */
	NULL, 	/* 41 */
	NULL, 	/* 42 */
	NULL, 	/* 43 */
	NULL, 	/* 44 */
	NULL, 	/* 45 */
	NULL, 	/* 46 */
	NULL, 	/* 47 */
	NULL, 	/* 48 */
	NULL, 	/* 49 */
	NULL, 	/* 50 */
	NULL, 	/* 51 */
#if 0
	NULL, 	/* 52: SCI0 ERI0 */
	int_sci_rxi0, 	/* 53: SCI0 RXI0 */
	int_sci_txi0, 	/* 54: SCI0 TXI0 */
	NULL, 	/* 55: SCI0 TEI0 */
	NULL, 	/* 56: SCI1 ERI1 */
	int_sci_rxi1, 	/* 57: SCI1 RXI1 */
	int_sci_txi1, 	/* 58: SCI1 TXI1 */
	NULL, 	/* 59: SCI1 TEI1 */
#else
	NULL, 	/* 52: SCI0 ERI0 */
	NULL, 	/* 53: SCI0 RXI0 */
	NULL, 	/* 54: SCI0 TXI0 */
	NULL, 	/* 55: SCI0 TEI0 */
	NULL, 	/* 56: SCI1 ERI1 */
	NULL, 	/* 57: SCI1 RXI1 */
	NULL, 	/* 58: SCI1 TXI1 */
	NULL, 	/* 59: SCI1 TEI1 */
#endif
	NULL, 	/* 60 */
	NULL, 	/* 61 */
	NULL, 	/* 62 */
	NULL, 	/* 63 */
};
