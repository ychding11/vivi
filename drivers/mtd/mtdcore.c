/*
 * vivi/drivers/mtd/mtdcore.c: MTD쪽 관련된 것들
 *
 * Copyright (C) 2001 MIZI Research, Inc.
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
 * Date  : $Date: 2002/12/10 06:59:07 $
 *
 * $Revision: 1.16 $
 * $Id: mtdcore.c,v 1.16 2002/12/10 06:59:07 tolkien Exp $
 *
 *
 * TODO:
 *  - In a do_format_jffs2(): after erase all blocks,
 *    we need write 'cleanmarke' to every blocks if needed.
 *
 * History
 * 
 * 2001-12-23: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *
 * 2002-02-23: Janghoon Lyu <nandy@mizi.com>
 *    -  Add flash commands
 */

#include "config.h"
#include "machine.h"
#include <priv_data.h>
#include "mtd/mtd.h"
#include "mtd/cfi.h"
#include "mtd/bonfs.h"
#include "command.h"
#include "printk.h"
#include "memory.h"
#include "serial.h"
#include "vivi_string.h"
#include <types.h>
#include <errno.h>

/* temporary debugging macros */
#undef DEBUG_MTDCORE
#ifdef DEBUG_MTDCORE
#define DPRINTK(args...)	printk(##args)
#else
#define DPRINTK(args...)
#endif

typedef enum {
	WS_LOCKING,
	WS_UNLOCKING,
	WS_FM_JFFS2,
	WS_ERASING,
	WS_WRITING,
	WS_VERIFYING,
	WS_ERROR,
	WS_DONE
} ws_state_t;

struct mtd_info *mymtd = NULL;


size_t 
find_erase_size(struct mtd_info *mtd, loff_t ofs, size_t len)
{
        struct mtd_erase_region_info *regions = mtd->eraseregions;
        size_t erasesize, retlen; 
        int i, j, first, last;

	switch (mtd->type) {
	case MTD_NORFLASH:
		i = 0;
		while ((i < mtd->numeraseregions) && (ofs >= regions[i].offset)) {
			i++;
		}
		i--;
		first = i;
		i = first;
		//printk("first = %d\n", first);
		while ((i < mtd->numeraseregions) && 
			((ofs +len) >= regions[i].offset)) {
			i++;
		}
		i--;
		last = i;
		//printk("first = %d, last = %d\n", first, last);

		retlen = 0;
		for (i = first; i <= last; i++) {
			for (j = 0; j < regions[i].numblocks; j++) {
				if (len > regions[i].erasesize) {
					retlen +=  regions[i].erasesize;
					len -= regions[i].erasesize;
					//printk("retlen = 0x%x\n", retlen);
				} else {
					retlen += regions[i].erasesize;
					//printk("Oh..! retlen = 0x%x\n", retlen);
					return retlen;
				}
			}
		}
		break;
	case MTD_NANDFLASH:
		erasesize = mtd->erasesize;

	        if (len & (erasesize - 1)) {	/* not aligned */
			return ((len & ~(erasesize - 1)) + erasesize);
		} else {			/* ok. aligned */
			return (len);
		}
		break;

	default:
		printk("Something wrong\n");
		return 0;
	}

	return 0;
}

static int
do_format_jffs2(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	int ret;
	mtd_partition_t *usr;
	struct erase_info erase;

	usr = find_mtd_partition((ulong)ofs);
	if (usr == NULL) {
		printk("Can no find information of mtd partition\n");
		return -1;
	}

	erase.addr = usr->offset;
	erase.len = usr->size;

	printk("Formating... ");
	ret = mtd->erase(mtd, &erase);
	if (ret) {
		printk(" ... failed\n");
		return ret;
	}
	printk(" ... done\n");
	return 0;
}

static int
nor_unlock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	int ret;

	printk("Unlocking... ");
	ret = mtd->unlock(mtd, ofs, len);
	if (ret) {
		printk(" ... failed\n");
		return -1;
	}
	printk(" ... done\n");
	return 0;
}

