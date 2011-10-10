/*
 * vivi/drivers/mtd/mtdpart.c: Simple MTD partitioning layer
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
 * Date  : $Date: 2003/01/03 10:31:52 $
 *
 * $Revision: 1.11 $
 * $Id: mtdpart.c,v 1.11 2003/01/03 10:31:52 tolkien Exp $
 *
 *
 * o Structure of MTD partition block
 *
 * +--------------------------------+
 * |  magic (4 bytes)               |
 * +--------------------------------+
 * |  number of partition (4 bytes) |
 * +--------------------------------+
 * |                                |
 * |  partition table               |
 * |                                |
 * +--------------------------------+
 *
 *
 * History
 * 
 * 2001-12-23: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *
 * 2002-02-23: Janghoon Lyu <nandy@mizi.com>
 *    -  Add flash commands
 *
 * 2002-07-11: Janghoon Lyu <nandy@mizi.com>
 *    -  ¿ÕÃ¢ °íÄ§
 *
 */

#include "config.h"
#include "machine.h"
#include "priv_data.h"
#include "command.h"
#include "heap.h"
#include "vivi_string.h"
#include "printk.h"
#include <types.h>
#include <string.h>

const char mtd_part_magic[8] = {'V', 'I', 'V', 'I', 'M', 'T', 'D', 'P'};

mtd_partition_t *mtd_parts = \
	(mtd_partition_t *)(VIVI_PRIV_RAM_BASE + MTD_PART_OFFSET + 16);
int *nb_mtd_parts = (int *)(VIVI_PRIV_RAM_BASE + MTD_PART_OFFSET + 8);

/*
 * General Interface
 */

/*
 * get a mtd partition by name
 */
mtd_partition_t *get_mtd_partition(const char *name)
{
	int i, num = *nb_mtd_parts;
	mtd_partition_t *parts = mtd_parts;

	for (i = 0; i < num; i++, parts++) {
		if ((strncmp(parts->name, name, strlen(name)) == 0) &&
		    (strlen(parts->name) == strlen(name))) {
			return parts;
		}
	}
	return NULL;
}

/*
 * get a information of mtd by offset
 */
mtd_partition_t *find_mtd_partition(ulong ofs)
{
	int i, num = *nb_mtd_parts;
	mtd_partition_t *parts = mtd_parts;

	for (i = 0; i < num; i++, parts++) {
		if (parts->offset == ofs) {
			return parts;
		}
	}
	return NULL;
}

/*
 * add mtd partitions
 */
int add_mtd_partition(mtd_partition_t *new_part)
{
	int num = *nb_mtd_parts;
	mtd_partition_t *parts = mtd_parts;

	memcpy((parts + num), new_part, sizeof(mtd_partition_t));
	num++;
	*(nb_mtd_parts) = num;
	return 0;
}

/*
 * delete mtd partition
 */
int del_mtd_partition(const char *part_name)
{
	int i, j;
	int num = *nb_mtd_parts;
	mtd_partition_t *parts = mtd_parts;

	for (i = 0; i < num; i++) {
		if (!strncmp((parts + i)->name, part_name, strlen(part_name)))
			break;
	}

	for (j = i; j < num; j++) {
		memcpy((parts+j), (parts+j+1), sizeof(mtd_partition_t));
	}

	num--;
	*(nb_mtd_parts) = num;

	return 0;
}

/*
 * reset mtd partitions: set mtd partition table to default partition table
 */
int reset_mtd_partition(void)
{
	return get_default_mtd_partition();
}

/*
 * print partition informations
 */
void display_mtd_partition(void)
{
    mtd_partition_t *parts = mtd_parts;
    int nb_parts = *(nb_mtd_parts);
    unsigned int size;

    printk("mtdpart info. (%d partitions)\n", nb_parts);
    printk("name              offset    \tsize        flag\n");
    printk("------------------------------------------------\n");
    for(; nb_parts > 0; nb_parts--, parts++) {
      printk("%-16s: 0x%08lx\t0x%08lx  %4d",
	     parts->name, parts->offset, parts->size, parts->flag);

      print_disk_size(parts->size, "  ", "\n");
    }
}


#ifdef CONFIG_CMD_PART
/*
 * User commands
 */

/*
 * Sub-commands
 */

static user_subcommand_t part_cmds[];


