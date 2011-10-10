/*
 * vivi/drivers/mtd/nand/bon.c 
 *
 * Based on vivi/util/imagewrite
 *
 * $Id: bon.c,v 1.13 2003/06/12 11:14:01 nandy Exp $
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Description: Simple management routines for the bon file system.
 *
 */

#include <config.h>
#include <mtd/mtd.h>
#include <mtd/nand_ecc.h>
#include <mtd/bonfs.h>
#include <printk.h>
#include <errno.h>
#include <heap.h>
#include <string.h>
#include <command.h>
#include <ctype.h>

extern struct mtd_info *mymtd;

#if defined(CONFIG_S3C2410_MPORT3) || defined(CONFIG_S3C2410_MPORT1)
#  define MAX_PART 		9
#else
#  define MAX_PART		5
#endif
#define MAX_RETRY		5
#define PARTITION_OFFSET	(~0)

#define IS_MTD  1
#define IS_BON  0

const char bon_part_magic[8] = {'M', 0, 0, 'I', 0, 'Z', 'I', 0};

bon_partition_t parts[MAX_PART];
int num_part;

/* sector size가 256인 2M이하의 NAND device는 처리하지 못함 */
#define NAND_SECTOR_SIZE	512
#define NAND_OOB_SIZE		16


static void
mark_bad(struct mtd_info *mtd, ulong offset)
{
	char oobbuf[NAND_OOB_SIZE];
	memset (oobbuf, 0xff, NAND_OOB_SIZE);
	oobbuf[5] = 0;
	mtd->write_oob(mtd, offset, NAND_OOB_SIZE, NULL, oobbuf);
}


static int
is_bad_block(struct mtd_info *mtd, ulong offset)
{
	unsigned char oobbuf[NAND_OOB_SIZE];
	char buf[NAND_SECTOR_SIZE];
	struct erase_info erase;
	size_t retlen;

	erase.addr = offset;
	erase.len = mtd->erasesize;

	if (mtd->erase(mtd, &erase) < 0) return 1;

	if (mtd->read_oob(mtd, offset, NAND_OOB_SIZE, &retlen, oobbuf)) {
		return -1;
	}
	if (oobbuf[5] != 0xFF) return -1;
	if (mtd->read(mtd, offset, NAND_SECTOR_SIZE, &retlen, buf)) {
		return -1;
	}
	if (retlen != NAND_SECTOR_SIZE) {
		printk("read error\n");
		mark_bad(mtd, offset);
		return -1;
	}
	return 0;
}

void
check_bad_block(struct mtd_info *mtd)
{
	int i;
	unsigned short bad_block[1024];
	for (i = 0; i < num_part; i++) {
		ulong offset = parts[i].offset;
		ulong end = (i + 1 < num_part) ? parts[i+1].offset : mtd->size;
		int num_bad = 0;
		int bad;
		printk("part = %d end = %ld\n", i, end);
		while (offset < end) {
			bad = is_bad_block(mtd, offset);
			if (bad) {
				bad_block[num_bad] = (offset - parts[i].offset) /\
							mtd->erasesize;
				num_bad++;
				printk("%lX: is bad\n", offset);
			}
			offset += mtd->erasesize;
		}
		parts[i].num_bad_block = num_bad;
#if 0 //  ???? - bushi
		parts[i].flag = 0;
#endif
		if (num_bad) {
			parts[i].bad_blocks = mmalloc(num_bad * sizeof(unsigned short));
			memcpy(parts[i].bad_blocks, bad_block, num_bad * 2);
		}
		parts[i].size = end - parts[i].offset - num_bad * mtd->erasesize;
	}
	parts[num_part - 1].size -= mtd->erasesize;
}

static ulong
calc_ecc(char *buf, unsigned char *ecc)
{
	nand_calculate_ecc(buf, ecc);
	return 0;
}

static int
write_oob(struct mtd_info *mtd, char *buf, ulong offset)
{
	char oob_buf[NAND_OOB_SIZE];
	unsigned char ecc[3];

	memset(oob_buf, 0xFF, NAND_OOB_SIZE);

	calc_ecc(buf, ecc);
	memcpy(oob_buf + 8, ecc, 3);

	calc_ecc(buf + 256, ecc);
	memcpy(oob_buf + 11, ecc, 3);

	if (mtd->write_oob(mtd, offset, NAND_OOB_SIZE, NULL, oob_buf))
		return 1;
	else
		return 0;
}

