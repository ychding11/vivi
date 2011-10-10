/*
 * vivi/arch/s3c2410/proc.c: 프로세서하고 관련된 놈들
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * 
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2003/01/30 15:46:11 $
 *
 * $Revision: 1.10 $
 * $Id: proc.c,v 1.10 2003/01/30 15:46:11 odyssey Exp $
 *
 *
 * History
 *
 * 2002-07-12: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *
 */

#include "config.h"
#include "machine.h"
#include "mmu.h"
#include "command.h"
#include "vivi_string.h"
#include "printk.h"
#include "priv_data.h"
#include <types.h>
#include <string.h>

#define GET_PCLK		0
#define GET_HCLK		1

struct cpu_clk_t {
	unsigned long clock;
	unsigned long mdiv;
	unsigned long pdiv;
	unsigned long sdiv;
};


/*
 * cpu_clk = FIN 
 */
struct cpu_clk_t s3c2410_clks[] = {
	/*	mdiv	sdiv	pdiv */
	{  33,  0x52,   0x2,    0x3 },
	{  45,  0x52,   0x1,    0x3 },
	{  50,  0xa1,   0x3,    0x3 },
	{  48,  0x28,   0x1,    0x2 },
	{  56,  0x8e,   0x2,    0x3 },
	{  67,  0x52,   0x2,    0x2 },
	{  79,  0x47,   0x1,    0x2 },
	{  84,  0x69,   0x2,    0x2 },
	{  90,  0x70,   0x2,    0x2 },
	{ 101,	0x74,	0x2,	0x2 },
	{ 113,	0x69,	0x1,	0x2 },
	{ 118,	0x96,	0x2,	0x2 },
	{ 124,  0x74,   0x1,    0x2 },
	{ 135,  0x52,   0x2,    0x1 },
	{ 147,  0x5a,   0x2,    0x1 },
	{ 152,  0x44,   0x1,    0x1 },
	{ 158,  0x47,   0x1,    0x1 },
	{ 170,  0x4d,   0x1,    0x1 },
	{ 180,  0x52,   0x1,    0x1 },
	{ 186,  0x55,   0x1,    0x1 },
	{ 192,  0x58,   0x1,    0x1 },
	{ 202,  0xa1,   0x3,    0x1 },
	{  51,  0x5c,   0x4,    0x2 },
	{ 100,  0x5c,   0x4,    0x1 },
	{ 200,	0x5c,	0x4,	0x0 },
	{ 203,	 0x3,	0x4,	0x1 },	/* not tested */
	{   0,	   0,	  0,	  0 }
};

static inline unsigned long
cal_bus_clk(unsigned long cpu_clk, unsigned long ratio, int who)
{
	if (!who) {	/* PCLK */
		switch (ratio) {
			case 0:
				return (cpu_clk);
			case 1:
			case 2:
				return (cpu_clk/2);
			case 3:
				return (cpu_clk/4);
			default:
				return 0;
		}
	} else {	/* HCLK */
		switch (ratio) {
			case 0:
			case 1:
				return (cpu_clk);
			case 2:
			case 3:
				return (cpu_clk/2);
			default:
				return 0;
		}
	}
}

/* 
 * Note: this code is not tested.
 *
 * set_cpu_clk():
 *   clk: cpu clock to set. unit is mega hertz 
 *   ratio: 
 */
static int 
set_cpu_clk(unsigned long clk)
{
	struct cpu_clk_t *clks = s3c2410_clks;

	while (clks->clock != 0) {
		if (clk == clks->clock)
		    break;
		clks++;
	}

	if (clks->clock == 0) {
		printk("Can not find cpu clock table\n");
		return -2;
	}
	
	MPLLCON = ((clks->mdiv << 12) | (clks->pdiv << 4) | (clks->sdiv));

	return 0;
}

/*
 * cpu clock = (((mdiv + 8) * FIN) / ((pdiv + 2) * (1 << sdiv)))
 *  FIN = Input Frequency (to CPU)
 */
