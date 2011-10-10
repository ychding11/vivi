/*
 * vivi/lib/boot-kernel.c: copy the kernel image to ram, then execute it 
 *
 * Copyright (C) 2001,2002 MIZI Research, Inc.
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
 * Date  : $Date: 2002/08/21 06:42:02 $
 *
 * $Revision: 1.9 $
 *
 *
 * Boot (command) sequence (2002-06-28, Janghoon Lyu)
 *
 * 1. 사용자가 'boot' 명령어를 명시적으로 내리거나, 타임아웃이 걸려
 *    부팅을 하려고 한다. 이 때 불려 지는 함수는 boot()이다.
 *
 * 2. boot()안에서는 다음의 절차를 거친다.
 *    1) 커널의 메모리에 위치시킨다. (이미 메모리에 위치해 있어면 
 *       이 과정을 그냥 통과한다.
 *    2) 커널 부트 파라미터 영역을 적절히 설정한다.
 *
 *
 * TODO
 *  1. boot ram 명령어가 정상적으로 동작하지 않는 버그가 있음.
 * 
 *
 * History
 * 
 * 2001-12-23: Janghoon Lyu <nandy@mizi.com>
 *    - Created code
 *
 * 2002-02-23: Janghoon Lyu <nandy@mizi.com>
 *    -  Add flash commands
 *
 * 2002-06-20: Janghoon Lyu <nandy@mizi.com>
 *    - 코드 정리. 새롭게 작성
 */

#include "config.h"
#include "machine.h"
#include "boot_kernel.h"
#include "priv_data.h"
#include "command.h"
#include "vivi_string.h"
#include "printk.h"
#include "mmu.h"
#include <types.h>
#include <setup.h>
#include <string.h>

#define LINUX_KERNEL_OFFSET	0x8000
#define LINUX_PARAM_OFFSET	0x100
#define LINUX_PAGE_SIZE		SZ_4K
#define LINUX_PAGE_SHIFT	12
#define LINUX_ZIMAGE_MAGIC	0x016f2818


/*
 * call_linux(): execute the linux kernel
 * r0 = must contain a zero or else the kernel loops
 * r1 = architecture type
 * r2 = address to be executed
 */
#ifdef CONFIG_ARCH_S3C2400
void call_linux(long a0, long a1, long a2)
{
	cache_clean_invalidate();
	tlb_invalidate();

__asm__(
	"mov	r0, %0\n"
	"mov	r1, %1\n"
	"mov	r2, %2\n"
	"mov	ip, #0\n"
	"mcr	p15, 0, ip, c13, c0, 0\n"	/* zero PID */
	"mcr	p15, 0, ip, c7, c7, 0\n"	/* invalidate I,D caches */
	"mcr	p15, 0, ip, c7, c10, 4\n"	/* drain write buffer */
	"mcr	p15, 0, ip, c8, c7, 0\n"	/* invalidate I,D TLBs */
	"mrc	p15, 0, ip, c1, c0, 0\n"	/* get control register */
	"bic	ip, ip, #0x0001\n"		/* disable MMU */
	"mcr	p15, 0, ip, c1, c0, 0\n"	/* write control register */
	"mov	pc, r2\n"
	"nop\n"
	"nop\n"
	: /* no outpus */
	: "r" (a0), "r" (a1), "r" (a2)
	);
}
#elif defined(CONFIG_ARCH_S3C2410)
void  call_linux(long a0, long a1, long a2)
{
	cache_clean_invalidate();
	tlb_invalidate();

__asm__(
	"mov	r0, %0\n"
	"mov	r1, %1\n"
	"mov	r2, %2\n"
	"mov	ip, #0\n"
	"mcr	p15, 0, ip, c13, c0, 0\n"	/* zero PID */
	"mcr	p15, 0, ip, c7, c7, 0\n"	/* invalidate I,D caches */
	"mcr	p15, 0, ip, c7, c10, 4\n"	/* drain write buffer */
	"mcr	p15, 0, ip, c8, c7, 0\n"	/* invalidate I,D TLBs */
	"mrc	p15, 0, ip, c1, c0, 0\n"	/* get control register */
	"bic	ip, ip, #0x0001\n"		/* disable MMU */
	"mcr	p15, 0, ip, c1, c0, 0\n"	/* write control register */
	"mov	pc, r2\n"
	"nop\n"
	"nop\n"
	: /* no outpus */
	: "r" (a0), "r" (a1), "r" (a2)
	);
}
#elif defined(CONFIG_ARCH_SA1100)
void  call_linux(long r0, long r1, long r2)
{
	__asm__(" mov r10, r1\n" 
		" mov r11, r2\n" 
		" mcr p15, 0, r0, c8, c7, 0\n" /* flush I and D TlB */
		" mcr p15, 0, r0, c7, c10, 4\n" /* drain the write buffer */
		" mov r3, #0x130\n"
		" mcr p15, 0, r3, c1, c0, 0\n"   /* disable the MMU */
		/* make sure the pipeline is emptied */
		" mov r0, #0\n"  
		" mov r0, r0\n"  
		" mov r0, r0\n"  
		" mov r0, r0\n"  
		" mov r0, r0\n"  
		/* zero PID in Fast Context Switch Extension PID register */
		" mov r0, #0\n"  
		" mcr p15, 0, r0, c13, c0, 0\n"
		" mov r0, #0\n"  
		" mov r1, r10\n" 
		" mov pc, r11\n"         /* jump to addr. bootloader is done */ 
		: : "r" (r0), "r" (r1), "r" (r2) );
}
#elif defined(CONFIG_ARCH_PXA250)
void  call_linux(long r0, long r1, long r2)
{
	__asm__(" mov r10, r1\n" 
		" mov r11, r2\n" 
		" mcr p15, 0, r0, c8, c7, 0\n" /* flush I and D TlB */
		" mcr p15, 0, r0, c7, c10, 4\n" /* drain the write buffer */
		" mov r3, #0x130\n"
		" mcr p15, 0, r3, c1, c0, 0\n"   /* disable the MMU */
		/* make sure the pipeline is emptied */
		" mov r0, #0\n"  
		" mov r0, r0\n"  
		" mov r0, r0\n"  
		" mov r0, r0\n"  
		" mov r0, r0\n"  
		/* zero PID in Fast Context Switch Extension PID register */
		" mov r0, #0\n"  
		" mcr p15, 0, r0, c13, c0, 0\n"
		" mov r0, #0\n"  
		" mov r1, r10\n" 
		" mov pc, r11\n"         /* jump to addr. bootloader is done */ 
		: : "r" (r0), "r" (r1), "r" (r2) );
}
#else
#error not defined call_linux() for this architecture	
#endif

