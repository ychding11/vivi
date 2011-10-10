/*
 * MTD map driver for AMD compatible flash chips (non-CFI)
 *
 * Author: Jonas Holmberg <jonas.holmberg@axis.com>
 *
 * $Id: amd_flash.c,v 1.5 2002/10/08 01:46:34 nandy Exp $
 *
 * Copyright (c) 2001 Axis Communications AB
 *
 * This file is under GPL.
 *
 * 
 * Author: Janghon Lyu <nandy@mizi.com>
 *
 * And then, I translated this file to the vivi.
 *
 */

#include <config.h>
#include <printk.h>
#include <heap.h>
#include <time.h>
#include <mtd/map.h>
#include <mtd/mtd.h>
#include <mtd/flashchip.h>
#include <mtd/amd_flash.h>
#include <command.h>
#include <types.h>
#include <io.h>
#include <sizes.h>
#include <vivi_string.h>
#include <errno.h>
#include <string.h>

/* debugging macros */
#undef AMDFLASH_DEBUG
#ifdef AMDFLASH_DEBUG
#define DPRINTK(args...)	printk(##args)
#else
#define DPRINTK(args...)
#endif

static const char im_name[] = "amd_flash";

static inline __u32 wide_read(struct map_info *map, __u32 addr)
{
	if (map->buswidth == 1) {
		return map->read8(map, addr);
	} else if (map->buswidth == 2) {
		return map->read16(map, addr);
	} else if (map->buswidth == 4) {
		return map->read32(map, addr);
        }

	return 0;
}

static inline void wide_write(struct map_info *map, __u32 val, __u32 addr)
{
	if (map->buswidth == 1) {
		map->write8(map, val, addr);
	} else if (map->buswidth == 2) {
		map->write16(map, val, addr);
	} else if (map->buswidth == 4) {
		map->write32(map, val, addr);
	}
}

static inline __u32 make_cmd(struct map_info *map, __u32 cmd)
{
	const struct amd_flash_private *private = map->fldrv_priv;
	if ((private->interleave == 2) &&
	    (private->device_type == DEVICE_TYPE_X16)) {
		cmd |= (cmd << 16);
	}

	return cmd;
}

static inline void send_unlock(struct map_info *map, unsigned long base)
{
	wide_write(map, (CMD_UNLOCK_DATA_1 << 16) | CMD_UNLOCK_DATA_1,
		   base + (map->buswidth * ADDR_UNLOCK_1));
	wide_write(map, (CMD_UNLOCK_DATA_2 << 16) | CMD_UNLOCK_DATA_2,
		   base + (map->buswidth * ADDR_UNLOCK_2));
}

static inline void send_cmd(struct map_info *map, unsigned long base, __u32 cmd)
{
	send_unlock(map, base);
	wide_write(map, make_cmd(map, cmd),
		   base + (map->buswidth * ADDR_UNLOCK_1));
}

static inline void send_cmd_to_addr(struct map_info *map, unsigned long base,
				    __u32 cmd, unsigned long addr)
{
	send_unlock(map, base);
	wide_write(map, make_cmd(map, cmd), addr);
}

static inline int flash_is_busy(struct map_info *map, unsigned long addr,
				int interleave)
{

	if ((interleave == 2) && (map->buswidth == 4)) {
		__u32 read1, read2;

		read1 = wide_read(map, addr);
		read2 = wide_read(map, addr);

		return (((read1 >> 16) & D6_MASK) !=
			((read2 >> 16) & D6_MASK)) ||
		       (((read1 & 0xffff) & D6_MASK) !=
			((read2 & 0xffff) & D6_MASK));
	}

	return ((wide_read(map, addr) & D6_MASK) !=
		(wide_read(map, addr) & D6_MASK));
}

static inline void unlock_sector(struct map_info *map, unsigned long sect_addr,
				 int unlock)
{
	/* Sector lock address. A6 = 1 for unlock, A6 = 0 for lock */
	int SLA = unlock ?
		(sect_addr |  (0x40 * map->buswidth)) :
		(sect_addr & ~(0x40 * map->buswidth)) ;

	__u32 cmd = make_cmd(map, CMD_UNLOCK_SECTOR);

	wide_write(map, make_cmd(map, CMD_RESET_DATA), 0);
	wide_write(map, cmd, SLA); /* 1st cycle: write cmd to any address */
	wide_write(map, cmd, SLA); /* 2nd cycle: write cmd to any address */
	wide_write(map, cmd, SLA); /* 3rd cycle: write cmd to SLA */
}