static inline unsigned long
get_cpu_clk(void)
{
	unsigned long val = MPLLCON;
	return (((GET_MDIV(val) + 8) * FIN) / \
		((GET_PDIV(val) + 2) * (1 << GET_SDIV(val))));
}

static unsigned long
get_bus_clk(int who)
{
	unsigned long cpu_clk = get_cpu_clk();
	unsigned long ratio = CLKDIVN;
	return (cal_bus_clk(cpu_clk, ratio, who));
}

static void 
print_cpu_info(void)
{
	long armrev = 0;
	unsigned long mpll = MPLLCON;
	
	__asm__("mrc	p15, 0, %0, c0, c0, 0" : "=r" (armrev));

	printk("\nProcessor Information (Revision: 0x%08lx)\n", armrev);
	printk("--------------------------------------------\n");
	printk("Processor clock: %d Hz\n", get_cpu_clk());
	printk("AHB bus clock  : %d Hz\n", get_bus_clk(GET_HCLK));
	printk("APB bus clock  : %d Hz\n", get_bus_clk(GET_PCLK));
	printk("\nRegister values\n");
	printk("MPLLCON: 0x%08lx", mpll);
	printk("  (MDIV: 0x%04x, PDIV: 0x%02x, SDIV: 0x%02x)\n",
		GET_MDIV(mpll), GET_PDIV(mpll), GET_SDIV(mpll));
	printk("CLKDIVN: 0x%08lx\n\n", CLKDIVN);
}

/*
 * A clock is the PCLK clock.
 */
static void 
change_baudrate(unsigned long clock)
{
	unsigned long baudrate;
	int ret;

	baudrate = get_param_value("baudrate", &ret);
	if (ret) {
		printk("There is no 'baudrate' parameter\n");
		return;
	}

#if defined(CONFIG_SERIAL_UART0)
	UBRDIV0 = ((clock / (baudrate * 16)) - 1);
#elif defined(CONFIG_SERIAL_UART1)
	UBRDIV1 = ((clock / (baudrate * 16)) - 1);
#elif defined(CONFIG_SERIAL_UART2)
	UBRDIV2 = ((clock / (baudrate * 16)) - 1);
#endif
}

/*
 * change cpu clock and bus clock
 */
int
change_sys_clks(unsigned long cpu_clk, unsigned long ratio)
{
	int ret = 0;

	/* Step 1: change a cpu clock */
	ret = set_cpu_clk(cpu_clk);
	if (ret) 
		return -1;	
	/* Step 2: change bus clocks */
	if (ratio > 3 || ratio < 0)
		return -1;
	CLKDIVN = ratio;
	/* Step 3: change a memory clock */
	/* change_memctl_regs(); */
	/* Step 4: change a uart baudrate */
	change_baudrate(get_bus_clk(GET_PCLK));

	return 0;
}

/*
 * change a uart baudrate 
 */
void 
change_uart_baudrate(void)
{
	change_baudrate(get_bus_clk(GET_PCLK));
}

static inline int 
get_clk_divider(void)
{
	int val = GET_DIVIDER_TIMER4(TCFG1);
	return (2 << (val));
}

unsigned long 
get_clock_tick_rate(void)
{
	unsigned long prescale = GET_PRESCALE_TIMER4(TCFG0);
	unsigned long divider = (unsigned long)get_clk_divider();
	unsigned freq = get_bus_clk(GET_PCLK);

	if (!divider)
		return 0;

	return (freq / (prescale + 1) / divider);
}

static void 
time_wait(unsigned int sec, int unit)
{
	unsigned long ticks, clock_tick_rate;

	/* clear interupt bit */
	SRCPND |= INT_TIMER4;
	INTPND |= INT_TIMER4;
	INTMSK &= ~INT_TIMER4;			/* enable timer 4 interrupt */

	clock_tick_rate = get_clock_tick_rate();
	if (clock_tick_rate == 0) {
		printk("Can not get a clock tick rate\n");
		return;
	}

	if (unit == 0) {
		ticks = (clock_tick_rate * (sec)) / (1000 * 1000);
	} else {
		ticks = (clock_tick_rate * (sec)) / (1000);
	}

	TCNTB4 = ticks;
	TCON = (TCON_4_UPDATE | COUNT_4_OFF);	/* load counter value */

	TCON = (COUNT_4_ON);			/* start timer */

	while (!(INTPND & INT_TIMER4)) ;
	
	TCON = (COUNT_4_OFF);			/* stop timer */

	INTMSK |= INT_TIMER4;			/* mask timer 4 interrupt */
	/* clear interupt bit */
	SRCPND |= INT_TIMER4;
	INTPND |= INT_TIMER4;
}