/*
 * pram_base: base address of linux paramter
 */
static void setup_linux_param(ulong param_base)
{
	struct param_struct *params = (struct param_struct *)param_base; 
	char *linux_cmd;

	printk("Setup linux parameters at 0x%08lx\n", param_base);
	memset(params, 0, sizeof(struct param_struct));

	/* 꼬오옥 해줘야 될 것들.. 난디가 경험적으로 대충 찍은 것.. */
	params->u1.s.page_size = LINUX_PAGE_SIZE;
	params->u1.s.nr_pages = (DRAM_SIZE >> LINUX_PAGE_SHIFT);
#if 0
	params->u1.s.page_size = LINUX_PAGE_SIZE;
	params->u1.s.nr_pages = (dram_size >> LINUX_PAGE_SHIFT);
	params->u1.s.ramdisk_size = 0;
	params->u1.s.rootdev = rootdev;
	params->u1.s.flags = 0;

	/* TODO */
	/* If use ramdisk */
	/*
	params->u1.s.initrd_start = ?;
	params->u1.s.initrd_size = ?;
	params->u1.s.rd_start = ?;
	*/

#endif

	/* set linux command line */
	linux_cmd = get_linux_cmd_line();
	if (linux_cmd == NULL) {
		printk("Wrong magic: could not found linux command line\n");
	} else {
		memcpy(params->commandline, linux_cmd, strlen(linux_cmd) + 1);
		printk("linux command line is: \"%s\"\n", linux_cmd);
	}
}

#ifdef CONFIG_S3C2410_NAND_BOOT
extern int nand_read_ll(unsigned char*, unsigned long, int);
#endif
/*
 * dst: destination address
 * src: source
 * size: size to copy
 * mt: type of storage device
 */
static inline int copy_kernel_img(ulong dst, const char *src, size_t size, int mt)
{
	int ret = 0;
	switch (mt) {
		case MT_RAM:	/* 이미 램에 있으므로 복사할 필요 없음 */
			/* noting to do */
			break;
		case MT_NOR_FLASH:
			/* FIXME: 로딩한 커널이 제대로 읽혀지지 않아서
			 * FLASH_UNCACHED_BASE를 더합니다.
			 */
			memcpy((char *)dst, (src + FLASH_UNCACHED_BASE), size);
			break;
		case MT_SMC_S3C2410: 
#ifdef CONFIG_S3C2410_NAND_BOOT
			ret = nand_read_ll((unsigned char *)dst, 
					   (unsigned long)src, (int)size);
#endif
			break;
		case MT_UNKNOWN:
		default:
			printk("Undefined media type.\n");
			return -1;
	}
	return ret;
}