static inline int is_sector_locked(struct map_info *map,
				   unsigned long sect_addr)
{
	int status;

	wide_write(map, CMD_RESET_DATA, 0);
	send_cmd(map, sect_addr, CMD_MANUFACTURER_UNLOCK_DATA);

	/* status is 0x0000 for unlocked and 0x0001 for locked */
	status = wide_read(map, sect_addr + (map->buswidth * ADDR_SECTOR_LOCK));
	wide_write(map, CMD_RESET_DATA, 0);
	return status;
}

static int amd_flash_do_unlock(struct mtd_info *mtd, loff_t ofs, size_t len,
			       int is_unlock)
{
	struct map_info *map;
	struct mtd_erase_region_info *merip;
	int eraseoffset, erasesize, eraseblocks;
	int i;
	int retval = 0;
	int lock_status;
      
	map = mtd->priv;

	/* Pass the whole chip through sector by sector and check for each
	   sector if the sector and the given interval overlap */
	for(i = 0; i < mtd->numeraseregions; i++) {
		merip = &mtd->eraseregions[i];

		eraseoffset = merip->offset;
		erasesize = merip->erasesize;
		eraseblocks = merip->numblocks;

		if (ofs > eraseoffset + erasesize)
			continue;

		while (eraseblocks > 0) {
			if (ofs < eraseoffset + erasesize && ofs + len > eraseoffset) {
				unlock_sector(map, eraseoffset, is_unlock);

				lock_status = is_sector_locked(map, eraseoffset);
				
				if (is_unlock && lock_status) {
					printk("Cannot unlock sector at address %x length %xx\n",
					       eraseoffset, merip->erasesize);
					retval = -1;
				} else if (!is_unlock && !lock_status) {
					printk("Cannot lock sector at address %x length %x\n",
					       eraseoffset, merip->erasesize);
					retval = -1;
				}
			}
			eraseoffset += erasesize;
			eraseblocks --;
		}
	}
	return retval;
}

static int amd_flash_unlock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	return amd_flash_do_unlock(mtd, ofs, len, 1);
}

static int amd_flash_lock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
	return amd_flash_do_unlock(mtd, ofs, len, 0);
}


/*
 * Reads JEDEC manufacturer ID and device ID and returns the index of the first
 * matching table entry (-1 if not found or alias for already found chip).
 */ 
