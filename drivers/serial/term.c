/*
 * vivi/drivers/serial/term.c: It's a simple serial termial.
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * This code is GPL.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/08/22 10:21:56 $
 *
 * $Revision: 1.2 $
 */


#include <config.h>
#include "printk.h"
#include "getcmd.h"
#include <command.h>
#include <types.h>

#define MAX_PROMPT_LEN	16
/*char prompt[16] = "vivi"; */
char prompt[MAX_PROMPT_LEN] = CONFIG_SERIAL_TERM_PROMPT;

void serial_term(void)
{
	char cmd_buf[MAX_CMDBUF_SIZE];

	for (;;) {
		printk("%s> ", prompt);

		getcmd(cmd_buf, MAX_CMDBUF_SIZE);

		/* execute a user command */
		if (cmd_buf[0])
			exec_string(cmd_buf);
	}
}

#ifdef CONFIG_CMD_PROMPT
void command_prompt(int argc, const char **argv)
{
	unsigned int len;

	switch (argc) {
	case 2:
		len = strlen(argv[1]);
		if (len >= MAX_PROMPT_LEN) {
			printk("Too long string...!\n");
			break;
		}
		memcpy(prompt, argv[1], 16);
		printk("Prompt is chagned to \"%s\"\n", prompt);
		break;
	default:
		printk("invalid 'prompt' command: too few or many arguments\n");
		printk("Usage:\n");
		printk("  prompt <string>\n");
		break;
	}
}

user_command_t prompt_cmd = {
	"prompt",
	command_prompt,
	NULL,
	"prompt <string>\t\t\t-- Change a prompt"
};
#endif
