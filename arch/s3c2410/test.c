#include "config.h"
#include "bootconfig.h"
#include "command-lib.h"
#include <types.h>
#ifdef CONFIG_MTD_CFI
#include "cfi.h"
#else
#include "flash.h"
#endif
#include "mtd.h"
#include "heap.h"
#include "debug.h"
#if defined(CONFIG_S3C2400_SMDK)
#include "smdk-cfg.h"
#elif defined(CONFIG_ARCH_S3C2400)
#include "s3c2400.h"
#elif defined(CONFIG_ARCH_SA1100)
#include "sa1100.h"
#elif defined(CONFIG_ARCH_S3C2410)
#include "s3c2410.h"
#endif



static void command_led1(int argc, const char **argv);
static void command_led2(int argc, const char **argv);
static void command_led3(int argc, const char **argv);
static void command_rtcdump(int argc, const char **argv);
static void command_resetrtc(int argc, const char **argv);
static void command_timer(int argc, const char **argv);
static void command_show_timer_reg(int argc, const char **argv);
static void command_enable_irq(int argc, const char **argv);
static void command_stop(int argc, const char **argv);
static void command_go_gpos(int argc, const char **argv);
static void command_led_key(int argc, const char **argv);
static void command_question(int argc, const char **argv);

extern struct mtd_info g_mtd;
extern struct cfi_private g_cfi;

void tests_usage(void);

static struct bootblk_command tests_commands[] = {
#if defined(CONFIG_S3C2400_SMDK)
	{ "led1",	command_led1,		NULL },
	{ "led2",	command_led2,		NULL },
	{ "led3",	command_led3,		NULL },
	{ "rtcdump",	command_rtcdump,	NULL },
	{ "restorertc",	command_resetrtc,	NULL },
	{ "timer",	command_timer,		NULL },
	{ "show_timer_reg",	command_show_timer_reg, NULL },
	{ "enable_irq",	command_enable_irq,	NULL },
#endif
#ifdef CONFIG_S3C2400_GAMEPARK
	{ "led_key",	command_led_key,	"key-led" },
	{ "gpos",	command_go_gpos,	"gpos" },
	{ "stop",	command_stop,		"stop" },
#endif
	{ "q",		command_question,	"q" },
	{ NULL,		NULL,			NULL }
};

void command_tests(int argc, const char **argv)
{
	if (argc == 1) {
		putstr("invalid 'test' command: too few arguments\r\n");
		tests_usage();
	} else {
		execcmd(tests_commands, argc - 1, argv + 1);
	}
}

static void command_question(int argc, const char **argv)
{
	int retry = 3;
	__u8 c[2];

	while (retry > 0) {
		putstr("are you sure (y/n) ? ");
		*c = getc();
		c[1] = '\0';
		if ((strcmp(c, "y") == 0) || (strcmp(c, "n") == 0)) {
			putstr(c);
			putstr("\r\n");
			break;
		}
		retry--;
		putstr("\r\n");
	}
}

#ifdef CONFIG_S3C2400_GAMEPARK
#define TMP_REG	(*(volatile unsigned long *)(0x15a00034))
static void command_go_gpos(int argc, const char **argv)
{
	putstr("Go GPOS\r\n");

	TMP_REG = 0x17;

	_SRCPND = 0xffffffff;
	_INTPND = 0xffffffff;

	cpu_arm920_cache_clean_invalidate_all();
	cpu_arm920_tlb_invalidate_all();

	__asm__("mov	r1, #0\n"
		"mov	r2, #0\n"
		"mrc	p15, 0, r1, c1, c0, 0\n"	/* read ctrl register */
		"bic	r1, r1, #0x1100\n"		/* ...i...s........ */
		"bic	r1, r1, #0x000f\n"		/* .............cam */
		"mcr	p15, 0, r1, c1, c0, 0\n"	/* write ctrl register */
		"mov	pc, r2\n"
		"nop\n"
		"nop\n");
}

static void command_led_key(int argc, const char **argv)
{
	_PDCON = (1 << 10);
	_PDDAT = (1 << 5);
}