static int probe_new_chip(struct mtd_info *mtd, __u32 base,
			  struct flchip *chips,
			  struct amd_flash_private *private,
			  const struct amd_flash_info *table, int table_size)
{
	__u32 mfr_id;
	__u32 dev_id;
	struct map_info *map = mtd->priv;
	struct amd_flash_private temp;
	int i;

	temp.device_type = DEVICE_TYPE_X16;	// Assume X16 (FIXME)
	temp.interleave = 2;
	map->fldrv_priv = &temp;

	/* Enter autoselect mode. */
	send_cmd(map, base, CMD_RESET_DATA);
	send_cmd(map, base, CMD_MANUFACTURER_UNLOCK_DATA);

	mfr_id = wide_read(map, base + (map->buswidth * ADDR_MANUFACTURER));
	dev_id = wide_read(map, base + (map->buswidth * ADDR_DEVICE_ID));

	if ((map->buswidth == 4) && ((mfr_id >> 16) == (mfr_id & 0xffff)) &&
	    ((dev_id >> 16) == (dev_id & 0xffff))) {
		mfr_id &= 0xffff;
		dev_id &= 0xffff;
	} else {
		temp.interleave = 1;
	}

	for (i = 0; i < table_size; i++) {
		if ((mfr_id == table[i].mfr_id) &&
		    (dev_id == table[i].dev_id)) {
			if (chips) {
				int j;

				/* Is this an alias for an already found chip?
				 * In that case that chip should be in
				 * autoselect mode now.
				 */
				for (j = 0; j < private->numchips; j++) {
					__u32 mfr_id_other;
					__u32 dev_id_other;

					mfr_id_other =
						wide_read(map, chips[j].start +
							       (map->buswidth *
								ADDR_MANUFACTURER
							       ));
					dev_id_other =
						wide_read(map, chips[j].start +
					    		       (map->buswidth *
							        ADDR_DEVICE_ID));
					if (temp.interleave == 2) {
						mfr_id_other &= 0xffff;
						dev_id_other &= 0xffff;
					}
					if ((mfr_id_other == mfr_id) &&
					    (dev_id_other == dev_id)) {

						/* Exit autoselect mode. */
						send_cmd(map, base,
							 CMD_RESET_DATA);

						return -1;
					}
				}

				if (private->numchips == MAX_AMD_CHIPS) {
					printk("%s: Too many flash chips "
					       "detected. Increase "
					       "MAX_AMD_CHIPS from %d.\n",
					       map->name, MAX_AMD_CHIPS);

					return -1;
				}

				chips[private->numchips].start = base;
				private->numchips++;
			}

			printk("%s: Found %d x %ldMiB %s at 0x%x\n", map->name,
			       temp.interleave, (table[i].size)/(1024*1024),
			       table[i].name, base);

			mtd->size += table[i].size * temp.interleave;
			mtd->numeraseregions += table[i].numeraseregions;

			break;
		}
	}

	/* Exit autoselect mode. */
	send_cmd(map, base, CMD_RESET_DATA);

	if (i == table_size) {
		printk("%s: unknown flash device at 0x%x, "
		       "mfr id 0x%x, dev id 0x%x\n", map->name,
		       base, mfr_id, dev_id);
		map->fldrv_priv = NULL;

		return -1;
	}

	private->device_type = temp.device_type;
	private->interleave = temp.interleave;

	return i;
}



