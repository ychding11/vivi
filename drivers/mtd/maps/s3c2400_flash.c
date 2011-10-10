/*
 * vivi/drivers/mtd/maps/s3c2400-flash.c:
 *   Flash memory access on S3C2400 based devices
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 * 
 * Based on linux/drivers/mtd/sa1100_flash.c
 *
 * This code is GPL.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/08/10 07:47:07 $
 *
 * $Revision: 1.3 $
 * $Id: s3c2400_flash.c,v 1.3 2002/08/10 07:47:07 nandy Exp $
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

#ifndef CONFIG_ARCH_S3C2400
#error This is for S3C2400 architecture only
#endif

#define WINDOW_ADDR	FLASH_UNCACHED_BASE

static __u8 s3c2400_read8(struct map_info *map, unsigned long ofs)
{
	return readb(map->map_priv_1 + ofs);
}

static __u16 s3c2400_read16(struct map_info *map, unsigned long ofs)
{
	return readw(map->map_priv_1 + ofs);
}

static __u32 s3c2400_read32(struct map_info *map, unsigned long ofs)
{
	return readl(map->map_priv_1 + ofs);
}

static void s3c2400_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
	memcpy(to, (void *)(map->map_priv_1 + from), len);
}

static void s3c2400_write8(struct map_info *map, __u8 d, unsigned long adr)
{
	writeb(d, map->map_priv_1 + adr);
}

static void s3c2400_write16(struct map_info *map, __u16 d, unsigned long adr)
{
	writew(d, map->map_priv_1 + adr);
}

static void s3c2400_write32(struct map_info *map, __u32 d, unsigned long adr)
{
	writel(d, map->map_priv_1 + adr);
}

static void s3c2400_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
	memcpy((void *)(map->map_priv_1 + to), from, len);
}

static struct map_info s3c2400_map = {
	name:		"S3C2400 flash",
	read8:		s3c2400_read8,
	read16:		s3c2400_read16,
	read32:		s3c2400_read32,
	copy_from:	s3c2400_copy_from,
	write8:		s3c2400_write8,
	write16:	s3c2400_write16,
	write32:	s3c2400_write32,
	copy_to:	s3c2400_copy_to,

	map_priv_1:	WINDOW_ADDR,
	map_priv_2:	-1,
};

extern void set_vpp(struct map_info *map, int vpp);

struct mtd_info *mymtd;

int mtd_init(void)
{
#ifndef CONFIG_MTD_CFI
	mymtd = NULL;
	return 0;
#endif
	/* Default flash buswidth */
	s3c2400_map.buswidth = FLASH_BUSWIDTH;
	/* Default flash size */
	s3c2400_map.size = FLASH_SIZE;

	s3c2400_map.set_vpp = set_vpp;

	/*
	 * Now let's probe for the actual flash.  Do it here since
	 * specific machine settings might have been set above.
	 */
	printk("S3C2400 flash: probing %d-bit flash bus\n", s3c2400_map.buswidth*8);
	mymtd = do_map_probe("cfi_probe", &s3c2400_map);
	if (!mymtd) return -ENXIO;

	return 0;
}
