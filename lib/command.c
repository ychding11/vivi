/*
 * vivi/lib/command.c: 
 *   - to support user commands on the boot loader
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
 * Date  : $Date: 2002/10/22 08:08:53 $
 *
 * $Revision: 1.14 $
 *
 * 
 * History
 * 
 * 2001-12-23: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *    - Base on bootldr/bootldr.c 
 *
 * 2002-02-23: Janghoon Lyu <nandy@mizi.com>
 *    -  Add flash commands
 *
 * 2002-07-13: Janghoon Lyu <nandy@mizi.com>
 *    - 명령어 구조 변경
 *    - 두 계층으로 명확하게 구분
 *        - 메인, 서브
 *    - 메인 명령어 등록해서 사용하도록.
 *    - 서브 명령어는 메인 명령어가 있는 곳에서 알아서 사용하도록
 *
 */

#include "config.h"
#include "machine.h"
#include "command.h"
#include <priv_data.h>
#include "heap.h"
#include "vivi_string.h"
#include "vivi_lib.h"
#include "printk.h"
#include <string.h>
#include <types.h>

/*
 * 물론 head하나만으로도 처리 할 수 있다.
 * 어쨌든 중요한 문제는 아니니 head, tail
 * 구조로 놔두겠다.
 */ 
static user_command_t *head_cmd = NULL;
static user_command_t *tail_cmd = NULL;

/*
 * Parse user command line
 */
void parseargs(char *argstr, int *argc_p, char **argv, char** resid)
{
	int argc = 0;
	char c;
	enum ParseState lastState = PS_WHITESPACE;

	/* tokenize the argstr */
	while ((c = *argstr) != 0) {
		enum ParseState newState;

		if (c == ';' && lastState != PS_STRING && lastState != PS_ESCAPE)
			break;

		if (lastState == PS_ESCAPE) {
			newState = stackedState;
		} else if (lastState == PS_STRING) {
			if (c == '"') {
				newState = PS_WHITESPACE;
				*argstr = 0;
			} else {
				newState = PS_STRING;
			}
		} else if ((c == ' ') || (c == '\t')) {
			/* whitespace character */
			*argstr = 0;
			newState = PS_WHITESPACE;
		} else if (c == '"') {
			newState = PS_STRING;
			*argstr++ = 0;
			argv[argc++] = argstr;
		} else if (c == '\\') {
			stackedState = lastState;
			newState = PS_ESCAPE;
		} else {
			/* token */
			if (lastState == PS_WHITESPACE) {
				argv[argc++] = argstr;
			}
			newState = PS_TOKEN;
		}

		lastState = newState;
		argstr++;
	}

#if 0 /* for debugging */
	{
		int i;
		putLabeledWord("parseargs: argc=", argc);
		for (i = 0; i < argc; i++) {
			putstr("   ");
			putstr(argv[i]);
			putstr("\r\n");
		}
	}
#endif
	
	argv[argc] = NULL;
	if (argc_p != NULL)
		*argc_p = argc;

	if (*argstr == ';') {
		*argstr++ = '\0';
	}
	*resid = argstr;
}

/*
 * 필요한가?
 */
void unparseargs(char *argstr, int argc, const char **argv)
{
	int i;
	for (i = 0; i < argc; i++) {
		if (argv[i] != NULL) {
			strcat(argstr, " ");
			strcat(argstr, argv[i]);
		}
	}
}

/*
 * Genernal interface
 */

/*
 * For (main) commands
 */

/* add user command */
void add_command(user_command_t *cmd)
{
	if (head_cmd == NULL) {
		head_cmd = tail_cmd = cmd;
	} else {
		tail_cmd->next_cmd = cmd;
		tail_cmd = cmd;
	}
	/*printk("Registered '%s' command\n", cmd->name);*/
}

/* find command */
user_command_t *find_cmd(const char *cmdname)
{
	user_command_t *curr;

	/* do da string compare for the first offset character of cmdstr
	  against each number of the cmdlist */
	curr = head_cmd;
	while(curr != NULL) {
		if (strncmp(curr->name, cmdname, strlen(cmdname)) == 0)
			return curr;
		curr = curr->next_cmd;
	}
	return NULL;
}

/* execute a function */
void execcmd(int argc, const char **argv)
{
	user_command_t *cmd = find_cmd(argv[0]);

	if (cmd == NULL) {
		printk("Could not found '%s' command\n", argv[0]);
		printk("If you want to konw available commands, type 'help'\n"); 
		return;
	}
	/*printk("execcmd: cmd=%s, argc=%d\n", argv[0], argc);*/

	cmd->cmdfunc(argc, argv);
}

/* parse and execute a string */
void exec_string(char *buf)
{
	int argc;
	char *argv[128];
	char *resid;

	while (*buf) {
		memset(argv, 0, sizeof(argv));
		parseargs(buf, &argc, argv, &resid);
		if (argc > 0)
			execcmd(argc, (const char **)argv);
		buf = resid;
	}
}

/*
 * For sub-commands
 */