static int
write_block(struct mtd_info *mtd, ulong offset, char *buf)
{
	int i;
	size_t retlen = 0;

	if (is_bad_block(mtd, offset)) return 1;

	for (i = 0; i < (mtd->erasesize/NAND_SECTOR_SIZE); i++) {
		if (mtd->write(mtd, offset, NAND_SECTOR_SIZE, &retlen, buf))
			return 1;
		if (retlen != NAND_SECTOR_SIZE)
			return 1;
		if (write_oob(mtd, buf, offset)) return 1;
		offset += NAND_SECTOR_SIZE;
		buf += NAND_SECTOR_SIZE;
	}
	return 0;
}

int 
write_bon_image(struct mtd_info *mtd, ulong offset, char *src, long size)
{
	int bad_block_nr = 0;
	/* NAND_SECTOR_SIZE*32 > meminfo.erasesize */
	char buf[NAND_SECTOR_SIZE*32];
	ulong block;
	unsigned short *bad;
	int i;

	printk("size = %ld\n", size);

	if (offset % mtd->erasesize) {
		printk("bad alignment\n");
		return -1;
	}

	for (i = 0; i < num_part; i++) {
		if (parts[i].offset >= offset) break;
	}
	if (i == num_part) i = num_part - 1;
	if (offset + size > parts[i].offset + parts[i].size) {
		printk("image is too big for this partition\n");
		return -1;
	}
	block = (offset - parts[i].offset) / mtd->erasesize;
	bad = parts[i].bad_blocks;
	if (bad) {
		while (*bad++ <= block) block++;
	}
	offset = parts[i].offset + block * mtd->erasesize;

	memcpy(buf, src, mtd->erasesize);
	while (size > 0) {
		if (write_block(mtd, offset, buf) == 0) {
			size -= mtd->erasesize;
			src += mtd->erasesize;
			if (size > 0)
				memcpy(buf, src, mtd->erasesize);
		} else {
			int k;
			int block_nr = (offset - parts[i].offset) / mtd->erasesize;
			for (k = 0; k < parts[i].num_bad_block; k++) {
				if (block_nr == parts[i].bad_blocks[k]) break;
			}
			if (k == parts[i].num_bad_block) {
				printk("*** warning: new bad block in %d\n", block_nr);
				return -2;
			}
			bad_block_nr++;
		}
		offset += mtd->erasesize;
	}
	printk("bad_block = %d\n", bad_block_nr);
	return 0;
}

int
read_bon_partition(struct mtd_info *mtd)
{
	int i, k;
	unsigned char oobbuf[NAND_OOB_SIZE];
	char buf[NAND_SECTOR_SIZE];
	unsigned int *s;
	ulong offset = PARTITION_OFFSET;
	ssize_t retlen;
	int ret;

	if (offset > mtd->size - mtd->erasesize)
		offset = mtd->size - mtd->erasesize;

	if (!mtd->read_oob)
		ret = -EOPNOTSUPP;
	ret = mtd->read_oob(mtd, offset, NAND_OOB_SIZE, &retlen, oobbuf);
	if (ret || oobbuf[5] != 0xFF)
		return -2;
	
	if (mtd->read(mtd, offset, NAND_SECTOR_SIZE, &retlen, buf))
	   	return -3;

	if (strncmp(buf, bon_part_magic, 8) != 0)
		return -1;

	s = (unsigned int *)(buf + 8);
	num_part = *s++;

	for (i = 0; i < num_part; i++) {
		parts[i].offset = *s++;
		parts[i].size = *s++;
		parts[i].flag = *s++;
	}
	for (i = 0; i < num_part; i++) {
		parts[i].num_bad_block = *s++;
		if (parts[i].num_bad_block) {
			parts[i].bad_blocks = mmalloc(parts[i].num_bad_block * sizeof(unsigned int));
			for (k = 0; k < parts[i].num_bad_block;k++) {
				parts[i].bad_blocks[k] = *s++;
			}
		} else {
			parts[i].bad_blocks = 0;
		}
	}
	return 0;
}

ulong
read_size(char *s)
{
	ulong size = 0;
	while (isdigit(*s)) {
		size = size * 10 + *s - '0';
		s++;
	}
	if (*s == 'M' || *s == 'm') size *= 1024*1024;
	else if (*s == 'K' || *s == 'k') size *= 1024;
	else if (*s) {
		printk("hmm bad size %s\n", s);
	}
	return size;
}

ulong
read_flag(char *s)
{
	ulong flag = 0;
    
	while ( *s && *s != ':' )
		s++;

	if (*s == 0) 
		return IS_BON;

	s++;

	if (*s == 'm' || *s == 'M')
		flag |= IS_MTD;

	return flag;
}

