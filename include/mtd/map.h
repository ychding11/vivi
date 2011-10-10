/*
 * vivi/include/map.h: 
 *
 * Based on linux/include/linux/mtd/map.h
 *
 * $Id: map.h,v 1.2 2002/08/10 07:47:08 nandy Exp $ 
 */

#ifndef __VIVI_MTD_MAP_H__
#define __VIVI_MTD_MAP_H__

#include "config.h"
#include "mtd.h"
#include <types.h>

/* The map stuff is very simple. You fill in your struct map_info with
   a handful of routines for accessing the device, making sure they handle
   paging etc. correctly if your device needs it. Then you pass it off
   to a chip driver which deals with a mapped device - generally either
   do_cfi_probe() or do_ram_probe(), either of which will return a 
   struct mtd_info if they liked what they saw. At which point, you
   fill in the mtd->module with your own module address, and register 
   it.
   
   The mtd->priv field will point to the struct map_info, and any further
   private data required by the chip driver is linked from the 
   mtd->priv->fldrv_priv field. This allows the map driver to get at 
   the destructor function map->fldrv_destroy() when it's tired
   of living.
*/

struct map_info {
	char *name;
	unsigned long size;
	int buswidth; /* in octets */
	__u8 (*read8)(struct map_info *, unsigned long);
	__u16 (*read16)(struct map_info *, unsigned long);
	__u32 (*read32)(struct map_info *, unsigned long);  
	__u64 (*read64)(struct map_info *, unsigned long);  
	/* If it returned a 'long' I'd call it readl.
	 * It doesn't.
	 * I won't.
	 * dwmw2 */
	
	void (*copy_from)(struct map_info *, void *, unsigned long, ssize_t);
	void (*write8)(struct map_info *, __u8, unsigned long);
	void (*write16)(struct map_info *, __u16, unsigned long);
	void (*write32)(struct map_info *, __u32, unsigned long);
	void (*write64)(struct map_info *, __u64, unsigned long);
	void (*copy_to)(struct map_info *, unsigned long, const void *, ssize_t);

	void (*set_vpp)(struct map_info *, int);
	/* We put these two here rather than a single void *map_priv, 
	   because we want mappers to be able to have quickly-accessible
	   cache for the 'currently-mapped page' without the _extra_
	   redirection that would be necessary. If you need more than
	   two longs, turn the second into a pointer. dwmw2 */
	unsigned long map_priv_1;
	unsigned long map_priv_2;
	void *fldrv_priv;
	struct mtd_chip_driver *fldrv;
};


struct mtd_chip_driver {
	struct mtd_info *(*probe)(struct map_info *map);
	char *name;
};


struct mtd_info *do_map_probe(const char *name, struct map_info *map);

#define ENABLE_VPP(map) do { if(map->set_vpp) map->set_vpp(map, 1); } while(0)
#define DISABLE_VPP(map) do { if(map->set_vpp) map->set_vpp(map, 0); } while(0)

#endif /* __VIVI_MTD_MAP_H__ */
