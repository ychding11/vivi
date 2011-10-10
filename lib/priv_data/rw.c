/*
 * vivi/lib/priv_data.c: Management routines for vivi private data
 *
 * Copyright (C) 2002 MIZI Research, Inc. 
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/10/09 10:58:58 $
 *
 * $Revision: 1.2 $
 *
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
 * $Id: rw.c,v 1.2 2002/10/09 10:58:58 nandy Exp $
 */

#include <config.h>
#include <machine.h>
#include <priv_data.h>
#include <printk.h>
#include <types.h>
#include <string.h>
#include <mtd/mtd.h>

enum {
	NO_DEFAULT_PARAM = 1,
	NO_DEFAULT_LINUXCMD,
	NO_DEFAULT_MTDPART,
	WRONG_MAGIC_PARAM,
	WRONG_MAGIC_LINUXCMD,
	WRONG_MAGIC_MTDPART
};


int get_default_mtd_partition(void)
{
	char *src_parts = (char *)&default_mtd_partitions;
	char *dst_parts = (char *)(VIVI_PRIV_RAM_BASE + MTD_PART_OFFSET);
	int num = default_nb_part;

	if (src_parts == NULL) return -1;

	/* printk("number of mtd partitions: %d\n", num); */
	*(nb_mtd_parts) = num;

	if ((sizeof(mtd_partition_t)*num + 16) > MTD_PART_SIZE) {
		printk("too large mtd partition table\n");
		return -1;
	}

	memcpy(dst_parts, mtd_part_magic, 8);	/* copy mtd magic */
	dst_parts += 16;
	/* copy partition table */
	memcpy(dst_parts, src_parts, (sizeof(mtd_partition_t)*num));	
	return 0;
}

int get_default_param_tlb(void)
{
	char *src = (char *)&default_vivi_parameters;
	char *dst = (char *)(VIVI_PRIV_RAM_BASE + PARAMETER_TLB_OFFSET);
	int num = default_nb_params;

	if (src == NULL) return -1;

	/*printk("number of vivi parameters = %d\n", num); */
	*(nb_params) = num;

	if ((sizeof(vivi_parameter_t)*num) > PARAMETER_TLB_SIZE) {
		printk("Error: too large partition table\n");
		return -1;
	}

	memcpy(dst, vivi_param_magic, 8);
	dst += 16;
	memcpy(dst, src, (sizeof(vivi_parameter_t)*num));
	return 0;
}

int get_default_linux_cmd(void)
{
	char *src = linux_cmd;
	char *dst = (char *)(VIVI_PRIV_RAM_BASE + LINUX_CMD_OFFSET);

	if (src == NULL) return -1;

	memcpy((char *)dst, (char *)linux_cmd_magic, 8);
	dst += 8;
	memcpy(dst, src, (strlen(src) + 1));

	return 0;
}

/*
 * save vivi private data
 */
int 
save_priv_data_blk(void)
{
	char *src = (char *)VIVI_PRIV_RAM_BASE;
	loff_t dst = (loff_t)VIVI_PRIV_ROM_BASE;
	int flag = 0;
	size_t len;

#ifdef CONFIG_USE_PARAM_BLK
	{
		mtd_partition_t *part = get_mtd_partition("param");
		if (part == NULL) {
			printk("Could not found 'param' partiton\n");
			return -1;
		}
		dst = (loff_t)part->offset;
		flag = part->flag;
	}
#endif
	len = (size_t)VIVI_PRIV_SIZE;

	return write_to_flash(dst, len, src, flag);
}


#ifdef CONFIG_ARCH_S3C2410
#ifdef CONFIG_S3C2410_NAND_BOOT
extern int nand_read_ll(unsigned char *, unsigned long, int);
static inline int
read_mem(char *dst, char *src, size_t size)
{
	return nand_read_ll(dst, (unsigned long)src, (int)size);
}
#else
static inline int
read_mem(char *dst, char *src, size_t size)
{
	memcpy(dst, src, size);
	return 0;
}
#endif
#else
static inline int
read_mem(char *dst, char *src, size_t size)
{
	memcpy(dst, src, size);
	return 0;
}
#endif

int
read_saved_priv_data_blk(char *buf)
{
	char *src = (char *)(VIVI_PRIV_ROM_BASE);
	size_t size = (size_t)(VIVI_PRIV_SIZE);

#ifdef CONFIG_USE_PARAM_BLK
	{
		mtd_partition_t *part = get_mtd_partition("param");
		if (part == NULL) {
			printk("Could not found 'param' partition\n");
			return -1;
		}
		src = (char *)part->offset;
	}
#endif
	return read_mem(buf, src, size);
}

int 
load_saved_priv_data(void)
{
	char *buf = (char *)(DRAM_BASE);
	char *dst = (char *)(VIVI_PRIV_RAM_BASE);

	if (read_saved_priv_data_blk(buf)) {
		printk("invalid (saved) parameter block\n");
		return -1;
	}

	/* load parameter table */
	if (strncmp((buf + PARAMETER_TLB_OFFSET), vivi_param_magic, 8) != 0)
		return WRONG_MAGIC_PARAM;
	memcpy(dst + PARAMETER_TLB_OFFSET, buf + PARAMETER_TLB_OFFSET, 
		PARAMETER_TLB_SIZE);
	/* load linux command line */
	if (strncmp((buf + LINUX_CMD_OFFSET), linux_cmd_magic, 8) != 0)
		return WRONG_MAGIC_LINUXCMD;
	memcpy((dst + LINUX_CMD_OFFSET), buf + LINUX_CMD_OFFSET, LINUX_CMD_SIZE);
	/* load mtd partition table */	
	if (strncmp(buf + MTD_PART_OFFSET, mtd_part_magic, 8) != 0)
		return WRONG_MAGIC_MTDPART;
	memcpy(dst + MTD_PART_OFFSET, buf + MTD_PART_OFFSET, MTD_PART_SIZE);

	return 0;
}

static inline int 
get_default_priv_data(void)
{
	if (get_default_param_tlb())
		return NO_DEFAULT_PARAM;
	if (get_default_linux_cmd())
		return NO_DEFAULT_LINUXCMD;
	if (get_default_mtd_partition())
		return NO_DEFAULT_MTDPART;

	return 0;
}

int 
init_priv_data(void)
{
	int ret_def;
#ifdef CONFIG_PARSE_PRIV_DATA
	int ret_saved;
#endif


	ret_def = get_default_priv_data();
#ifdef CONFIG_PARSE_PRIV_DATA
	ret_saved = load_saved_priv_data();
	if (ret_def && ret_saved) {
		printk("Could not found vivi parameters.\n");
		return -1;
	} else if (ret_saved && !ret_def) {
		printk("Could not found stored vivi parameters.");
		printk(" Use default vivi parameters.\n");
	} else {
		printk("Found saved vivi parameters.\n");
	} 
#else
	if (ret_def) {
		printk("Could not found vivi parameters\n");
		return -1;
	} else {
		printk("Found default vivi parameters\n");
	}
#endif

#ifdef CONFIG_DEBUG_VIVI_PRIV
	display_param_tlb();
	display_mtd_partition();
#endif
	return 0;
}
