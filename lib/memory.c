/*
 * vivi/lib/memory.c:
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
 * Date  : $Date: 2002/08/28 05:59:13 $
 *
 * $Revision: 1.12 $
 * $Id: memory.c,v 1.12 2002/08/28 05:59:13 nandy Exp $
 *
 *
 * History
 * 
 * 2002-02-25: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *
 * 2002-07-08: Janghoon Lyu <nandy@mizi.com>
 *    - 완전히 코드 새로 작성.
 *
 * 2002-07-15: Janghoon Lyu <nandy@mizi.com>
 *    - 바뀐 명령어 체계에 따른 수정
 *    - 디램 크기 찾아 내는 코드 수정. (PPCBOOT에서 따옴)
 *
 */

#include "config.h"
#include "machine.h"
#include "command.h"
#include "hardware.h"
#include "vivi_string.h"
#include "sizes.h"
#include "vivi_lib.h"
#include "printk.h"
#include <types.h>
#include <string.h>


 /*
  * Clear memory (set to '0')
  */
void
clear_mem(unsigned long base, unsigned long len)
{
	__asm__ volatile (
		"mov	r0, %0\n"	/* store base address */
		"mov	r1, %1\n"	/* store length */
		"mov	r2, #0\n"
		"mov	r3, r2\n"
		"mov	r4, r2\n"
		"mov	r5, r2\n"
		"mov	r6, r2\n"
		"mov	r7, r2\n"
		"mov	r8, r2\n"
		"mov	r9, r2\n"
	" 1:	 stmia	r0!, {r2-r9}\n"	/* clear 32 (4 bytes * 8) bytes */
		"subs	r1, r1, #(8 * 4)\n"
		"bne	1b\n"
		: /* no outputs */
		: "r" (base), "r" (len)
		: "r0", "r1", "r2", "r3", "r4", 
		  "r5", "r6", "r7", "r8", "r9" 
	);
}

/*
 * Based on ppcboot/board/board.c
 *
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */
long find_dram_size(long *base, long maxsize)
{
	volatile long *addr;  
	ulong cnt, val;
	ulong save[32];     /* to make test non-destructive */
	int i = 0;

	for (cnt = maxsize/sizeof(long); cnt > 0; cnt >>= 1) {
		addr = base + cnt;      /* pointer arith! */

		save[i++] = *addr;
		*addr = ~cnt; 
	}

	/* write 0 to base address */
	addr = base; 
	save[i] = *addr;
	*addr = 0;

	/* check at base address */
	if ((val = *addr) != 0) { 
		*addr = save[i];
		return (0);
	}

	for (cnt = 1; cnt <= maxsize/sizeof(long); cnt <<= 1) {
		addr = base + cnt;      /* pointer arith! */

		val = *addr;
		*addr = save[--i];

		if (val != (~cnt)) {
			return (cnt * sizeof(long));
		}
	}
	return (maxsize);
}


long probe_dram_size(void)
{
	long mem_size;

	mem_size = find_dram_size((long *)DRAM_BASE, SZ_256M);
	printk("Detected memory size = 0x%08lx, %dM (%d bytes)\n", 
		mem_size, (mem_size >> 20), mem_size);

	return mem_size;
}

unsigned long mem_compare(const char *to, const char *from, size_t len, int echo)
{
	unsigned long ofs = 0;

	if (echo) progress_bar(ofs, len + ofs);
	while (len >= sizeof(unsigned long)) {
		if (*(unsigned long *)(to + ofs) != *(unsigned long *)(from + ofs))
			return ofs;

		len -= sizeof(unsigned long);
		ofs += sizeof(unsigned long);
	}
	if (echo) progress_bar(ofs, len + ofs);

	if (len > 0) {
		if (*(to + ofs) != *(from + ofs));
			return ofs;
		len -= sizeof(char);
		ofs += sizeof(char);
	}
	if (echo) progress_bar(ofs, len + ofs);

	return ofs;
}

#ifdef CONFIG_CMD_MEM

static size_t mem_copy(void *to, const void *from, size_t len, int echo)
{
	ulong ofs = 0;

	while (len >= sizeof(unsigned long)) {
		*(unsigned long *)(to + ofs) = *(unsigned long *)(from + ofs);
		len -= sizeof(unsigned long);
		ofs += sizeof(unsigned long);
	}

	if (len > 0) {
		(*(char *)(to + ofs) = *(char *)(from + ofs));
		len -= sizeof(char);
		ofs += sizeof(char);
	}

	return ofs;
}

static void search_value(unsigned long *start, unsigned long *end, 
                         unsigned long value)
{
	volatile unsigned long *addr;
	while ((end - start) != 0) {
		addr = start;
		if (*addr == value) 
			printk("address = 0x%08lx\n", start);
		start++;
	}
}

static void inline
write(long *to, ulong value)
{
	volatile long *addr = to;
	*addr = value;
}

