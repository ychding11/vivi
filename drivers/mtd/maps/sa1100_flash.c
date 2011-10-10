/*
 * vivi/drivers/mtd/maps/sa1100-flash.c: 
 *   Flash memory access on SA11x0 based devices
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 * 
 * Based on linux/drivers/mtd/maps/sa1100-flash.c
 *
 * This code is GPL.
 * 
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/08/21 06:31:08 $
 *
 * $Revision: 1.4 $
 * $Id: sa1100_flash.c,v 1.4 2002/08/21 06:31:08 nandy Exp $
 *
 * 기계에 따라 다른 맵 정보를 이용해서 flash 메모리를 초기화한다.
 *
 * History
 *
 * 2002-06-26: Janghoon Lyu <nandy@mizi.com>
 *    - Created this code
 *
 */

#include "config.h"
#include "machine.h"
#include "mtd/mtd.h"
#include "mtd/map.h"
#include "io.h"
#include "printk.h"
#include <types.h>
#include <errno.h>

#ifndef CONFIG_ARCH_SA1100
#error This is for SA1100 architecture only
#endif

#define WINDOW_ADDR	FLASH_UNCACHED_BASE

static __u8 sa1100_read8(struct map_info *map, unsigned long ofs)
{
	return readb(map->map_priv_1 + ofs);
}

static __u16 sa1100_read16(struct map_info *map, unsigned long ofs)
{
	return readw(map->map_priv_1 + ofs);
}

static __u32 sa1100_read32(struct map_info *map, unsigned long ofs)
{
	return readl(map->map_priv_1 + ofs);
}

static void sa1100_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	memcpy(to, (void *)(map->map_priv_1 + from), len);
}

static void sa1100_write8(struct map_info *map, __u8 d, unsigned long adr)
{
	writeb(d, map->map_priv_1 + adr);
}

static void sa1100_write16(struct map_info *map, __u16 d, unsigned long adr)
{
	writew(d, map->map_priv_1 + adr);
}

static void sa1100_write32(struct map_info *map, __u32 d, unsigned long adr)
{
	writel(d, map->map_priv_1 + adr);
}

static void sa1100_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	memcpy((void *)(map->map_priv_1 + to), from, len);
}

static struct map_info sa1100_map = {
	name:		"SA1100 flash",
	read8:		sa1100_read8,
	read16:		sa1100_read16,
	read32:		sa1100_read32,
	copy_from:	sa1100_copy_from,
	write8:		sa1100_write8,
	write16:	sa1100_write16,
	write32:	sa1100_write32,
	copy_to:	sa1100_copy_to,

	map_priv_1:	WINDOW_ADDR,
	map_priv_2:	-1,
};


extern void set_vpp(struct map_info *map, int vpp);

extern struct mtd_info *mymtd;

int mtd_init(void)
{
	/* Default flash buswidth */
	sa1100_map.buswidth = FLASH_BUSWIDTH;
	/* Default flash size */
	sa1100_map.size = FLASH_SIZE;

	sa1100_map.set_vpp = set_vpp;

	/*
	 * Now let's probe for the actual flash.  Do it here since
	 * specific machine settings might have been set above.
	 */
	printk("SA1100 flash: probing %d-bit flash bus\n", sa1100_map.buswidth*8);
	mymtd = do_map_probe("cfi_probe", &sa1100_map);
	if (!mymtd) return -ENXIO;
	return 0;
}