int
write_partition(struct mtd_info *mtd, ulong offset)
{
	unsigned char oobbuf[NAND_OOB_SIZE];
	char buf[NAND_SECTOR_SIZE];
	struct erase_info erase;
	unsigned int *s;
	int i, k;
	size_t retlen;

	if (mtd->read_oob(mtd, offset, NAND_OOB_SIZE, &retlen, oobbuf))
		return -1;
	if (oobbuf[5] != 0xFF) return -1;
	if (mtd->read(mtd, offset, NAND_SECTOR_SIZE, &retlen, buf)) {
		printk("read error: mark bad: offset = %lX\n", offset);
		mark_bad(mtd, offset);
		return -1;
	}

	erase.addr = offset;
	erase.len = mtd->erasesize;

	if (mtd->erase(mtd, &erase) < 0) {
		printk("erase error: mark bad: offset = %lX\n", offset);
		mark_bad(mtd, offset);
		return -1;
	}

	memcpy(buf, bon_part_magic, 8);
	s = (unsigned int *)(buf+8);
	*s++ = num_part;
	for (i = 0; i < num_part; i++) {
		*s++ = parts[i].offset;
		*s++ = parts[i].size;
		*s++ = parts[i].flag;
	}
	for (i = 0; i < num_part; i++) {
		*s++ = parts[i].num_bad_block;
		for (k = 0; k < parts[i].num_bad_block; k++) {
			*s++ = parts[i].bad_blocks[k];
			printk("k = %d block = %d\n", k, parts[i].bad_blocks[k]);
		}
	}


	if (mtd->write(mtd, offset, NAND_SECTOR_SIZE, &retlen, buf))
		return -1;
	if (retlen != NAND_SECTOR_SIZE) {
		printk("write error: offset = %lu\n", offset);
		mark_bad(mtd, offset);
		return -1;
	}
	return 0;
}

int
write_partition_table(struct mtd_info *mtd)
{
	int i, k;
	ulong offset = PARTITION_OFFSET;
	
	if (offset > mtd->size - mtd->erasesize)
		offset = mtd->size - mtd->erasesize;

	if (write_partition(mtd, offset) != 0) {
		printk("can not write bon partition info\n");
		return -1;
	}

	for (i = 0; i < num_part; i++) {
		printk("part%d:\n", i);
		printk("\toffset = %ld\n", parts[i].offset);
		printk("\tsize = %ld\n", parts[i].size);
		printk("\tbad_block = %ld\n", parts[i].num_bad_block);
		for (k = 0; k < parts[i].num_bad_block; k++) {
			printk(" %d\n", parts[i].bad_blocks[k]);
		}
	}
	return 0;
}

void display_partition_table(void)
{
	int i = 0, k;
	unsigned int size;

	if (read_bon_partition(mymtd)) {
	   	printk("Invalid partition table info\n");
		return;
	}
	printk("BON info. (%d partitions)\n", num_part);
	printk("No: offset    \tsize        \tflags     bad\n");
	printk("---------------------------------------------\n");
	for (i = 0; i < num_part; i++) {
	  printk("%2d: 0x%08lx\t0x%08lx\t%08lx  %3d",
		 i, parts[i].offset, parts[i].size,
		 parts[i].flag, parts[i].num_bad_block);

	  print_disk_size(parts[i].size, "  ", "\n");
	}
}

#ifdef CONFIG_CMD_BONFS

extern void UpdateMtd(void);

static void display_help(void)
{
	printk("Usage:\n");
	printk("\tbon part info\n");
	printk("\tbon part <offsets>\n");
}

static void 
command_part(int argc, const char **argv)
{
	int i;
	struct mtd_info *mtd = mymtd;
	unsigned long ret;

	if (mymtd == NULL) {
		printk("we have not mtd\n");
		return;
	}

	if ((argc == 3) &&
		((strncmp("info", argv[2], 4) == 0) ||
		 (strncmp("show", argv[2], 4) == 0))) {
		display_partition_table();
		return;
	}

	/* write partition table */
	num_part = (argc - 2);
	printk("doing partition\n");
	for (i = 0; i < num_part; i++) {
		parts[i].offset = read_size((char *)argv[2+i]);
		printk("offset = %ld\n", parts[i].offset);
		parts[i].flag = read_flag((char *)argv[2+i]);
		printk("flag = %ld\n", parts[i].flag);
	}
	printk("check bad block\n");
	check_bad_block(mtd);
	write_partition_table(mtd);
	
	//add by threewater
	UpdateMtd();
}

void command_bon(int argc, const char **argv)
{
	if (strncmp("help", argv[1], 4) == 0) {
		display_help();
		return;
	}

	if (strncmp("part", argv[1], 4) == 0) {
		if (argc == 2) { 
			display_help();
			return;
		}
		command_part(argc, argv);
		return;
	}

	display_help();
}

user_command_t bon_cmd = {
	"bon",
	command_bon,
	NULL,
	"bon [{cmds}]\t\t\t\t-- Manage the bon file system"
};

#endif /* CONFIG_CMD_BONFS */