extern void go_sleep(void);
static void command_stop(int argc, const char **argv)
{
#if 0
	unsigned long cpsr;
	unsigned long count = 0;

	_SRCPND = 0xffffffff;
	_INTPND = 0xffffffff;

	putLabeledWord("SRCPND = 0x", _SRCPND);
	putLabeledWord("INTPND = 0x", _INTPND);

	_INTMSK = 0x0;
	putLabeledWord("INTMSK = 0x", _INTMSK);

	_PECON = (0x2 << 2);
	_PEUP = (0 << 1);
	_EXTINT = (4 << 4);
	_INTMOD = 0x00000000;
	//_INTMSK = ~(IRQ_EINT1);
	_INTMSK = 0xffffffff;

	while (1) {
		if ((0x000000ff & _SRCPND) == 0x2) break;
		putstr("show\r\n");
		putLabeledWord("SRCPND = 0x", _SRCPND);
		putLabeledWord("INTPND = 0x", _INTPND);
		count = 0x100000;
		while (count > 0) count--;

	}

	putstr("Intrrupt is occured\r\n");
	putLabeledWord("SRCPND = 0x", _SRCPND);
	putLabeledWord("INTPND = 0x", _INTPND);

	count = 0x100000;
	while (count > 0) count--;

	__asm__("mrs %0, cpsr" : "=r" (cpsr));
	cpsr &= ~(I_BIT);
	__asm__("msr cpsr_c, %0" : : "r" (cpsr));	/* enable IRQ */

	putstr("Enable I_BIT\r\n");
	putLabeledWord("SRCPND = 0x", _SRCPND);
	putLabeledWord("INTPND = 0x", _INTPND);
#endif

	unsigned long cpsr;

	__asm__("mrs %0, cpsr" : "=r" (cpsr));
	cpsr &= ~(I_BIT);
	__asm__("msr cpsr_c, %0" : : "r" (cpsr));	/* enable IRQ */

#ifdef CONFIG_S3C2400_SMDK
	_PECON = 0x2;
	_INTMOD = 0x00000000;
	_INTMSK = ~(IRQ_EINT0);
#elif defined(CONFIG_S3C2400_GAMEPARK)
	_PECON = (0x2 << 14);
	_PEUP = (0 << 7);
	_EXTINT = (4 << 28);
	_INTMOD = 0x00000000;
	_INTMSK = ~(IRQ_EINT7);
#endif

	//go_sleep();

	_SRCPND = 0xffffffff;
	_INTPND = 0xffffffff;
	_PECON = (0x2 << 14);
	_PEUP = (0 << 7);
	_EXTINT = (4 << 28);

	cpu_arm920_cache_clean_invalidate_all();
	cpu_arm920_tlb_invalidate_all();

        __asm__("mov    r1, #0\n"
                "mov    r2, #0x34\n"
                "mrc    p15, 0, r1, c1, c0, 0\n"        /* read ctrl register */
                "bic    r1, r1, #0x1100\n"              /* ...i...s........ */
                "bic    r1, r1, #0x000f\n"              /* .............cam */
                "mcr    p15, 0, r1, c1, c0, 0\n"        /* write ctrl register */
                "mov    pc, r2\n"
                "nop\n"
                "nop\n");
}
#endif

#if defined(CONFIG_S3C2400_SMDK)
#define PCDAT *(volatile unsigned long *)(S3C2400_IOPORT_BASE_ADDR + S3C2400_PCDAT)
static void command_led1(int argc, const char **argv)
{
	int i;

	for (i = 0; i < 3; i++) {
		PCDAT = (PCDAT & 0xfff) | ((0xf & 0xf) << 12);
		udelay(10000);
		PCDAT = (PCDAT & 0xfff) | ((0x0 & 0xf) << 12);
		udelay(10000);
	}
}

static void command_led2(int argc, const char **argv)
{
	PCDAT = (PCDAT & 0xfff) | ((0x7 & 0xf) << 12);
	udelay(10000);
	PCDAT = (PCDAT & 0xfff) | ((0x3 & 0xf) << 12);
	udelay(10000);
	PCDAT = (PCDAT & 0xfff) | ((0x1 & 0xf) << 12);
	udelay(10000);
	PCDAT = (PCDAT & 0xfff) | ((0x0 & 0xf) << 12);
	udelay(10000);
}

static void command_led3(int argc, const char **argv)
{
	unsigned long temp;

	PCDAT |= (1 << 12);
	temp = 1000000;
	while (temp != 0)
		temp--;
	PCDAT &= ~(1 << 12);
}

