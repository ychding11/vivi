/*
 * vivi/lib/time.c: Simple timers
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
 * Date  : $Date: 2002/08/14 10:26:16 $
 *
 * $Revision: 1.3 $
 * $Id: time.c,v 1.3 2002/08/14 10:26:16 nandy Exp $
 *
 *
 * History
 * 
 * 2002-08-05: Janghoon Lyu <nandy@mizi.com>
 *    - Ã³À½ ¾¸
 *
 */

#include "config.h"
#include "machine.h"
#include "printk.h"
#include "time.h"
#include "vivi_string.h"
#ifdef CONFIG_CMD_SLEEP
#include "command.h"
#endif

#include <string.h>

extern void arch_udelay(unsigned int usec);
extern void arch_mdelay(unsigned int msec);

void udelay(unsigned int usec)
{
	arch_udelay(usec);
}

void mdelay(unsigned int msec)
{
	arch_mdelay(msec);
}

#ifdef CONFIG_CMD_SLEEP
void command_sleep(int argc, const char **argv)
{
	int len;
	unsigned int sec;
	char buf[10];
	char unit;

	if (argc != 2) {
		printk("invalid 'sleep' command: too few or many arguments\n");
		printk("Usage:\n");
		printk("  sleep <sec> -- 1m == mili 1u == micro 1s == secon\n");
		return;
	}

	len = strlen(argv[1]);
	strncpy(buf, argv[1], len-1);
	buf[len-1] = '\0';
	strncpy(&unit, argv[1]+(len-1), 1);
	sec = (unsigned int)strtoul(buf, NULL, 0, NULL);
	if (strncmp("s", (char *)&unit, 1) == 0) {
		printk("sleep %d seconds\n", sec);
		mdelay(sec * 1000);
	} else if (strncmp("m", (char *)&unit, 1) == 0) {
		printk("sleep %d mili-seconds\n", sec);
		mdelay(sec);
	} else if (strncmp("u", (char *)&unit, 1) == 0) {
		printk("sleep %d micro-seconds\n", sec);
		udelay(sec);
	}
}

user_command_t sleep_cmd = {
	"sleep",
	command_sleep,
	NULL,
	"merong"
};
#endif