static inline ulong
flagtoul(char *flag)
{
	if (strncmp(flag, "BONFS", 5) == 0) return MF_BONFS;
	else if (strncmp(flag, "JFFS2", 5) == 0) return MF_JFFS2;
	else if (strncmp(flag, "LOCK", 5) == 0) return MF_LOCKED;
	else if (strncmp(flag, "RAM", 5) == 0) return MF_ISRAM;
	else return 0;
}

static ulong
parse_flags(char *flags)
{
	char *delim;
	ulong flag = 0;

	while (flags != NULL) {
		delim = strchr(flags, '|');
		if (delim != NULL)
			*delim++ = 0;
		flag |= flagtoul(flags);
		flags = delim;
	}
	return flag;
}
/*
 *
 * vivi> part add <name> <offset> <size> <flag>
 *
 *      add (argv[0]): command of mtd partition 
 *   <name> (argv[1]): name of partition.
 * <offset> (argv[2]): offset of partition.
 *   <size> (argv[3]): size of partition.
 *   <flag> (argv[4]): flag of partition.
 */
static void command_add(int argc, const char **argv)
{
	mtd_partition_t part;
	int ret;

	if (argc != 5) {
		invalid_cmd("part add", part_cmds);
		return;
	}

	if (argv[1] == NULL) goto error_parse_arg;
	if (strlen(argv[1]) >= MAX_PART_NAME) {
		printk("too long partition name\n");
	}
	strncpy((char *)part.name, argv[1], strlen(argv[1]) + 1);
	part.offset = strtoul(argv[2], NULL, 0, &ret);
	if (ret) goto error_parse_arg;
	part.size = strtoul(argv[3], NULL, 0, &ret);
	if (ret) goto error_parse_arg;
	part.flag = parse_flags((char *)argv[4]);

	add_mtd_partition(&part);
	printk("%s: offset = 0x%08lx, size = 0x%08lx, flag = %d\n",
		part.name, part.offset, part.size, part.flag);

	return;

error_parse_arg:
	printk("Can't parsing arguments\n");
	return;
}

/*
 * vivi> part del <name>
 *
 *    del (argv[0]): command of mtd partition
 * <name> (argv[1]): name of mtd partition
 */
static void command_del(int argc, const char **argv)
{
	if (argc != 2) {
		invalid_cmd("part del", part_cmds);
		return;
	}

	if (argv[1] == NULL) {
		printk("Can't parsing arguments\n");
		return;
	}

	del_mtd_partition(argv[1]);
	printk("deleted '%s' partition\n", argv[1]);
}

static void command_reset(int argc, const char **argv)
{
	if (argc != 1) {
		invalid_cmd("part reset", part_cmds);
		return;
	}

	reset_mtd_partition();
}

static void command_save(int argc, const char **argv)
{
	if (argc != 1) {
		invalid_cmd("part save", part_cmds);
		return;
	}
	save_priv_data_blk();
}

static void command_show(int argc, const char **argv)
{
	if (argc != 1) {
		invalid_cmd("part show", part_cmds);
		return;
	}

	display_mtd_partition();
}

static void command_help(int argc, const char **argv)
{
	print_usage("part", part_cmds);
}

/*
 * set of 'part' user command
 */
static user_subcommand_t part_cmds[] = {
{
	"help",
	command_help,
	"help"
}, {
	"add",
	command_add,
	"add <name> <offset> <size> <flag> \t-- Add a mtd partition entry"
}, {
	"del",
	command_del,
	"del <name> \t\t\t-- Delete a mtd partition entry"
}, {
	"reset",
	command_reset,
	"reset \t\t\t\t-- Reset mtd parition table"
}, {
	"save",
	command_save,
	"save \t\t\t\t-- Save mtd partition table"
}, {
	"show",
	command_show,
	"show \t\t\t\t-- Display mtd partition table"
}, {
	NULL,
	NULL,
	NULL
}
};

/*
 * Main command
 */
void command_part(int argc, const char **argv)
{
	if (argc == 1) {
		printk("invalid 'part' command: too few arguments\n");
		command_help(0, NULL);
		return;
	}
	execsubcmd(part_cmds, argc-1, argv+1);
}

user_command_t part_cmd = {
	"part", 
	command_part, 
	NULL,
	"part [add|del|show|reset] \t\t-- Manage MTD partitions"
};

#endif /* CONFIG_CMD_PART */