static ulong inline
read(long *from)
{
	volatile long *addr = from;
	return *addr;
}

void print_mem_map(void)
{
	long dram_size;
	long sp = 0;

	dram_size = find_dram_size((long *)DRAM_BASE, SZ_256M);
	printk("Default ram size: %dM\n", (DRAM_SIZE >> 20));
	printk("Real ram size   : %dM\n", (dram_size >> 20));
	printk("Free memory     : %dM\n\n", (RAM_SIZE >> 20));

	printk("RAM mapped to           : 0x%08lx - 0x%08lx\n", \
		DRAM_BASE, (DRAM_BASE + DRAM_SIZE));
	printk("Flash memory mapped to  : 0x%08lx - 0x%08lx\n", \
		FLASH_UNCACHED_BASE, (FLASH_UNCACHED_BASE + FLASH_SIZE));
	printk("Available memory region : 0x%08lx - 0x%08lx\n", \
		RAM_BASE, (RAM_BASE + RAM_SIZE));
	printk("Stack base address      : 0x%08lx\n", (STACK_BASE + STACK_SIZE - 4));
	__asm__("mov %0, sp" : "=r" (sp));
	printk("Current stack pointer   : 0x%08lx\n", sp);
}

void print_mem_ctrl_regs(void)
{
	printk("\nMemory control register vlaues\n");
#if defined(CONFIG_ARCH_S3C2400)
	printk("     BWSCON = 0x%08lx\n", _BWSCON);
	printk("   BANKCON0 = 0x%08lx\n", _BANKCON0);
	printk("   BANKCON1 = 0x%08lx\n", _BANKCON1);
	printk("   BANKCON2 = 0x%08lx\n", _BANKCON2);
	printk("   BANKCON3 = 0x%08lx\n", _BANKCON3);
	printk("   BANKCON4 = 0x%08lx\n", _BANKCON4);
	printk("   BANKCON5 = 0x%08lx\n", _BANKCON5);
	printk("   BANKCON6 = 0x%08lx\n", _BANKCON6);
	printk("   BANKCON7 = 0x%08lx\n", _BANKCON7);
	printk("    REFRESH = 0x%08lx\n", _REFRESH);
#elif defined(CONFIG_ARCH_S3C2410)
	printk("     BWSCON = 0x%08lx\n", BWSCON);
	printk("   BANKCON0 = 0x%08lx\n", BANKCON0);
	printk("   BANKCON1 = 0x%08lx\n", BANKCON1);
	printk("   BANKCON2 = 0x%08lx\n", BANKCON2);
	printk("   BANKCON3 = 0x%08lx\n", BANKCON3);
	printk("   BANKCON4 = 0x%08lx\n", BANKCON4);
	printk("   BANKCON5 = 0x%08lx\n", BANKCON5);
	printk("   BANKCON6 = 0x%08lx\n", BANKCON6);
	printk("   BANKCON7 = 0x%08lx\n", BANKCON7);
	printk("    REFRESH = 0x%08lx\n", REFRESH);
	printk("   BANKSIZE = 0x%08lx\n", BANKSIZE);
	printk("     MRSRB6 = 0x%08lx\n", MRSRB6);
	printk("     MRSRB7 = 0x%08lx\n", MRSRB7);
#elif defined(CONFIG_ARCH_SA1100)
	printk("SDRAM bank0:\n");
	printk("   mdcnfg = 0x%08lx\n",
			CTL_REG_READ(SA1100_DRAM_CONFIGURATION_BASE+SA1100_MDCNFG));
	printk("   mdrefr = 0x%08lx\n",
			CTL_REG_READ(SA1100_DRAM_CONFIGURATION_BASE+SA1100_MDREFR));
#else
	printk("UNKNOWN ARCHITECTURE!\n");
#endif
}

static void mem_info(void)
{
	printk("\nRAM Information:\n");
	print_mem_map();
	print_mem_ctrl_regs();
}


/*
 * Commands
 */
static user_subcommand_t mem_cmds[];



static void command_mem_info(int argc, const char **argv)
{
	mem_info();
}

static void command_mem_set_regs(int argc, const char **argv)
{
	//init_mem_regs();
}

void command_mem_copy(int argc, const char *argv[])
{
        void *dst;
        void *src;
	int ret;
	size_t retlen, len;

	if (argc != 4) {
		printk("invalid 'mem copy' command: too few(many) arguments\n");
		return;
	}

	dst = (void *)strtoul(argv[1], NULL, 0, &ret);
	src = (void *)strtoul(argv[2], NULL, 0, &ret);
	len = (size_t)strtoul(argv[3], NULL, 0, &ret);

	printk("Copy from 0x%08lx, to 0%08lx. length is 0x%08lx\n", src, dst, len);

	retlen = mem_copy(dst, src, len, 0);

	printk("\nCopied %d (0x%08lx) bytes\n", retlen, retlen);
}