struct mtd_info *amd_flash_probe(struct map_info *map)
{
	/* Keep this table on the stack so that it gets deallocated after the
	 * probe is done.
	 */
	const struct amd_flash_info table[] = {
	{
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV160DT,
		name: "AMD AM29LV160DT",
		size: 0x00200000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x10000, numblocks: 31 },
			{ offset: 0x1F0000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x1F8000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x1FC000, erasesize: 0x04000, numblocks:  1 }
		}
	}, {
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV160DB,
		name: "AMD AM29LV160DB",
		size: 0x00200000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	}, {
		mfr_id: MANUFACTURER_TOSHIBA,
		dev_id: TC58FVT160,
		name: "Toshiba TC58FVT160",
		size: 0x00200000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x10000, numblocks: 31 },
			{ offset: 0x1F0000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x1F8000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x1FC000, erasesize: 0x04000, numblocks:  1 }
		}
	}, {
		mfr_id: MANUFACTURER_FUJITSU,
		dev_id: MBM29LV160TE,
		name: "Fujitsu MBM29LV160TE",
		size: 0x00200000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x10000, numblocks: 31 },
			{ offset: 0x1F0000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x1F8000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x1FC000, erasesize: 0x04000, numblocks:  1 }
		}
	}, {
		mfr_id: MANUFACTURER_TOSHIBA,
		dev_id: TC58FVB160,
		name: "Toshiba TC58FVB160",
		size: 0x00200000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	}, {
		mfr_id: MANUFACTURER_FUJITSU,
		dev_id: MBM29LV160BE,
		name: "Fujitsu MBM29LV160BE",
		size: 0x00200000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	}, {
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV800BB,
		name: "AMD AM29LV800BB",
		size: 0x00100000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 15 }
		}
	}, {
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29F800BB,
		name: "AMD AM29F800BB",
		size: 0x00100000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 15 }
		}
	}, {
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV800BT,
		name: "AMD AM29LV800BT",
		size: 0x00100000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x10000, numblocks: 15 },
			{ offset: 0x0F0000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x0F8000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x0FC000, erasesize: 0x04000, numblocks:  1 }
		}
	}, {
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29F800BT,
		name: "AMD AM29F800BT",
		size: 0x00100000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x10000, numblocks: 15 },
			{ offset: 0x0F0000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x0F8000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x0FC000, erasesize: 0x04000, numblocks:  1 }
		}
	}, {
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29LV800BB,
		name: "AMD AM29LV800BB",
		size: 0x00100000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x10000, numblocks: 15 },
			{ offset: 0x0F0000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x0F8000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x0FC000, erasesize: 0x04000, numblocks:  1 }
		}
	}, {
		mfr_id: MANUFACTURER_ST,
		dev_id: M29W800T,
		name: "ST M29W800T",
		size: 0x00100000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x10000, numblocks: 15 },
			{ offset: 0x0F0000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x0F8000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x0FC000, erasesize: 0x04000, numblocks:  1 }
		}
	}, {
		mfr_id: MANUFACTURER_ST,
		dev_id: M29W160DT,
		name: "ST M29W160DT",
		size: 0x00200000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x10000, numblocks: 31 },
			{ offset: 0x1F0000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x1F8000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x1FC000, erasesize: 0x04000, numblocks:  1 }
		}
	}, {
		mfr_id: MANUFACTURER_ST,
		dev_id: M29W160DB,
		name: "ST M29W160DB",
		size: 0x00200000,
		numeraseregions: 4,
		regions: {
			{ offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
			{ offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
			{ offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	}, {
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29BDS323D,
		name: "AMD AM29BDS323D",
		size: 0x00400000,
		numeraseregions: 3,
		regions: {
			{ offset: 0x000000, erasesize: 0x10000, numblocks: 48 },
			{ offset: 0x300000, erasesize: 0x10000, numblocks: 15 },
			{ offset: 0x3f0000, erasesize: 0x02000, numblocks:  8 },
		}
	}, {
		mfr_id: MANUFACTURER_AMD,
		dev_id: AM29BDS643D,
		name: "AMD AM29BDS643D",
		size: 0x00800000,
		numeraseregions: 3,
		regions: {
			{ offset: 0x000000, erasesize: 0x10000, numblocks: 96 },
			{ offset: 0x600000, erasesize: 0x10000, numblocks: 31 },
			{ offset: 0x7f0000, erasesize: 0x02000, numblocks:  8 },
		}
	}, {
		mfr_id: MANUFACTURER_ATMEL,
		dev_id: AT49xV16x,
		name: "Atmel AT49xV16x",
		size: 0x00200000,
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x02000, numblocks:  8 },
			{ offset: 0x010000, erasesize: 0x10000, numblocks: 31 }
		}
	}, {
		mfr_id: MANUFACTURER_ATMEL,
		dev_id: AT49xV16xT,
		name: "Atmel AT49xV16xT",
		size: 0x00200000,
		numeraseregions: 2,
		regions: {
			{ offset: 0x000000, erasesize: 0x10000, numblocks: 31 },
			{ offset: 0x1F0000, erasesize: 0x02000, numblocks:  8 }
               }
       }, {
               mfr_id: MANUFACTURER_HYNIX,
               dev_id: HY29LV800B,
               name: "Hynix HY29LV800B",
               size: 0x00100000,
               numeraseregions: 4,
               regions: {
                       { offset: 0x000000, erasesize: 0x04000, numblocks:  1 },
                       { offset: 0x004000, erasesize: 0x02000, numblocks:  2 },
                       { offset: 0x008000, erasesize: 0x08000, numblocks:  1 },
                       { offset: 0x010000, erasesize: 0x10000, numblocks: 15 }
               }
       }, {
               mfr_id: MANUFACTURER_HYNIX,
               dev_id: HY29LV800T,
               name: "Hynix HY29LV800T",
               size: 0x00100000,
               numeraseregions: 4,
               regions: {
                       { offset: 0x000000, erasesize: 0x10000, numblocks: 15 },
                       { offset: 0x0F0000, erasesize: 0x08000, numblocks:  1 },
                       { offset: 0x0F8000, erasesize: 0x02000, numblocks:  2 },
                       { offset: 0x0FC000, erasesize: 0x04000, numblocks:  1 }

		}
	} 
	};

	struct mtd_info *mtd;
	struct flchip chips[MAX_AMD_CHIPS];
	int table_pos[MAX_AMD_CHIPS];
	struct amd_flash_private temp;
	struct amd_flash_private *private;
	u_long size;
	unsigned long base;
	int i;
	int reg_idx;
	int offset;

	mtd = (struct mtd_info*)mmalloc(sizeof(*mtd));
	if (!mtd) {
		printk("%s: malloc failed for info structure\n", map->name);
		return NULL;
	}
	memset(mtd, 0, sizeof(*mtd));
	mtd->priv = map;

	memset(&temp, 0, sizeof(temp));

	printk("%s: Probing for AMD compatible flash...\n", map->name);

	if ((table_pos[0] = probe_new_chip(mtd, 0, NULL, &temp, table,
					   sizeof(table)/sizeof(table[0])))
	    == -1) {
		printk("%s: Found no AMD compatible device at location zero\n",
		       map->name);
		mfree(mtd);

		return NULL;
	}

	chips[0].start = 0;
	temp.numchips = 1;

	for (size = mtd->size; size > 1; size >>= 1) {
		temp.chipshift++;
	}
	switch (temp.interleave) {
		case 2:
			temp.chipshift += 1;
			break;
		case 4:
			temp.chipshift += 2;
			break;
	}

	/* Find out if there are any more chips in the map. */
	for (base = (1 << temp.chipshift);
	     base < map->size;
	     base += (1 << temp.chipshift)) {
	     	int numchips = temp.numchips;
		table_pos[numchips] = probe_new_chip(mtd, base, chips,
			&temp, table, sizeof(table)/sizeof(table[0]));
	}

	mtd->eraseregions = mmalloc(sizeof(struct mtd_erase_region_info) *
				    mtd->numeraseregions);
	if (!mtd->eraseregions) { 
		printk("%s: Failed to allocate "
		       "memory for MTD erase region info\n", map->name);
		mfree(mtd);
		map->fldrv_priv = NULL;
		return 0;
	}

	reg_idx = 0;
	offset = 0;
	for (i = 0; i < temp.numchips; i++) {
		int dev_size;
		int j;

		dev_size = 0;
		for (j = 0; j < table[table_pos[i]].numeraseregions; j++) {
			mtd->eraseregions[reg_idx].offset = offset +
				(table[table_pos[i]].regions[j].offset *
				 temp.interleave);
			mtd->eraseregions[reg_idx].erasesize =
				table[table_pos[i]].regions[j].erasesize *
				temp.interleave;
			mtd->eraseregions[reg_idx].numblocks =
				table[table_pos[i]].regions[j].numblocks;
			if (mtd->erasesize <
			    mtd->eraseregions[reg_idx].erasesize) {
				mtd->erasesize =
					mtd->eraseregions[reg_idx].erasesize;
			}
			dev_size += mtd->eraseregions[reg_idx].erasesize *
				    mtd->eraseregions[reg_idx].numblocks;
			reg_idx++;
		}
		offset += dev_size;
	}
	mtd->type = MTD_NORFLASH;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->name = map->name;
	mtd->erase = amd_flash_erase;	
	mtd->write = amd_flash_write;	
	mtd->lock = amd_flash_lock;
	mtd->unlock = amd_flash_unlock;

	private = mmalloc(sizeof(*private) + (sizeof(struct flchip) *
					      temp.numchips));
	if (!private) {
		printk("%s: kmalloc failed for private structure\n", map->name);
		mfree(mtd);
		map->fldrv_priv = NULL;
		return NULL;
	}
	memcpy(private, &temp, sizeof(temp));
	memcpy(private->chips, chips,
	       sizeof(struct flchip) * private->numchips);

	map->fldrv_priv = private;
	/* map->fldrv = &amd_flash_chipdrv; */

	return mtd;
}

