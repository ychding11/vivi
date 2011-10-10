/*
 * vivi/lib/param.c: Simple vivi parameter management
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
 * Date  : $Date: 2002/10/12 08:48:51 $
 *
 * $Revision: 1.4 $
 * $Id: param.c,v 1.4 2002/10/12 08:48:51 nandy Exp $
 *
 *
 * History
 * 
 * 2002-12-23: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *
 * 2002-02-23: Janghoon Lyu <nandy@mizi.com>
 *    - Add commands
 *
 * 2002-06-27: Janghoon Lyu <nandy@mizi.com>
 *    - ¿ÕÃ¢ °íÄ§
 *
 */

#include "config.h"
#include "machine.h"
#include "priv_data.h"
#include "printk.h"
#include "command.h"
#include "vivi_string.h"
#include <types.h>
#include <string.h>

const char vivi_param_magic[8] = {'V', 'I', 'V', 'I', 'P', 'A', 'R', 'A'};
const char linux_cmd_magic[8] = {'V', 'I', 'V', 'I', 'C', 'M', 'D', 'L'};
	          
vivi_parameter_t *vivi_params = \
	(vivi_parameter_t *)(VIVI_PRIV_RAM_BASE + PARAMETER_TLB_OFFSET + 16);
int *nb_params = (int *)(VIVI_PRIV_RAM_BASE + PARAMETER_TLB_OFFSET + 8);
char *linux_cmd_line = (char *)(VIVI_PRIV_RAM_BASE + LINUX_CMD_OFFSET + 8);


/*
 * General Interfaces
 */

/* get parameter data by name */
vivi_parameter_t *get_param(const char *name)
{
	int i, namelen;
	vivi_parameter_t *params = vivi_params;
	int num = *(nb_params);

	namelen = strlen(name);
	for(i = 0; i < num; i++) {
		if (strncmp(name, params->name, namelen) == 0
		    && name[namelen] == 0) {
			return params;
		}
		params++;
	}
	return NULL;
}

/* get value of prameter */
param_value_t get_param_value(const char *name, int *ret)
{
	vivi_parameter_t *param = get_param(name);
	if (param == NULL) { *ret = -1; return 0; }
	*ret = 0;
	return param->value;
}


/* set parameter value */
int set_param_value(const char *name, param_value_t value)
{
	vivi_parameter_t *param = get_param(name);
	if (param == NULL) return -1;
	param->value = value;
	return 0;
}

/* display paramter table */
void display_param_tlb(void)
{
	vivi_parameter_t *params = vivi_params;
	int i, num = *(nb_params);

	printk("Number of parameters: %d\n", num);
	printk("%-24s:\t   hex\t\t   integer\n", "name");
	      /*mach_type               :  00000090               144*/
	printk("-------------------------------------------------------------\n");
	for (i = 0; i < num; i++) {
		printk("%-24s:\t%08lx\t%13lu\n", 
			params->name, params->value, (ulong)params->value);
		params++;
	}
	printk("Linux command line: %s\n", linux_cmd_line);
}

/*
 * Manage linux command line
 */
char *get_linux_cmd_line(void)
{
	char *from = (char *)(VIVI_PRIV_RAM_BASE + LINUX_CMD_OFFSET);

	if (strncmp(from, linux_cmd_magic, 8) != 0) {
		printk("Wrong magic: could not get linux command line\n");
		return NULL;
	}

	return from + 8;
}

int set_linux_cmd_line(const char *cmd)
{
	memcpy(linux_cmd_line, cmd, (strlen(cmd) + 1));
	return 0;
}


#ifdef CONFIG_CMD_PARAM
/*
 * User Commands
 */
static user_subcommand_t param_cmds[];

/*
 * External command interface
 */

void command_param(int argc, const char **argv)
{
	if (argc == 1) {
		printk("invalid 'params' command: too few arguments\n");
		print_usage(NULL, param_cmds);
		return;
	} 
	execsubcmd(param_cmds, argc-1, argv+1);
}

user_command_t param_cmd = {
	"param",
	command_param,
	NULL,
	"param [set|show|save|reset] \t-- set/get parameter"
};

/*
 * user commands ('param' command)
 */

static void command_help(int argc, const char **argv)
{
	print_usage("param", param_cmds);
}


/* 
 * Change a value of parameter
 *
 * vivi> param set <name> <value>
 *
 *     set (argv[0]): command of parameter
 *  <name> (argv[1]): name of parameter
 * <value> (argv[2]): value to set
 */
static void command_set(int argc, const char **argv)
{
	int ret;
	char *name;
	param_value_t pre_value, value;


	if (argc != 3) {
		printk("invalid 'param set' command: too few(many) arguments\n");
		return;
	}

	/* parsing arguments */
	name = (char *)argv[1];

	/* if a user want to set the 'linux command line' */
	if (strncmp(name, "linux_cmd_line", strlen(name)) == 0) {
		ret = set_linux_cmd_line(argv[2]);
		if (!ret) {
			printk("Change linux command line to \"%s\"\n", 
				linux_cmd_line);
		} else {
			printk("Failed changing linux command line\n");
		}
		return;
	}

	/* Change normag vivi parameters */
	value = (param_value_t)strtoul(argv[2], NULL, 0, &ret);
	if (ret) {
		printk("Can not parsing arguments\n");
	}

	pre_value = get_param_value(name, &ret);
	ret = set_param_value(name, value);
	if (ret) {
		printk("Error: can not change '%s' parameter value\n", name);
	}
	printk("Change '%s' value. 0x%08lx(%d) to 0x%08lx(%d)\n",
		name, pre_value, pre_value, value, value);
	return;
}

/*
 * set parameter table to default table
 *
 * vivi> param reset
 */
static void command_reset_tlb(int argc, const char **argv)
{
	if (get_default_param_tlb()) {
		printk("There are no default parameter table\n");
	}
	if (get_default_linux_cmd()) {
		printk("There are no default parameter table\n");
	}
}

/*
 * display parameter table
 *
 * vivi> param show
 */
static void command_show(int argc, const char **argv)
{
	display_param_tlb();
}

/*
 * Save vivi private data
 *
 * vivi> param save
 */
static void command_save(int argc, const char **argv)
{
	if (argc != 1) {
		invalid_cmd("param save", param_cmds);
		return;
	}

	if (save_priv_data_blk())
	  printk("Could not save vivi private data\n");
	else printk("Saved vivi private data\n");
}

/*
 * set of 'param' user command
 */
static user_subcommand_t param_cmds[] = {
{
	"help",
	command_help,
	"help \t\t\t-- Help aout 'param' command"
}, {
	"reset",
	command_reset_tlb,
	"reset \t\t\t-- Reset parameter table to default table"
}, {
	"save",
	command_save,
	"save \t\t\t-- Save parameter table to flash memeory"
}, {
	"set",
	command_set,
	"set <name> <value> \t-- Reset value of parameter\n"
	"param set linux_cmd_line \"...\" \t-- set boot parameter"
}, {
	"show",
	command_show,
	"show \t\t\t-- Display parameter table"
}, {
	NULL,
	NULL,
	NULL
}
};

#endif /* CONFIG_CMD_PARAM */