static void command_rtcdump(int argc, const char **argv)
{
	putLabeledWord("TICNT = 0x", _TICNT);
	putLabeledWord("RTCALM = 0x", _RTCALM);
	putLabeledWord("BCDSEC = 0x", _BCDSEC);
	putLabeledWord("BCDMIN = 0x", _BCDMIN);
	putLabeledWord("BCDHOUR = 0x", _BCDHOUR);
	putLabeledWord("BCDDAY = 0x", _BCDDAY);
	putLabeledWord("BCDDATE = 0x", _BCDDATE);
	putLabeledWord("BCDMON = 0x", _BCDMON);
	putLabeledWord("BCDYEAR = 0x", _BCDYEAR);

}

/* reset rtc to 1970. 1. 1. 0. 0. 0 */
static void command_resetrtc(int argc, const char **argv)
{
	_RTCCON = (unsigned char)0x1;
	_TICNT = (unsigned char)0x0;
	_RTCALM = (unsigned char)0x0;
	_BCDSEC = (unsigned char)0x0;
	_BCDMIN = (unsigned char)0x0;
	_BCDHOUR = (unsigned char)0x0;
	_BCDDAY = (unsigned char)0x0;
	_BCDDATE = (unsigned char)0x1;
	_BCDMON = (unsigned char)0x1;
	_BCDYEAR = (unsigned char)0x70;
}

static void command_show_timer_reg(int argc, const char **argv)
{
	unsigned long cpsr = 0;

	putLabeledWord("TCON = ", _TCON);
	putLabeledWord("TCNTO4 = ", _TCNTO4);
	putLabeledWord("SRCPND = ", _SRCPND);
	putLabeledWord("INTPND = ", _INTPND);
	putLabeledWord("INTMSK = ", _INTMSK);
	__asm__("mrs %0, cpsr" : "=r" (cpsr));
	putLabeledWord(">> current CPSR = ", cpsr);
}

static void command_enable_irq(int argc, const char **argv)
{
	unsigned long cpsr = 0;

	__asm__("mrs %0, cpsr" : "=r" (cpsr));
	cpsr &= ~(I_BIT);
	__asm__("msr cpsr_c, %0" : : "r" (cpsr));	/* enable IRQ */
}

static void command_timer(int argc, const char **argv)
{
	unsigned long cpsr = 0;
	unsigned long count = 0;

	__asm__("mrs %0, cpsr" : "=r" (cpsr));
	/*putLabeledWord(">> current CPSR = ", cpsr);*/
#if 1 /* hacked by nandy */
	cpsr &= ~(I_BIT);
	__asm__("msr cpsr_c, %0" : : "r" (cpsr));	/* enable IRQ */
	/*putLabeledWord(">> current CPSR = ", cpsr);*/
#endif
	_TCFG0 = 0x0f00;	/* prescaler value = 15 */

#if 1
	_TCNTB4 = 15626;	/* down-counter, maximum value is 65535 (2^16) */
#else
	_TCNTB4 = 65535;	/* down-counter, maximum value is 65535 (2^16) */
#endif

	_INTMOD = 0x00000000;
#if 0
	_INTMSK = 0xffffbfff;
#else
	_INTMSK = 0x00000000;
#endif
	_SRCPND = 0x00000000;
	_INTPND = 0x00000000;

	_TCON = 0x600000; /* internal mode (auto reload), update TCNTB4, stop Timer 4 */
	_TCON = 0x500000;	/* interval mode, no operation, start Timer 4 */

#if 0
	count = 0;
	while (1) {
		if ((_INTPND & 0x00004000) && (0x00004000)) {
			if (count < 100000) {
				count++;
				_INTPND = 0x00000000;
			} else {
				count = 0;
				putstr(".\n");
				_INTPND = 0x00000000;
			}
		}
	}
#endif
#if 0
	putLabeledWord("SRCPND = ", _SRCPND);
	putLabeledWord("TCON = ", _TCON);
	putLabeledWord("TCNTB4 = ", _TCNTB4);
#endif
}
#endif


/*
 * Messages
 */

void tests_usage(void)
{
	int i = 0;
	putstr("Usage:\r\n");
	putstr("  test <items>\r\n");
	putstr("  items: ");
	for (i = 0; tests_commands[i].cmdstr != NULL; i++) {
		if (!(i % 5)) {
			putstr("\r\n"); 
			putstr("         "); 
		}
		putstr(tests_commands[i].helpstr); putstr(", ");
	}
	putstr("\r\n");
}