void execsubcmd(user_subcommand_t *cmds, int argc, const char **argv)
{

	while (cmds->name != NULL) {
		if (strncmp(argv[0], cmds->name, strlen(argv[0])) == 0) {
			/*printk("subexeccmd: cmd=%s, argc=%d\n", argv[0], argc);*/
			cmds->cmdfunc(argc, argv);
			return;
		}
		cmds++;
	}
	printk("Could not found '%s' sub-command\n", argv[0]);
}

void print_usage(char *strhead, user_subcommand_t *cmds)
{
	printk("Usage:\n");
	while (cmds->name != NULL) {
		if (strhead)
			printk("%s ", strhead);
		if (*cmds->helpstr)
			printk("%s\n", cmds->helpstr);
		cmds++;
	}
}

void invalid_cmd(const char *cmd_name, user_subcommand_t *cmds)
{
	printk("invalid '%s' command: wrong argumets\n", cmd_name);
	print_usage("  ", cmds);
}


/*
 * Define (basic) built-in commands
 */
#if 0

	"help [{cmds}] -- Help about help?"
	"boot [{cmds}] - Booting linux kernel"
	"call <addr> <args> -- Execute binaries"
	"dump <addr> <length> -- Display (hex dump) a range of memory."
	"flash [{cmds}]-- Manage Flash memory"
	"info -- Display vivi information"
	"load [{cmds}] -- Load a file"
	"mem -- Show info about memory"
	"reset -- Reset the System"
	"param [eval|show|save [-n]|reset]"
	"part [ help | add | delete | show | reset ] -- MTD partition"
	"test -- Test items" 
#endif

/* help command */
void command_help(int argc, const char **argv)
{
	user_command_t *curr;

	/* help <command>. invoke <command> with 'help' as an argument */
	if (argc == 2) {
		if (strncmp(argv[1], "help", strlen(argv[1])) == 0) {
			printk("Are you kidding?\n");
			return;
		}
		argv[0] = argv[1];
		argv[1] = "help";
		execcmd(argc, argv);
		return;
	}

	printk("Usage:\n");
	curr = head_cmd;
	while(curr != NULL) {
		printk("   %s\n", curr->helpstr);
		curr = curr->next_cmd;
	}
}

user_command_t help_cmd = {
	"help",
	command_help,
	NULL,
	"help [{cmds}] \t\t\t-- Help about help?"
};

/* dump command */
void command_dump(int argc, const char *argv[])
{
	size_t num;
	unsigned char *p;

	if (argc == 3)	/* size spcified */
		num = strtoul(argv[2], NULL, 0, NULL);
	else
		num = 16;

	p = (unsigned char *)strtoul(argv[1], NULL, 0, NULL);

	hex_dump(p, num);
}

user_command_t dump_cmd = {
	"dump",
	command_dump,
	NULL,
	"dump <addr> <length> \t\t-- Display (hex dump) a range of memory."
};

extern user_command_t boot_cmd;
#ifdef CONFIG_CMD_MEM
extern user_command_t mem_cmd;
#endif
extern user_command_t load_cmd;
extern user_command_t reset_cmd;
extern user_command_t go_cmd;
extern user_command_t call_cmd;
#ifdef CONFIG_CMD_PART
extern user_command_t part_cmd;
#endif
#ifdef CONFIG_CMD_PARAM
extern user_command_t param_cmd;
#endif
#ifdef CONFIG_CMD_SLEEP
extern user_command_t sleep_cmd;
#endif
#ifdef CONFIG_CMD_BONFS
extern user_command_t bon_cmd;
#endif
#ifdef CONFIG_CMD_PROMPT
extern user_command_t prompt_cmd;
#endif
#ifdef CONFIG_TEST
extern user_command_t test_cmd;
#endif
#ifdef CONFIG_CMD_AMD_FLASH
extern user_command_t amd_cmd;
#endif
#ifdef CONFIG_MD5
extern user_command_t md5sum_cmd;
#endif

/* Register basic user commands */
int init_builtin_cmds(void)
{
#ifdef CONFIG_DEBUG
	printk("init built-in commands\n");
#endif

#ifdef CONFIG_MD5
	add_command(&md5sum_cmd);
#endif
#ifdef CONFIG_CMD_AMD_FLASH
	add_command(&amd_cmd);
#endif
#ifdef CONFIG_TEST
	add_command(&test_cmd);
#endif
#ifdef CONFIG_CMD_PROMPT
	add_command(&prompt_cmd);
#endif
#ifdef CONFIG_CMD_SLEEP
	add_comamnd(&sleep_cmd);
#endif
#ifdef CONFIG_CMD_BONFS
	add_command(&bon_cmd);
#endif
	add_command(&reset_cmd);
#ifdef CONFIG_CMD_PARAM
	add_command(&param_cmd);
#endif
#ifdef CONFIG_CMD_PART
	add_command(&part_cmd);
#endif
#ifdef CONFIG_CMD_MEM
	add_command(&mem_cmd);
#endif
	add_command(&load_cmd);
	add_command(&go_cmd);
	add_command(&dump_cmd);
	add_command(&call_cmd);
	add_command(&boot_cmd);
	add_command(&help_cmd);
	return 0;
}
