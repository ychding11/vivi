/*
 * vivi/lib/exec.c: Execute a binary.
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/08/10 07:47:08 $
 *
 * $Revision: 1.4 $
 *
 * $Id: exec.c,v 1.4 2002/08/10 07:47:08 nandy Exp $
 *
 * TODO:
 *   1. 실행하고 다시 VIVI로 돌아올 수 있도록 해야 함.
	- tolkien: 했음. (2002/08/01 21:13)
 */

#include "mmu.h"
#include "command.h"
#include "printk.h"
#include "vivi_string.h"
#include <types.h>

static void call(unsigned long addr, long a0, long a1, long a2, long a3)
{
	printk("jump to 0x%08lx with argumests\n", addr);
	printk("  argument 0 = 0x%08lx\n", a0);
	printk("  argument 1 = 0x%08lx\n", a1);
	printk("  argument 2 = 0x%08lx\n", a2);
	printk("  argument 3 = 0x%08lx\n", a3);

	__asm__(
		"mov	r0, %0\n"
		"mov	r1, %1\n"
		"mov	r2, %2\n"
		"mov	r3, %3\n"
		"add	lr, pc, #8\n"
		"mov	pc, %4\n"
	: /* no outputs */
	: "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (addr)
	);
}

static void jump(unsigned long addr)
{
	printk("jump to 0x%08lx\n", addr);

	__asm__(
		"mov	pc, %0\n"
		"nop\n"
		"nop\n"
		: /* no outputs */
		: "r" (addr)
		);
}

static void go(unsigned long addr, long a0, long a1, long a2, long a3)
{
	printk("go to 0x%08lx\n", addr);
	printk("  argument 0 = 0x%08lx\n", a0);
	printk("  argument 1 = 0x%08lx\n", a1);
	printk("  argument 2 = 0x%08lx\n", a2);
	printk("  argument 3 = 0x%08lx\n", a3);

	cache_clean_invalidate();
	tlb_invalidate();

__asm__(
	"mov	r0, %0\n"
	"mov	r1, %1\n"
	"mov	r2, %2\n"
	"mov	r3, %3\n"
	"mov	r4, %4\n"
	"mov	ip, #0\n"
	"mcr	p15, 0, ip, c13, c0, 0\n"	/* zero PID */
	"mcr	p15, 0, ip, c7, c7, 0\n"	/* invalidate I,D caches */
	"mcr	p15, 0, ip, c7, c10, 4\n"	/* drain write buffer */
	"mcr	p15, 0, ip, c8, c7, 0\n"	/* invalidate I,D TLBs */
	"mrc	p15, 0, ip, c1, c0, 0\n"	/* get control register */
	"bic	ip, ip, #0x0001\n"		/* disable MMU */
	"mcr	p15, 0, ip, c1, c0, 0\n"	/* write control register */
	"mov	pc, r4\n"
	"nop\n"
	"nop\n"
	: /* no outpus */
	: "r" (a0), "r" (a1), "r" (a2), "r" (a3), "r" (addr)
	);
}

void exec(int argc, const char **argv, int who)
{
	unsigned long addr;
	long a0, a1, a2, a3;

	a0 = a1 = a2 = a3 = 0;

	if (argc < 2) {
		printk("invalid 'go' command: wrong arguments\n");
		return;
	}

	addr = strtoul(argv[1], NULL, 0, NULL);
	switch (argc) {
		case 2:
			if (!who) jump(addr);
			else go(addr, a0, a1, a2, a3);
			return;
		case 3:
			a0 = strtoul(argv[2], NULL, 0, NULL);
			break;
		case 4:
			a0 = strtoul(argv[2], NULL, 0, NULL);
			a1 = strtoul(argv[3], NULL, 0, NULL);
			break;
		case 5:
			a0 = strtoul(argv[2], NULL, 0, NULL);
			a1 = strtoul(argv[3], NULL, 0, NULL);
			a2 = strtoul(argv[4], NULL, 0, NULL);
			break;
		case 6:
			a0 = strtoul(argv[2], NULL, 0, NULL);
			a1 = strtoul(argv[3], NULL, 0, NULL);
			a2 = strtoul(argv[4], NULL, 0, NULL);
			a3 = strtoul(argv[5], NULL, 0, NULL);
			break;
		default:
			printk("invalid 'go' command: wrong arguments\n");
			return;
	}

	if (who == 0) {
		call(addr, a0, a1, a2, a3);
	} else {
		go(addr, a0, a1, a2, a3);
	}
}

void command_call(int argc, const char **argv)
{
	exec(argc, argv, 0);
}

void command_go(int argc, const char **argv)
{
	exec(argc, argv, 1);
}


user_command_t go_cmd = {
	"go",
	command_go,
	NULL,
	"go <addr> <a0> <a1> <a2> <a3> \t-- jump to <addr>"
};

user_command_t call_cmd = {
	"call",
	command_call,
	NULL,
	"call <addr> <a0> <a1> <a2> <a3> \t-- jump_with_return to <addr>"
};
