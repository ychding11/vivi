/*
 * vivi/main.c: main routine
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
 * Date  : $Date: 2002/08/28 06:00:11 $
 *
 * $Revision: 1.7 $
 * $Id: main.c,v 1.7 2002/08/28 06:00:11 nandy Exp $
 *
 *
 * History
 *
 * 2001-10-xx: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 *
 * 2001-01-26: Janghoon Lyu <nandy@mizi.com>
 *    - Release VIVI Bootloader
 *
 * 2002-07-02: Janghoon Lyu <nandy@mizi.com>
 *    - bootldr의 잔재를 청소하면서 새롭게 작성
 */

#include "config.h"
#include "machine.h"
#include "mmu.h"
#include "heap.h"
#include "serial.h"
#include "printk.h"
#include "command.h"
#include "priv_data.h"
#include "getcmd.h"
#include "vivi_string.h"
#include "mtd/mtd.h"
#include "processor.h"
#include <reset_handle.h>
#include <types.h>
	
extern char *vivi_banner;

void 
vivi_shell(void)
{
#ifdef CONFIG_SERIAL_TERM
	serial_term();
#else
#error there is no terminal.
#endif
}

void run_autoboot(void)
{
	while (1) {
		exec_string("boot");
		printk("Failed 'boot' command. reentering vivi shell\n");
		/* if default boot fails, drop into the shell */
		vivi_shell();
	}
}

static void 
error(void)
{
	putstr("Sorry. We can not run vivi\n");
	for (;;) /* nothing */;		/* infinite loop */
}

#define DEFAULT_BOOT_DELAY	0x30000000
void boot_or_vivi(void)
{
	char c;
	int ret;
	ulong boot_delay;

	boot_delay = get_param_value("boot_delay", &ret);
	if (ret) boot_delay = DEFAULT_BOOT_DELAY;
	/* If a value of boot_delay is zero, 
	 * unconditionally call vivi shell */
	if (boot_delay == 0) vivi_shell();


	/*
	 * wait for a keystroke (or a button press if you want.)
	 */
	printk("Press Return to start the LINUX now, any other key for vivi\n");
	c = awaitkey(boot_delay, NULL);
	if (((c != '\r') && (c != '\n') && (c != '\0'))) {
		printk("type \"help\" for help.\n");
		vivi_shell();
	}
	run_autoboot();

	return;
}

int main(int argc, char *argv[])
{
	int ret;

	/* NB: 아직까지는 MMU가 꺼져 있음. */
	/*
	 * Step 1:
	 *  일단 간단한 배너하나 뿌리고..
	 */
	putstr("\r\n");
	putstr(vivi_banner);

	reset_handler();

	/*
	 * Step 2:
	 *   Board를 초기화 합니다.
	 */
	ret = board_init();
	if (ret) {
		putstr("Failed a board_init() procedure\r\n");
		error();
	}

	/*
	 * Step 3:
	 *   4G를 리니어(linear)하게 매핑하고, 플래쉬 메모리를
	 *   원하는 대로 매핑하고,
	 *   MMU를 켜세요.
	 */
	mem_map_init();
	mmu_init();
	putstr("Succeed memory mapping.\r\n");

	/* NB: 지금부터는 MMU가 동작할 수도 있음 */
	/*
	 * Now, vivi is running on the ram. MMU is enabled.
	 */

	/* 
	 * Step 4:
	 *   부트로더에서 동적 메모리 할당을 사용할 수 있도록
	 */
	/* initialize the heap area*/
	ret = heap_init();
	if (ret) {
		putstr("Failed initailizing heap region\r\n");
		error();
	}

	/* Step 5:
	 *    자, 위에서 메모리 매핑이 제대로 됐다면,
	 *    MTD쪽을 잡아 봅시다.
	 *    동시에, MTD의 파티션partition 정보가 있다면
	 *    잡아내겠죠.
	 */
	ret = mtd_dev_init();

	/* Step 6:
	 *   부트로더를 위한 파라미터 값들을 잡아내야겠죠.
	 */
	init_priv_data();

	/* Step 7:
	 *   가끔 사이(사악하고 이상한)한 보드들을 위해서
	 */
	misc();

	init_builtin_cmds();

	/* Step 8:
	 *   어디로 갈까나?
	 */
	boot_or_vivi();

	/* 여기까지 오면 이상하지요. 그렇지 않나요? */
	return 0;
}