static int write_one_word(struct map_info *map, struct flchip *chip,
			  unsigned long adr, __u32 datum)
{
	struct amd_flash_private *private = map->fldrv_priv;
	int ret = 0;
	int times_left;

	adr += chip->start;
	ENABLE_VPP(map);
	send_cmd(map, chip->start, CMD_PROGRAM_UNLOCK_DATA);
	wide_write(map, datum, adr);

	times_left = 500000;
	while (times_left-- && flash_is_busy(map, adr, private->interleave)) ;

	if (!times_left) {
		printk("%s: write to 0x%lx timed out!\n",
		       map->name, adr);
		ret = -EIO;
	} else {
		__u32 verify;
		if ((verify = wide_read(map, adr)) != datum) {
			printk("%s: write to 0x%lx failed. "
			       "datum = %x, verify = %x\n",
			       map->name, adr, datum, verify);
			ret = -EIO;
		}
	}

	DISABLE_VPP(map);

	return ret;
}



static int amd_flash_write(struct mtd_info *mtd, loff_t to , size_t len,
			   size_t *retlen, const u_char *buf)
{
	struct map_info *map = mtd->priv;
	struct amd_flash_private *private = map->fldrv_priv;
	int ret = 0;
	int chipnum;
	unsigned long ofs;
	unsigned long chipstart;

	*retlen = 0;
	if (!len) {
		return 0;
	}

	chipnum = to >> private->chipshift;
	ofs = to  - (chipnum << private->chipshift);
	chipstart = private->chips[chipnum].start;

	/* If it's not bus-aligned, do the first byte write. */
	if (ofs & (map->buswidth - 1)) {
		unsigned long bus_ofs = ofs & ~(map->buswidth - 1);
		int i = ofs - bus_ofs;
		int n = 0;
		u_char tmp_buf[4];
		__u32 datum;

#if 0 /* comment out by nandy */
		map->copy_from(map, tmp_buf,
			       bus_ofs + private->chips[chipnum].start,
			       map->buswidth);
#endif
		while (len && i < map->buswidth)
			tmp_buf[i++] = buf[n++], len--;

		if (map->buswidth == 2) {
			datum = *(__u16*)tmp_buf;
		} else if (map->buswidth == 4) {
			datum = *(__u32*)tmp_buf;
		} else {
			return -EINVAL;  /* should never happen, but be safe */
		}

		ret = write_one_word(map, &private->chips[chipnum], bus_ofs,
				     datum);
		if (ret) {
			return ret;
		}
		
		ofs += n;
		buf += n;
		(*retlen) += n;

		if (ofs >> private->chipshift) {
			chipnum++;
			ofs = 0;
			if (chipnum == private->numchips) {
				return 0;
			}
		}
	}
	
	/* We are now aligned, write as much as possible. */
	while(len >= map->buswidth) {
		__u32 datum;

		if (map->buswidth == 1) {
			datum = *(__u8*)buf;
		} else if (map->buswidth == 2) {
			datum = *(__u16*)buf;
		} else if (map->buswidth == 4) {
			datum = *(__u32*)buf;
		} else {
			return -EINVAL;
		}

		ret = write_one_word(map, &private->chips[chipnum], ofs, datum);

		if (ret) {
			return ret;
		}

		ofs += map->buswidth;
		buf += map->buswidth;
		(*retlen) += map->buswidth;
		len -= map->buswidth;

		if (ofs >> private->chipshift) {
			chipnum++;
			ofs = 0;
			if (chipnum == private->numchips) {
				return 0;
			}
			chipstart = private->chips[chipnum].start;
		}
	}

	if (len & (map->buswidth - 1)) {
		int i = 0, n = 0;
		u_char tmp_buf[2];
		__u32 datum;

#if 0	/* comment out by nandy */
		map->copy_from(map, tmp_buf,
			       ofs + private->chips[chipnum].start,
			       map->buswidth);
#endif
		while (len--) {
			tmp_buf[i++] = buf[n++];
		}

		if (map->buswidth == 2) {
			datum = *(__u16*)tmp_buf;
		} else if (map->buswidth == 4) {
			datum = *(__u32*)tmp_buf;
		} else {
			return -EINVAL;  /* should never happen, but be safe */
		}

		ret = write_one_word(map, &private->chips[chipnum], ofs, datum);

		if (ret) {
			return ret;
		}
		
		(*retlen) += n;
	}

	return 0;
}