void command_mem_compare(int argc, const char *argv[])
{
        char *to, *from;
	int ret;
	unsigned long ofs;
	size_t len;

	if (argc != 4) {
		printk("invalid 'mem cmp' command: too few(many) arguments\n");
		return;
	}

        to = (char *)strtoul(argv[1], NULL, 0, &ret);
        from = (char *)strtoul(argv[2], NULL, 0, &ret);
	len = (size_t)strtoul(argv[3], NULL, 0, &ret);

	ofs = mem_compare(to, from, len, 1);

	if (ofs != len) {
		printk("\nNot matched. offset = 0x%08lx\n", ofs);
		printk("value:  src = 0x%08lx, dst = 0x%08lx",
			*(unsigned long *)(from + ofs), *(unsigned long *)(to + ofs));
	}
	printk("\nOK.\n");
}

/*
 * vivi> mem search <start_addr> <end_addr> <value>
 *
 *       search (argv[0]): command of memeory
 * <start_addr> (argv[1]): start address for search.
 *   <end_addr> (argv[2]): end address for serach.
 *      <value> (argv[3]): value to serach.
 */
void command_search(int argc, const char **argv)
{
	unsigned long *start;
	unsigned long *end;
	unsigned long value;
	int ret;

	if (argc != 4) {
		printk("invalid 'mem serach' command: too few(many) arguments\n");
		print_usage("  ", mem_cmds);
		return;
	}

	start = (unsigned long *)strtoul(argv[1], NULL, 0, &ret);
	if (ret) goto error_parse_arg;
	end = (unsigned long *)strtoul(argv[2], NULL, 0, &ret);
	if (ret) goto error_parse_arg;
	value = strtoul(argv[3], NULL, 0, &ret);
	if (ret) goto error_parse_arg;

	printk("serach 0x%08lx value from 0x%08lx to 0x%08lx\n", value, start, end);
	search_value(start, end, value);
	return;

error_parse_arg:
	printk("Can't parsing arguments\n");
	return;
}

void command_size(void)
{
	probe_dram_size();
}

static user_subcommand_t mem_cmds[] = {
{
	"cmp",
	command_mem_compare,
	"compare <dst> <src> <length> -- compare "
}, { 
	"copy",
	command_mem_copy,
	"mem copy <dst> <src> <length>" 
}, { 
	"info",	
	command_mem_info,
	"mem info" 
}, {
	"reset",
	command_mem_set_regs,
	"mem reset -- reset memory control register" 
}, {
	"search",
	command_search,
	"mem serach <start_addr> <end_addr> <value> -- serach memory address that contain value" 
}, {	
	NULL,
	NULL,
	NULL 
}
};

#ifdef CONFIG_MEMORY_RAM_TEST
extern int mem_test(unsigned long start, unsigned long ramsize, int quiet);
#endif
void command_mem(int argc, const char **argv)
{
	switch (argc) {
	case 1:
		invalid_cmd("mem", mem_cmds);
		break;
	case 2:
		if (strncmp("size", argv[1], 4) == 0) {
			command_size();
			break;
		}
	case 3:
		if (strncmp("read", argv[1], 4) == 0) {
			ulong value;
			long *addr = (long *)strtoul(argv[2], NULL, 0, NULL);
			value = read(addr);
			printk("addr: 0x%08lx, value: 0x%08lx\n", addr, value);
			break;
		}
	case 4:
		if (strncmp("write", argv[1], 4) == 0) {
			long *addr = (long *)strtoul(argv[2], NULL, 0, NULL);
			ulong value = strtoul(argv[3], NULL, 0, NULL);
			write(addr, value);
			printk("addr: 0x%08lx, value: 0x%08lx\n", addr, value);
			break;
		}
#ifdef CONFIG_MEMORY_RAM_TEST
		/*
		 * vivi> mem test <start> <size>
		 */
		if (strncmp("test", argv[1], 4) == 0) {
			unsigned long start = strtoul(argv[2], NULL, 0, NULL);
			unsigned long size = strtoul(argv[3], NULL, 0, NULL);
			mem_test(start, size, 1);
			break;
		}
#endif
	case 5:
#ifdef CONFIG_MEMORY_RAM_TEST
		/*
		 * vivi> mem test <start> <size>
		 */
		if (strncmp("test", argv[1], 4) == 0) {
			unsigned long start = strtoul(argv[2], NULL, 0, NULL);
			unsigned long size = strtoul(argv[3], NULL, 0, NULL);
			unsigned long quiet = strtoul(argv[4], NULL, 0, NULL);
			mem_test(start, size, quiet);
			break;
		}
#endif

	default:
		execsubcmd(mem_cmds, argc-1, argv+1);
	}
}

user_command_t mem_cmd = {
	"mem",
	command_mem,
	NULL,
	"mem [{cmds}] \t\t\t-- Manage Memory"
};

#endif /* CONFIG_CMD_MEM */