static int
nor_lock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
   	int ret;
	printk("Locking...   ");
	ret = mtd->lock(mtd, ofs, len);
	if (ret < 0) {
		printk("  ... failed\n");
		return ret;
	}
	printk(" ... done\n");

	return 0;
}

static int
nor_erase(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	int ret;
	struct erase_info erase;
	erase.addr = ofs;
	erase.len = len;

	printk("Erasing...   ");
	ret = mtd->erase(mtd, &erase);
	if (ret) {
		printk(" ... failed\n");
		return ret;
	}
	printk(" ... done\n");
	return 0;
}

static int
nor_write(struct mtd_info *mtd, loff_t ofs, size_t len, 
          const u_char * buf)
{
	int ret;
	__u32 retlen = 0;
	printk("Writing...   ");
	retlen = 0;
	ret = mtd->write(mtd, ofs, len, &retlen, buf);
	if ((ret < 0) || (retlen != len)) {
		printk(" ... failed\n\tretlen = %d, ret = %d\n", retlen, ret);
		return ret;
	}
	printk(" ... done\n");
	return 0;
}

static int
nor_verify(const u_char *buf, loff_t ofs,  size_t len)
{
	int ret;

	printk("Verifying... ");
#ifdef CONFIG_MSG_PROGRESS
	ret = mem_compare(buf, (u_char*)(ulong)(ofs + FLASH_UNCACHED_BASE), 
		 	  len, 1);
#else
	ret = mem_compare(buf, (u_char*)(ulong)(ofs + FLASH_UNCACHED_BASE), 
			  len, 0);
#endif
	if (ret != len) {
		printk(" ... failed\n\tnot mached. offset = 0x%08lx, ", ret);
		printk("\tsrc = 0x%08lx, dst = 0x%08lx\n", 
			*(ulong *)((ulong)(buf + ret)), 
			*(ulong *)((ulong)(ofs + ret)));
		return -1;
	}
	printk(" ... done\n");
	return 0;
}

static inline int 
make_steps(ws_state_t *steps, int flag)
{
	int i, temp, num_step;

	steps[0] = WS_ERASING;
	steps[1] = WS_WRITING;
	steps[2] = WS_VERIFYING;
	num_step = 3;

	if (flag & MF_JFFS2) {
		steps[0] = WS_FM_JFFS2;
	}
	if (flag & MF_LOCKED) {
		temp = num_step;
		for (i = 0; i < num_step; i++) {
			steps[temp] = steps[temp-1];
			temp--;
		}
		steps[0] = WS_UNLOCKING;
		num_step++;
		steps[num_step++] = WS_LOCKING;
	}
	steps[num_step++] = WS_DONE;
	DPRINTK("number of steps = %d\n", num_step);
	return num_step;
}

static int 
write_to_nor(struct mtd_info *mtd, loff_t ofs, size_t len, 
             const u_char *buf, int flag)
{
	int ret = 0;
	ws_state_t steps[10];
	int num_step = 0, i;
	size_t blk_size = find_erase_size(mtd, ofs, len);

	DPRINTK("Found block size = 0x%08lx\n", blk_size);

	num_step = make_steps(steps, flag);

	for (i = 0; i <= num_step; i++) {
		switch(steps[i]) {
		case WS_LOCKING:
			ret = nor_lock(mtd, ofs, blk_size);
			if (ret) steps[i+1] = WS_ERROR;
			break;
		case WS_UNLOCKING:
			ret = nor_unlock(mtd, ofs, blk_size);
			if (ret) steps[i+1] = WS_ERROR;
			break;
		case WS_FM_JFFS2:
			ret = do_format_jffs2(mtd, ofs, len);
			if (ret) steps[i+1] = WS_ERROR;
			break;
		case WS_ERASING:
			ret = nor_erase(mtd, ofs, blk_size);
			if (ret) steps[i+1] = WS_ERROR;
			break; 
		case WS_WRITING:
			ret = nor_write(mtd, ofs, len, buf);
			if (ret) steps[i+1] = WS_ERROR;
			break;
		case WS_VERIFYING:
			ret = nor_verify(buf, ofs, len);
			if (ret) steps[i+1] = WS_ERROR;
			break;
		case WS_ERROR:
			printk("Error\n");
			return -1;
		case WS_DONE:
			printk("Written %d bytes\n", len);
			return 0;
		default:
			printk("Error while writing a image.\n");
			return -1;
		}
	}

	return 0;
}