static inline int erase_one_block(struct map_info *map, struct flchip *chip,
				  unsigned long adr, u_long size)
{
	unsigned long timeo = TIMEO;
	struct amd_flash_private *private = map->fldrv_priv;

	adr += chip->start;
	ENABLE_VPP(map);
	send_cmd(map, chip->start, CMD_SECTOR_ERASE_UNLOCK_DATA);
	send_cmd_to_addr(map, chip->start, CMD_SECTOR_ERASE_UNLOCK_DATA_2, adr);
	
	while (flash_is_busy(map, adr, private->interleave)) {

		/* OK Still waiting */
		if (timeo < 0) {
			printk("%s: waiting for erase to complete "
			       "timed out.\n", map->name);
			DISABLE_VPP(map);

			return -EIO;
		}
		
		udelay(1);
	}

	/* Verify every single word */
	{
		int address;
		int error = 0;
		__u8 verify;

		for (address = adr; address < (adr + size); address++) {
			if ((verify = map->read8(map, address)) != 0xFF) {
				error = 1;
				break;
			}
		}
		if (error) {
			printk("%s: verify error at 0x%x, size %ld.\n",
			       map->name, address, size);
			DISABLE_VPP(map);

			return -EIO;
		}
	}
	
	DISABLE_VPP(map);

	return 0;
}