static void
time_delay(unsigned int sec, int unit)
{
	unsigned int remain = sec;

	while (remain > 0) {
		if (remain > 40) {
			sec = 40;
		} else {
			sec = remain;
		}
		time_wait(sec, unit);
		remain -= sec;
	}
}

void 
arch_udelay(unsigned int usec)
{
	time_delay(usec, 0);
}

void 
arch_mdelay(unsigned int msec)
{
	time_delay(msec, 1);
}

void 
init_time(void)
{
	TCFG0 = (TCFG0_DZONE(0) | TCFG0_PRE1(15) | TCFG0_PRE0(0));
}

/*
 * Perform a soft reset of the system.  Put the CPU into the
 * same state as it would be if it had been reset, and branch
 * to what would be the reset vector.
 *
 * loc: location to jump to for soft reset
 */
void processor_reset(unsigned long loc)
{
	cache_clean_invalidate();
	tlb_invalidate();

__asm__(
	"mov	ip, #0\n"
	"mcr	p15, 0, ip, c7, c7, 0\n"	/* invalidate I,D caches */
	"mcr	p15, 0, ip, c7, c10, 4\n"	/* drain WB */
	"mcr	p15, 0, ip, c8, c7, 0\n"	/* invalidate I & D TLBs */
	"mrc	p15, 0, ip, c1, c0, 0\n"	/* ctrl register */
	"bic	ip, ip, #0x000f\n"		/* ............wcam */
	"bic	ip, ip, #0x1100\n"		/* ...i...s........ */
	"mcr	p15, 0, ip, c1, c0, 0\n"	/* ctrl register */
	"mov	pc, %0\n"
	: : "r" (loc) );
}

void command_reset(int argc, const char **argv)
{
	char mode;

	if (argc == 2) {
		mode = *argv[1];
	} else {
		mode = 'h';
	}
	if (mode == 's') {
		processor_reset(0);
	} else {
		WTDAT = 0x100;
		WTCNT = 0x100;
		WTCON = 0x8021;
	}
}

user_command_t reset_cmd = {
	"reset",
	command_reset,
	NULL,
	"reset\t\t\t\t-- Reset the system"
};

static void 
display_cpu_help(void)
{
	printk("Usage:\n");
	printk("cpu info\t\t\t-- Display cpu informatin\n");
	printk("cpu set <clock> <ratio>\t\t-- Change cpu clock and bus clock\n");
}

static void command_cpu(int argc, const char **argv)
{
	switch (argc) {
	case 1:
		goto print_usage;
		break;
	case 2:
		if (strncmp("help", argv[1], 4) == 0) {
			display_cpu_help();
			break;
		}
		if (strncmp("info", argv[1], 4) == 0) {
			print_cpu_info();
			break;
		}
		goto print_usage;
	case 4:
		if (strncmp("set", argv[1], 3) == 0) {
			unsigned long clock = strtoul(argv[2], NULL, 0, NULL);
			unsigned long ratio = strtoul(argv[3], NULL, 0, NULL);
			int ret = 0;

			ret = change_sys_clks(clock, ratio);
			if (ret)
				printk("Failed. Can not change cpu clock\n");
			else
				printk("OK\n");
			break;
		}
	default:
		goto print_usage;
		break;
	}

	return;

print_usage:
	display_cpu_help();
}

user_command_t cpu_cmd = {
	"cpu",
	command_cpu,
	NULL,
	"cpu [{cmds}] \t\t\t-- Manage cpu clocks"
};
