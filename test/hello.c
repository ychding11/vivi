/*
 * test code for pxa250's PREMIUM
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2002/08/02 03:02:43 $ 
 *
 * $Revision: 1.5 $
 */
#include "config.h"
#include "machine.h"
#include <printk.h>
#include <hardware.h>

#define ADDR_START	0x00200000
#define ADDR_OFFSET	0x100
#define MTD_THIS	__REG(ADDR_START + ADDR_OFFSET)
#define MTD_OK		0x00800080

#include "mmu.h"

int main() {
    unsigned long i, stat;
    volatile unsigned long tlb_base;
	
    printk("hello world\n");

    {	/* print MMU table */
      /* read Translation Table Base */
      __asm__(
	      "mrc	p15, 0, %0, c2, c0, 0\n"
	      : "=&r" (tlb_base)
	      : /* no input */ );
      printk("mmu table base address = 0x%08lx\n", tlb_base);

      printk("%04x: ", 0);
      for (i = 0; i < (4 * 4096); i += 4) {
	printk("%08lx ", __REG(i + tlb_base));
	if ((i % 32) == 28)
	  printk("\n%04x: ", (i + 4) / 4);
      }
    }

    if (0) {
      for(i=ADDR_START; i < (ADDR_START + ADDR_OFFSET*2); i += 0x10)
	printk("[0x%08lx] %08lx\n", i, __REG(i));
    }

    /* 현재 이 code를 실행하면 죽는다. MMU mapping이 뭔가 문제있다. */
    if (0) {
      GPSR0 = GPIO14_VPP_EN;

      /* Clear the status register first */
      MTD_THIS = 0x00500050;

      /* Now erase */
      MTD_THIS = 0x00200020;
      MTD_THIS = 0x00D000D0;

      i = 1024*1024;
      while (i > 0) {
	stat = MTD_THIS;
	if ((stat & MTD_OK) == MTD_OK)
	break;
	i--;
      }

      GPCR0 = GPIO14_VPP_EN;
      if (i > 0)
	printk("ERASING success![%d]\n", i);
      else
	printk("ERASING failure!\n");
    }

    printk(__FILE__ ":end\n");
    return 0;
}
/*
 | $Id: hello.c,v 1.5 2002/08/02 03:02:43 tolkien Exp $
 |
 | Local Variables:
 | mode: c
 | mode: font-lock
 | version-control: t
 | delete-old-versions: t
 | End:
 |
 | -*- End-Of-File -*-
 */