static int 
write_to_nand(struct mtd_info *mtd, loff_t ofs, size_t len, 
             const u_char *buf, int flag)
{
	struct erase_info instr;
	int ret = 0;
	__u32 retlen = 0;
	size_t blk_size = find_erase_size(mtd, ofs, len);

	printk("Found block size = 0x%08lx\n", blk_size);

	instr.addr = ofs;
	instr.len = blk_size;

	if (!(flag & MF_BONFS)) {
		printk("Erasing...   ", 
			instr.addr, instr.addr + instr.len);
		ret = mtd->erase(mtd, &instr);
		if (ret) {
			printk(" ... failed\n");
			return ret;
		}
			printk(" ... done\n");
	}

	printk("Writing...   ");
	retlen = 0;
	if (flag & MF_BONFS) {
#ifdef CONFIG_MTD_SMC
		read_bon_partition(mtd);
		ret = write_bon_image(mtd, (ulong)ofs, (char *)buf, (long)len);
		retlen = len;
#endif
	} else {
		ret = mtd->write(mtd, ofs, len, &retlen, buf);
	}
	if ((ret < 0) || (retlen != len)) {
		printk(" ... failed\n\tretlen = %d, ret = %d\n", retlen, ret);
		return ret;
	} else {
		printk(" ... done\n");
	}

	/* OK. well done. */
	printk("Written %d bytes\n", retlen);
	return 0;
}
/*
 * write_to_flash(): write buffer to MTD device
 *
 * There are five stages.
 *    Stage 1: Unlock a region if you want.
 *    Stage 2: Erase a region.
 *    Stage 3: Write the buffer to a region.
 *    Stage 4: Verify result of previous stage.
 *    Stage 5: Lock a region if you want.
 *
 * Arguments:
 *    ofs: offset to write.
 *    len: length of data to write.
 *    buf: location of buffers.
 *   flag: a flag of a mtd partiton.
 *
 */
int write_to_flash(loff_t ofs, size_t len, const u_char *buf, int flag)
{
	struct mtd_info *mtd = mymtd;

	if (mymtd == NULL) {
		printk("Error. invalid MTD informations\n");
		return -1;
	}

	switch (mtd->type) {
	case MTD_NORFLASH:
		write_to_nor(mtd, ofs, len, buf, flag);
		break;
	case MTD_NANDFLASH:
		write_to_nand(mtd, ofs, len, buf, flag);
		break;
	default:
		printk("Not support this MTD\n");
		return -1;
	}
	return 0;
}


/*
 * User commands
 */
static user_subcommand_t flash_cmds[];

static void command_erase(int argc, const char **argv)
{
	struct mtd_info *mtd = mymtd;
	struct erase_info instr;
	int ret;


	if ((argc != 2) && (argc != 3)) {
		printk("invalid 'flash erase' command: too few(many) arguments\n");
		return;
	}

	if (argc == 2) {
		mtd_partition_t *part = get_mtd_partition(argv[1]);
		if (part == NULL) {
			printk("Could not found partition \"%s\"\n", argv[1]);
			return;
		}
		printk("Erasing \"%s\" parittion\n", argv[1]);
		instr.addr = part->offset;
		instr.len = part->size;
		goto do_erase;
	} else {
		instr.addr = strtoul(argv[1], NULL, 0, &ret);
		instr.len = strtoul(argv[2], NULL, 0, &ret);
		printk("Erasing block from 0x%08lx to 0x%08lx\n", 
			instr.addr, instr.addr + instr.len);
		goto do_erase;
	}

do_erase:
	printk("Erasing block from 0x%08lx to 0x%08lx... ", 
		instr.addr, instr.addr + instr.len);
	ret = mtd->erase(mtd, &instr);
	if (ret < 0)
		printk(" ... failed\n");
	else
		printk(" ... done\n");
}


