#include "defines.h"
#include "lib.h"
#include "dram.h"

#define DRAM_START 0x400000
#define DRAM_END   0x600000

#define H8_3069F_ABWCR  ((volatile uint8 *)0xfee020)
#define H8_3069F_ASTCR  ((volatile uint8 *)0xfee021)
#define H8_3069F_RTCOR  ((volatile uint8 *)0xfee02a)
#define H8_3069F_RTMCSR ((volatile uint8 *)0xfee028)
#define H8_3069F_DRCRB  ((volatile uint8 *)0xfee027)
#define H8_3069F_DRCRA  ((volatile uint8 *)0xfee026)

#define H8_3069F_P1DDR  ((volatile uint8 *)0xfee000)
#define H8_3069F_P2DDR  ((volatile uint8 *)0xfee001)
#define H8_3069F_P8DDR  ((volatile uint8 *)0xfee007)
#define H8_3069F_PBDDR  ((volatile uint8 *)0xfee00a)

#define H8_3069F_WCRH   ((volatile uint8 *)0xfee022)
#define H8_3069F_WCRL   ((volatile uint8 *)0xfee023)

typedef struct {
  union {
    volatile uint8  val8[4];
    volatile uint16 val16[2];
    volatile uint32 val32[1];
  } u;
} val_t;

int dram_init()
{
  *H8_3069F_ABWCR  = 0xff;
  *H8_3069F_RTCOR  = 0x07;
  *H8_3069F_RTMCSR = 0x37;
  *H8_3069F_DRCRB  = 0x98;
  *H8_3069F_DRCRA  = 0x30;

  *H8_3069F_P1DDR  = 0xff;
  *H8_3069F_P2DDR  = 0x07;
  *H8_3069F_P8DDR  = 0xe4;
  /* *H8_3069F_PBDDR = ...; */

  /* H8_3069F_WCRH = ...; */
  *H8_3069F_WCRL = 0xcf;

  *H8_3069F_ASTCR = 0xfb;

  return 0;
}

static int check_val(volatile val_t *p, volatile val_t *wval)
{
  volatile val_t rval;

  p->u.val8[0] = wval->u.val8[0]; p->u.val8[1] = wval->u.val8[1];
  p->u.val8[2] = wval->u.val8[2]; p->u.val8[3] = wval->u.val8[3];
  rval.u.val8[0] = p->u.val8[0]; rval.u.val8[1] = p->u.val8[1];
  rval.u.val8[2] = p->u.val8[2]; rval.u.val8[3] = p->u.val8[3];

  if (rval.u.val32[0] != wval->u.val32[0])
    return -1;

  p->u.val16[0] = wval->u.val16[0]; p->u.val16[1] = wval->u.val16[1];
  rval.u.val16[0] = p->u.val16[0]; rval.u.val16[1] = p->u.val16[1];

  if (rval.u.val32[0] != wval->u.val32[0])
    return -1;

  p->u.val32[0] = wval->u.val32[0];
  rval.u.val32[0] = p->u.val32[0];

  if (rval.u.val32[0] != wval->u.val32[0])
    return -1;

  return 0;
}

int dram_check()
{
  volatile uint32 *p;
  val_t val;

  puts("DRAM checking...\n");

  for (p = (uint32 *)DRAM_START; p < (uint32 *)DRAM_END; p++) {
/*    putxval((unsigned long)p, 8);*/

    val.u.val32[0] = (uint32)p;
    if (check_val((val_t *)p, &val) < 0)
      goto err;

    val.u.val32[0] = 0;
    if (check_val((val_t *)p, &val) < 0)
      goto err;

    val.u.val32[0] = 0xffffffffUL;
    if (check_val((val_t *)p, &val) < 0)
      goto err;

  }
  puts("\nall check OK.\n");
  return 0;

err:
  puts("\nERROR: ");
  putxval((unsigned long)*p, 8);
  puts("\n");
  return -1;
}