static int amd_flash_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct map_info *map = mtd->priv;
	struct amd_flash_private *private = map->fldrv_priv;
	unsigned long adr, len;
	int chipnum;
	int ret = 0;
	int i;
	int first;
	struct mtd_erase_region_info *regions = mtd->eraseregions;

	if (instr->addr > mtd->size) {
		return -EINVAL;
	}

	if ((instr->len + instr->addr) > mtd->size) {
		return -EINVAL;
	}

	/* Check that both start and end of the requested erase are
	 * aligned with the erasesize at the appropriate addresses.
	 */

	i = 0;

        /* Skip all erase regions which are ended before the start of
           the requested erase. Actually, to save on the calculations,
           we skip to the first erase region which starts after the
           start of the requested erase, and then go back one.
        */

        while ((i < mtd->numeraseregions) &&
	       (instr->addr >= regions[i].offset)) {
               i++;
	}
        i--;

	/* OK, now i is pointing at the erase region in which this
	 * erase request starts. Check the start of the requested
	 * erase range is aligned with the erase size which is in
	 * effect here.
	 */

	if (instr->addr & (regions[i].erasesize-1)) {
		return -EINVAL;
	}

	/* Remember the erase region we start on. */

	first = i;

	/* Next, check that the end of the requested erase is aligned
	 * with the erase region at that address.
	 */

	while ((i < mtd->numeraseregions) && 
	       ((instr->addr + instr->len) >= regions[i].offset)) {
                i++;
	}

	/* As before, drop back one to point at the region in which
	 * the address actually falls.
	 */

	i--;

	if ((instr->addr + instr->len) & (regions[i].erasesize-1)) {
                return -EINVAL;
	}

	chipnum = instr->addr >> private->chipshift;
	adr = instr->addr - (chipnum << private->chipshift);
	len = instr->len;

	i = first;

	DPRINTK("Start erase\n");
	while (len) {
		DPRINTK("remain length = 0x%lx\n", len);
		ret = erase_one_block(map, &private->chips[chipnum], adr,
				      regions[i].erasesize);

		if (ret) {
			return ret;
		}

		adr += regions[i].erasesize;
		len -= regions[i].erasesize;

		if ((adr % (1 << private->chipshift)) ==
		    ((regions[i].offset + (regions[i].erasesize *
		    			   regions[i].numblocks))
		     % (1 << private->chipshift))) {
			i++;
		}

		if (adr >> private->chipshift) {
			adr = 0;
			chipnum++;
			if (chipnum >= private->numchips) {
				break;
			}
		}
	}
		
	return 0;
}



#ifdef CONFIG_CMD_AMD_FLASH
struct mtd_info *amdstd_mtd;

#define AMDFLASH_SIZE	SZ_1M
#define AMDFLASH_BUSWIDTH	DEVICE_TYPE_X16
#define AMDFLASH_BASE		0x08000000

static void
amdstd_set_vpp(struct map_info *map, int vpp)
{
	/* nothing */
}

static __u8 amdstd_read8(struct map_info *map, unsigned long ofs)
{
        return readb(map->map_priv_1 + ofs); 
}

static __u16 amdstd_read16(struct map_info *map, unsigned long ofs)
{
        return readw(map->map_priv_1 + ofs); 
}

