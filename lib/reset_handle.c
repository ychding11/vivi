/* 
 * vivi/lib/reset_handle.c: reset handling
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/08/28 05:57:09 $
 *
 * $Revision: 1.1 $
 *
 * This code is GPL.
 */

#include <config.h>
#include <machine.h>
#include <vivi_string.h>
#include <memory.h>
#include <types.h>

#define HIGH	1
#define LOW	0

#define DPRINTK(x)	putstr(##x)

static int
read_bt_status(void)
{
	ulong status;

	//status = ((GPLR & (1 << GPIO_PWBT)) >> GPIO_PWBT);
	status = ((PWBT_REG & (1 << PWBT_GPIO_NUM)) >> PWBT_GPIO_NUM);
	
	if (status)
		return HIGH;
	else
		return LOW;
}

static int
is_pressed_pw_btn(void)
{
	return read_bt_status();
}

static void
hard_reset_handle(void)
{
#if 0
	clear_mem((unsigned long)(DRAM_BASE + VIVI_RAM_ABS_POS), \
		  (unsigned long)(DRAM_SIZE - VIVI_RAM_ABS_POS));
#endif
	clear_mem((unsigned long)USER_RAM_BASE, (unsigned long)USER_RAM_SIZE); 
}

static void
soft_reset_handle(void)
{
	/* nothing to do */
}

void
reset_handler(void)
{
	int pressed;

	pressed = is_pressed_pw_btn();

	if (pressed == PWBT_PRESS_LEVEL) {
		DPRINTK("HARD RESET\r\n");
		hard_reset_handle();
	} else {
		DPRINTK("SOFT RESET\r\n");
		soft_reset_handle();
	}
}
