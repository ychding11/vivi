/*
 * vivi/arch/s3c2410/smdk2410_test.c: test codes for SMDK-2410
 *
 * Copyrigth (C) 2002 MIZI Research, Inc.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/09/12 03:50:23 $
 *
 * $Revision: 1.5 $
 */

#include <config.h>
#include <machine.h>
#include <command.h>
#include <printk.h>
#include <string.h>
#include <mmu.h>
#include <time.h>
#include <types.h>
#include <serial.h>

#define PM_RESUME	0
#define PM_SUSPEND	2

#define LOWLEVEL	0
#define HIGHLEVEL	1
#define FALLING_EDGE	2
#define RISINGE_EDGE	4
#define BOTH_EDGE	6
#define EINT0		0


int pm_set_gpios(int request)
{
	switch (request) {
	case PM_SUSPEND:
		break;
	case PM_RESUME:
		break;
	default:
		return -1;
	}

	return 0;
}

int pm_send_all(int request)
{
	switch (request) {
	case PM_SUSPEND:
		/* Suspend USB pads */
		PMCTL1 |= (USBSPD1 | USBSPD0);
		/* Disable LCD */
		break;
	case PM_RESUME:
		/* Resume USB pads */
		PMCTL1 &= ~(USBSPD1 | USBSPD0);
		/* Enable LCD */
		break;
	default:
		return -1;
	}
	return 0;
}

int pm_s3c2410_cpu_suspend(void)
{
	/* save return address */
	PMSR0 = 0x38;

	printk("call SleepRamProc()\n");

	cache_clean_invalidate();
	tlb_invalidate();

	__asm__(
		"mov	r2, #0x34\n"
		"mrc	p15, 0, r1, c1, c0, 0\n"	/* read ctrl register */
		"bic	r1, r1, #0x0001\n"		/* MMU is off */	
		"mcr	p15, 0, r1, c1, c0, 0\n"	/* write ctrl register */
		"mov	pc, r2\n"
		"nop\n"
		"nop\n");

	for (;;) ;
}

int pm_cpu_suspend(void)
{
	pm_set_gpios(PM_SUSPEND);

	pm_s3c2410_cpu_suspend();

	pm_set_gpios(PM_RESUME);

	return 0;
}

int pm_sys_suspend(void)
{
	int ret;

	/* Disable all interrupts */

	ret = pm_send_all(PM_SUSPEND);

	pm_cpu_suspend();

	ret = pm_send_all(PM_RESUME);

	/* Enable all interrupts */
	
	return 0;
}


int pm_go_suspend(void)
{
	GPFCON = 2;
	EXTINT0 = (FALLING_EDGE << EINT0);

	SRCPND = 0xffffffff;
	INTPND = 0xffffffff;
	INTMOD = 0x00000000;
	INTMSK = ~(0x1);

	pm_sys_suspend();
	
	return 0;
}


void wait_eint0(void)
{
	unsigned long cpsr;

	/* Setup power button */
	__asm__("mrs %0, cpsr" : "=r" (cpsr));
	cpsr &= ~(I_BIT);
	__asm__("msr cpsr_c, %0" : : "r" (cpsr));       /* enable IRQ */

	/* setup edge */
	GPFCON = 2;
	EXTINT0 = (FALLING_EDGE << EINT0);

	SRCPND = 0xffffffff;
	INTPND = 0xffffffff;
	INTMOD = 0x00000000;
	INTMSK = ~(0x1);

}

void z_car(void)
{
	__u8 c = 0;

	GPFCON = 0x55aa;
	GPFUP = 0xff;
	for (;;) {
		GPFDAT = 0xe0;
		mdelay(100);
		GPFDAT = 0xd0;
		mdelay(100);
		GPFDAT = 0xb0;
		mdelay(100);
		GPFDAT = 0x70;
		mdelay(100);
		GPFDAT = 0xb0;
		mdelay(100);
		GPFDAT = 0xd0;
		mdelay(100);

		c = do_getc(NULL, 0x100, NULL);
		if (c != 0) break;
	}
}

static void
wait_int(int irq)
{
	while (!(INTPND & (1 << irq))) ;

	printk("OK. detected irq %d\n", irq);
}

void test_btn(void)
{
	GPFCON = 2;
	EXTINT0 = (BOTH_EDGE << EINT0);

	SRCPND = 0xffffffff;
	INTPND = 0xffffffff;
	INTMOD = 0x00000000;
	INTMSK = ~(0x1);

	printk("GPFDAT = 0x%08lx\n", GPFDAT);
	wait_int(EINT0);
	printk("GPFDAT = 0x%08lx\n", GPFDAT);
	SRCPND = 0xffffffff;
	INTPND = 0xffffffff;
	wait_int(EINT0);
	printk("GPFDAT = 0x%08lx\n", GPFDAT);
}

void display_help(void)
{
	printk("Usage:\n");
	printk("\ttest sleep\t\t-- Test sleep mode. (pwbt is eint0)\n");
	printk("\ttest int\t\t-- Test external interrupt 0\n");
	printk("\ttest led\t\t-- Test LEDs\n");
}

void command_test(int argc, const char **argv)
{
	switch (argc) {
	case 2:
		if (strncmp("sleep", argv[1], 4) == 0) {
			pm_go_suspend();
			break;
		}
		if (strncmp("int", argv[1], 3) == 0) {
			wait_eint0();
			break;
		}
		if (strncmp("led", argv[1], 3) == 0) {
			z_car();
			break;
		}
		if (strncmp("btn", argv[1], 3) == 0) {
			test_btn();
			break;
		}
	default:
		display_help();
	}
}

user_command_t test_cmd = {
	"test",
	command_test,
	NULL,
	"test [{cmds}]\t\t\t-- Test functions"
};