static __u32 amdstd_read32(struct map_info *map, unsigned long ofs)
{
        return readl(map->map_priv_1 + ofs); 
}

static void amdstd_copy_from(struct map_info *map, void *to, unsigned long from, ssize_t len)
{
        memcpy(to, (void *)(map->map_priv_1 + from), len);
}

static void amdstd_write8(struct map_info *map, __u8 d, unsigned long adr)
{
        writeb(d, map->map_priv_1 + adr); 
}

static void amdstd_write16(struct map_info *map, __u16 d, unsigned long adr)
{
        writew(d, map->map_priv_1 + adr); 
}

static void amdstd_write32(struct map_info *map, __u32 d, unsigned long adr)
{
        writel(d, map->map_priv_1 + adr); 
}

static void amdstd_copy_to(struct map_info *map, unsigned long to, const void *from, ssize_t len)
{
        memcpy((void *)(map->map_priv_1 + to), from, len);
}

static struct map_info amdstd_map = {
        name:           "S3C2410 amd flash",
        read8:          amdstd_read8,
        read16:         amdstd_read16,
        read32:         amdstd_read32,
        copy_from:      amdstd_copy_from,
        write8:         amdstd_write8,
        write16:        amdstd_write16,
        write32:        amdstd_write32,
        copy_to:        amdstd_copy_to,

        map_priv_1:     AMDFLASH_BASE,
        map_priv_2:     -1,
};


static void
fill_map(struct map_info *map)
{
	map->size = AMDFLASH_SIZE;
	map->buswidth = AMDFLASH_BUSWIDTH;	/* DEVICE_TYPE_X16 */
	map->set_vpp = amdstd_set_vpp;
}

static void
command_erase(int argc, const char **argv)
{
	struct mtd_info *mtd = amdstd_mtd;
	struct erase_info instr;
	int ret;

	if (argc != 3) {
		printk("invalid 'amd erase' command: too few(many) arguments\n");
		return;
	}

	instr.addr = strtoul(argv[1], NULL, 0, NULL);
	instr.len = strtoul(argv[2], NULL, 0, NULL);

	printk("Erase from 0x%08lx-0x%08lx... ", instr.addr, instr.addr + instr.len);

	ret = amd_flash_erase(mtd, &instr);
	if (ret)
		printk("failed.\n");
	else
		printk("done.\n");
}

static void
command_write(int argc, const char **argv)
{
	struct mtd_info *mtd = amdstd_mtd;
	loff_t ofs;
	size_t size;
	u_char *from;
	size_t retlen;

	if (argc != 4) {
		printk("invalid 'amd write' command: too few(many) arguments\n");
		return;
	}

	ofs = (loff_t)strtoul(argv[1], NULL, 0, NULL);
	size = (size_t)strtoul(argv[2], NULL, 0, NULL);
	from = (u_char *)strtoul(argv[3], NULL, 0, NULL);

	amd_flash_write(mtd, ofs, size, &retlen, from);

	printk("retlen = 0x%x, %d-byte\n", retlen, retlen);
}

static void
command_probe(int arg, const char **argv)
{
	fill_map(&amdstd_map);

	amdstd_mtd = amd_flash_probe(&amdstd_map);
}

static user_subcommand_t amd_cmds[] = {
	{
		"erase",
		command_erase,
		"amd erase <ofs> <size>\t\t-- erase a region"
	}, {
		"probe", 
		command_probe, 
		"amd probe\t\t\t-- probing AMD flash"
	}, {
		"write",
		command_write,
		"amd write <ofs> <size> <buf_addr>\t-- write images"
	}, {
		NULL,
		NULL,
		NULL
	}

};

void
command_amd(int argc, const char **argv)
{
	switch (argc) {
		case 1:
			invalid_cmd("amd", amd_cmds);
			break;
		case 2:
			if (strncmp("help", argv[1], 4) == 0) {
				print_usage("", amd_cmds);
				break;
			}
		default:
			execsubcmd(amd_cmds, argc-1, argv+1);
	}
}

user_command_t amd_cmd = {
	"amd",
	command_amd,
	NULL,
	"amd [{cmds}]\t\t\t-- Manage AMD flash memory"
};
#endif /* CONFIG_CMD_AMD_FLASH */
