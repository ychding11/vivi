#include <printk.h>
#include "machine.h"

int main()
{
	int i;
	unsigned long ticks;

	printk("\ntimer test code\n");

	OIER = 0x0;
	OWER = 0x0;
	OSMR0 = 0;
	OSSR = 0xf;
	OIER |= OIER_E0;
	OSCR = 0;

	//printk("OSSR = 0x%08lx\n", OSSR);
	//printk("OSCR = 0x%08lx\n", OSCR);
	//printk("OSMR0 = 0x%08lx\n", OSMR0);
	//for (i = 0; i < 0x1000; i++) ;
	//printk("OSCR = 0x%08lx\n", OSCR);
	//printk("OSMR0 = 0x%08lx\n", OSMR0);

	OSSR = OSSR_M0;
	//printk("OSSR = 0x%08lx\n", OSSR);

	//OSMR0 = (OSCR + (37000 * 500));
	ticks = (3686400 * (10)) / (1000);
	printk("ticks = %d\n", ticks);
	OSMR0 = (OSCR + ticks);
	//OSMR0 = (OSCR + (4 * 1000 * 1000));


	//OSMR0 = OSCR + 0x100000;
	//printk("OSMR0 = 0x%08lx\n", OSMR0);
	//printk("OSSR = 0x%08lx\n", OSSR);
	//printk("OSMR0 = 0x%08lx\n", OSMR0);

	while (!(OSSR & OSSR_M0)) ;
	//printk("OSCR = 0x%08lx\n", OSCR);
	//for (i = 0; i < 0x80000; i++) ;
	//printk("OSCR = 0x%08lx\n", OSCR);

	//printk("OSMR0 = 0x%08lx\n", OSMR0);
	printk("OSSR = 0x%08lx\n", OSSR);
	printk("OK\n");

	return 0;
}