static void command_lock(int argc, const char **argv)
{
	struct mtd_info *mtd = mymtd;
	loff_t	ofs;
	size_t blk_size, len;

	if (argc != 3) {
		putstr("invalid 'flash lock' command: too few(many) arguments\r\n");
		return;
	}

	ofs = (loff_t)strtoul(argv[1], NULL, 0, NULL);
	len = (size_t)strtoul(argv[2], NULL, 0, NULL);
	blk_size = find_erase_size(mtd, ofs, len);
	printk("Locking blocks 0x%08lx-0x%08lx... ", (u32)ofs, (u32)(ofs+blk_size));

	if (mtd->lock(mtd, ofs, blk_size) < 0)
		putstr("failed\r\n");
	else
		putstr("done\r\n");
}

static void command_unlock(int argc, const char **argv)
{
	struct mtd_info *mtd = mymtd;
	loff_t	ofs;
	size_t blk_size, len;

	if (argc != 3) {
		putstr("invalid 'flash unlock' command: too few(many) arguments\r\n");
		return;
	}

	ofs = (loff_t)strtoul(argv[1], NULL, 0, NULL);
	len = (size_t)strtoul(argv[2], NULL, 0, NULL);
	blk_size = find_erase_size(mtd, ofs, len);
	printk("Unlocking blocks 0x%08lx-0x%08lx... ", (u32)ofs, (u32)(ofs+blk_size));

	if (mtd->unlock(mtd, ofs, blk_size) < 0)
		putstr("failed\r\n");
	else
		putstr("done\r\n");
}

static void command_info(int argc, const char **argv)
{
    struct mtd_info *mtd = mymtd;
    struct map_info *map = (struct map_info *)(mtd->priv);
    struct cfi_private *cfi = (struct cfi_private *)(map->fldrv_priv);
    struct mtd_erase_region_info *eri = mtd->eraseregions;

    printk("Flash Memory Information\n"
	   "------------------------\n"
	   "%s: Found %d x%d devices in %d-bit mode\n"
	   "    total size:  0x%08lx (%4ldM) bytes\n"
	   "    erase block: 0x%08lx (%4ldk) x %d blocks\n\n",
	   map->name, cfi->interleave, cfi->device_type*8, map->buswidth*8,
	   mtd->size, (mtd->size)/(SZ_1M),
	   eri->erasesize, (eri->erasesize)/(SZ_1K), eri->numblocks);
}

static void command_help(int argc, const char **argv)
{
	print_usage("flash", flash_cmds);
}

static user_subcommand_t flash_cmds[] = {
{
	"help",
	command_help,
	"help"
}, {
	"erase",
	command_erase,
	"erase [<partition>] or [<start_addr> <length>]" 
}, { 
	"lock",
	command_lock,	
	"lock <start_addr> <length>" 
}, { 
	"unlock",
	command_unlock,	
	"unlock <start_addr> <length>" 
}, { 
	"info",
	command_info,	
	"info" 
}, { 
	NULL,
	NULL,	
	NULL }
};

void command_flash(int argc, const char **argv)
{
	if (mymtd == NULL) {
		printk("Error: Can not find MTD information\n");
		return;
	}

	if (argc == 1) {
		printk("invalid 'flash' command: too few arguments\n");
		command_help(0, NULL);
		return;
	}
	execsubcmd(flash_cmds, argc-1, argv+1);
}

static user_command_t flash_cmd = {
	"flash",
	command_flash,
	NULL,
	"flash [{cmds}] \t\t\t-- Manage Flash memory"
};

/*
 * Initialise
 */
extern int mtd_init(void);

int mtd_dev_init(void)
{
	int ret = 0;

#ifdef CONFIG_DEBUG
	printk("Initialize MTD device\n");
#endif
	ret = mtd_init();

#ifdef CONFIG_MTD_CFI
	add_command(&flash_cmd);
#endif
	return ret;
}
