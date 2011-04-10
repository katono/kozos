#include "defines.h"

void start(void);
void intr_softerr(void);
void intr_syscall(void);
void intr_serintr(void);

void (*vectors[])(void) = {
	start,	/*  0 */
	NULL, 	/*  1 */
	NULL, 	/*  2 */
	NULL, 	/*  3 */
	NULL, 	/*  4 */
	NULL, 	/*  5 */
	NULL, 	/*  6 */
	NULL, 	/*  7 */
	intr_syscall, 	/*  8 */
	intr_softerr, 	/*  9 */
	intr_softerr, 	/* 10 */
	intr_softerr, 	/* 11 */
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
	intr_serintr, 	/* 52: SCI0 ERI0 */
	intr_serintr, 	/* 53: SCI0 RXI0 */
	intr_serintr, 	/* 54: SCI0 TXI0 */
	intr_serintr, 	/* 55: SCI0 TEI0 */
	intr_serintr, 	/* 56: SCI1 ERI1 */
	intr_serintr, 	/* 57: SCI1 RXI1 */
	intr_serintr, 	/* 58: SCI1 TXI1 */
	intr_serintr, 	/* 59: SCI1 TEI1 */
	intr_serintr, 	/* 60 SCI2 ERI1 */
	intr_serintr, 	/* 61 SCI2 RXI1 */
	intr_serintr, 	/* 62 SCI2 TXI1 */
	intr_serintr, 	/* 63 SCI2 TEI1 */
};