static inline int media_type_is(const char *mt)
{
	if (strncmp("ram", mt, 3) == 0) {
		return MT_RAM;
	} else if (strncmp("nor", mt, 3) == 0) {
		return MT_NOR_FLASH;
	} else if (strncmp("smc", mt, 3) == 0) {
		return MT_SMC_S3C2410;
	} else {
		return MT_UNKNOWN;
	}
}

/*
 * boot_kernel: booting the linux kernel
 *
 * from: address of stored kernel image
 * size: size of kernel image
 * media_type: a type of stoage device
 */
int boot_kernel(ulong from, size_t size, int media_type)
{
	int ret;
	ulong boot_mem_base;	/* base address of bootable memory 맞나? */
	ulong to;
	ulong mach_type;

	boot_mem_base = get_param_value("boot_mem_base", &ret);
	if (ret) {
		printk("Can't get base address of bootable memory\n");
		printk("Get default DRAM address. (0x%08lx\n", DRAM_BASE);
		boot_mem_base = DRAM_BASE;
	}

	/* copy kerne image */
	to = boot_mem_base + LINUX_KERNEL_OFFSET;
	printk("Copy linux kernel from 0x%08lx to 0x%08lx, size = 0x%08lx ... ",
		from, to, size);
	ret = copy_kernel_img(to, (char *)from, size, media_type);
	if (ret) {
		printk("failed\n");
		return -1;
	} else {
		printk("done\n");
	}

	/* 리눅스 커널 이미지가 맞는지 간단히 체크. 아니면 말구... */
	if (*(ulong *)(to + 9*4) != LINUX_ZIMAGE_MAGIC) {
		printk("Warning: this binary is not compressed linux kernel image\n");
		printk("zImage magic = 0x%08lx\n", *(ulong *)(to + 9*4));
	} else {
		printk("zImage magic = 0x%08lx\n", *(ulong *)(to + 9*4));
	}

	/* Setup linux parameters and linux command line */
	setup_linux_param(boot_mem_base + LINUX_PARAM_OFFSET);

	/* Get machine type */
	mach_type = get_param_value("mach_type", &ret);
	printk("MACH_TYPE = %d\n", mach_type);

	/* Go Go Go */
	printk("NOW, Booting Linux......\n");
	call_linux(0, mach_type, to);

	return 0;	/* 여기까지 오면 이상하죠??? */
}
	

/*
 * User Commands
 */

static inline void display_help(void)
{
	printk("invalid 'params' command: too few(many) arguments\n");
	printk("Usage:\n");
	printk("  boot <media_type> -- booting kernel \n");
	printk("    value of media_type (location of kernel image\n");
	printk("       1 = RAM\n");
	printk("       2 = NOR Flash Memory\n");
	printk("       3 = SMC (On S3C2410)\n");
	printk("  boot <media_type> <mtd_part> -- boot from specific mtd partition\n");
	printk("  boot <media_type> <addr> <size>\n");
	printk("  boot help -- help about 'boot' command\n");
}

/*
 * default values:
 *   kernel mtd partition = "kernel"
 *   base adress of bootable memory = DRAM_BASE
 *   media type = 
 *
 * avalable commands
 *
 * boot
 * boot <media_type>
 * boot <media_type> <mtd_part_name>
 * boot <media_type> <base address of stored kernel image> <kernel_size>
 * boot help
 *
 * Anyway, I need three values. this:
 *  media type, address of kernel image, size of kernel image,
 */

void command_boot(int argc, const char **argv)
{
	int media_type = 0;
	ulong from = 0;
	size_t size = 0;
	mtd_partition_t *kernel_part;
	int ret;

	switch (argc) {
		case 1:
			media_type = get_param_value("media_type", &ret);
			if (ret) {
				printk("Can't get default 'media_type'\n");
				return;
			}
			kernel_part = get_mtd_partition("kernel");
			if (kernel_part == NULL) {
				printk("Can't find default 'kernel' partition\n");
				return;
			}
			from = kernel_part->offset;
			size = kernel_part->size;
			break;
		case 2:
			if (strncmp("help", argv[1], 4) == 0) {
				display_help();
				return;
			}
			media_type = media_type_is(argv[1]);
			kernel_part = get_mtd_partition("kernel");
			from = kernel_part->offset;
			size = kernel_part->size;
			break;
		case 3:
			media_type = media_type_is(argv[1]);
			kernel_part = get_mtd_partition(argv[2]);
			from = kernel_part->offset;
			size = kernel_part->size;
			break;
		case 4:
			media_type = media_type_is(argv[1]);
			from = strtoul(argv[2], NULL, 0, NULL);
			size = strtoul(argv[3], NULL, 0, NULL);
			break;
		default:
			display_help();
			break;
	}

	boot_kernel(from, size, media_type);
}

user_command_t boot_cmd = {
	"boot",
	command_boot,
	NULL,
	"boot [{cmds}] \t\t\t-- Booting linux kernel"
};
