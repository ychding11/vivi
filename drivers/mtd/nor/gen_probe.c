/*
 * Routines common to all CFI-type probes.
 * (C) 2001, 2001 Red Hat, Inc.
 * GPL'd
 * $Id: gen_probe.c,v 1.1.1.1 2002/07/19 08:02:35 nandy Exp $
 */

#include "mtd/mtd.h"
#include "mtd/map.h"
#include "mtd/cfi.h"
#include "mtd/gen_probe.h"
#include "printk.h"
#include "heap.h"

static struct mtd_info *check_cmd_set(struct map_info *, int);
static struct cfi_private *genprobe_ident_chips(struct map_info *map,
						struct chip_probe *cp);
static int genprobe_new_chip(struct map_info *map, struct chip_probe *cp,
			     struct cfi_private *cfi);

struct mtd_info *mtd_do_chip_probe(struct map_info *map, struct chip_probe *cp)
{
	struct mtd_info *mtd = NULL;
	struct cfi_private *cfi;

	/* First probe the map to see if we have CFI stuff there. */
	cfi = genprobe_ident_chips(map, cp);
	
	if (!cfi)
		return NULL;

	map->fldrv_priv = cfi;
	/* OK we liked it. Now find a driver for the command set it talks */

	mtd = check_cmd_set(map, 1); /* First the primary cmdset */
	if (!mtd)
		mtd = check_cmd_set(map, 0); /* Then the secondary */
	
	if (mtd)
		return mtd;

	printk("gen_probe: No supported Vendor Command Set found\n");
	
	mfree(cfi->cfiq);
	mfree(cfi);
	map->fldrv_priv = NULL;
	return NULL;
}


struct cfi_private *genprobe_ident_chips(struct map_info *map, struct chip_probe *cp)
{
	unsigned long base=0;
	struct cfi_private cfi;
	struct cfi_private *retcfi;
	struct flchip chip[MAX_CFI_CHIPS];

	memset(&cfi, 0, sizeof(cfi));

	/* Call the probetype-specific code with all permutations of 
	   interleave and device type, etc. */
	if (!genprobe_new_chip(map, cp, &cfi)) {
		/* The probe didn't like it */
		printk("%s: Found no %s device at location zero\n",
		       cp->name, map->name);
		return NULL;
	}		

	chip[0].start = 0;
	chip[0].state = FL_READY;
	cfi.chipshift = cfi.cfiq->DevSize;

	switch(cfi.interleave) {
#ifdef CFIDEV_INTERLEAVE_1
	case 1:
		break;
#endif
#ifdef CFIDEV_INTERLEAVE_2
	case 2:
		cfi.chipshift++;
		break;
#endif
#ifdef CFIDEV_INTERLEAVE_4
	case 4:
		cfi.chipshift+=2;
		break;
#endif
	default:
	}
		
	cfi.numchips = 1;

	/*
	 * Now probe for other chips, checking sensibly for aliases while
	 * we're at it. The new_chip probe above should have let the first
	 * chip in read mode.
	 *
	 * NOTE: Here, we're checking if there is room for another chip
	 *       the same size within the mapping. Therefore, 
	 *       base + chipsize <= map->size is the correct thing to do, 
	 *       because, base + chipsize would be the  _first_ byte of the
	 *       next chip, not the one we're currently pondering.
	 */

	for (base = (1<<cfi.chipshift); base + (1<<cfi.chipshift) <= map->size;
	     base += (1<<cfi.chipshift))
		cp->probe_chip(map, base, &chip[0], &cfi);

	/*
	 * Now allocate the space for the structures we need to return to 
	 * our caller, and copy the appropriate data into them.
	 */

	retcfi = mmalloc(sizeof(struct cfi_private) + cfi.numchips * sizeof(struct flchip));

	if (!retcfi) {
		printk("%s: mmalloc failed for CFI private structure\n", map->name);
		mfree(cfi.cfiq);
		return NULL;
	}

	memcpy(retcfi, &cfi, sizeof(cfi));
	memcpy(&retcfi->chips[0], chip, sizeof(struct flchip) * cfi.numchips);

	return retcfi;
}

	
static int genprobe_new_chip(struct map_info *map, struct chip_probe *cp,
			     struct cfi_private *cfi)
{
	switch (map->buswidth) {
#ifdef CFIDEV_BUSWIDTH_1		
	case CFIDEV_BUSWIDTH_1:
		cfi->interleave = CFIDEV_INTERLEAVE_1;

		cfi->device_type = CFI_DEVICETYPE_X8;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;

		cfi->device_type = CFI_DEVICETYPE_X16;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;
		break;			
#endif /* CFIDEV_BUSWITDH_1 */

#ifdef CFIDEV_BUSWIDTH_2		
	case CFIDEV_BUSWIDTH_2:
#ifdef CFIDEV_INTERLEAVE_1
		cfi->interleave = CFIDEV_INTERLEAVE_1;

		cfi->device_type = CFI_DEVICETYPE_X16;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;
#endif /* CFIDEV_INTERLEAVE_1 */
#ifdef CFIDEV_INTERLEAVE_2
		cfi->interleave = CFIDEV_INTERLEAVE_2;

		cfi->device_type = CFI_DEVICETYPE_X8;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;

		cfi->device_type = CFI_DEVICETYPE_X16;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;
#endif /* CFIDEV_INTERLEAVE_2 */
		break;			
#endif /* CFIDEV_BUSWIDTH_2 */

#ifdef CFIDEV_BUSWIDTH_4
	case CFIDEV_BUSWIDTH_4:
#if defined(CFIDEV_INTERLEAVE_1) && defined(SOMEONE_ACTUALLY_MAKES_THESE)
                cfi->interleave = CFIDEV_INTERLEAVE_1;

                cfi->device_type = CFI_DEVICETYPE_X32;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;
#endif /* CFIDEV_INTERLEAVE_1 */
#ifdef CFIDEV_INTERLEAVE_2
		cfi->interleave = CFIDEV_INTERLEAVE_2;

#ifdef SOMEONE_ACTUALLY_MAKES_THESE
		cfi->device_type = CFI_DEVICETYPE_X32;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;
#endif
		cfi->device_type = CFI_DEVICETYPE_X16;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;

		cfi->device_type = CFI_DEVICETYPE_X8;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;
#endif /* CFIDEV_INTERLEAVE_2 */
#ifdef CFIDEV_INTERLEAVE_4
		cfi->interleave = CFIDEV_INTERLEAVE_4;

#ifdef SOMEONE_ACTUALLY_MAKES_THESE
		cfi->device_type = CFI_DEVICETYPE_X32;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;
#endif
		cfi->device_type = CFI_DEVICETYPE_X16;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;

		cfi->device_type = CFI_DEVICETYPE_X8;
		if (cp->probe_chip(map, 0, NULL, cfi))
			return 1;
#endif /* CFIDEV_INTERLEAVE_4 */
		break;
#endif /* CFIDEV_BUSWIDTH_4 */

	default:
		printk("genprobe_new_chip called with unsupported buswidth %d\n", map->buswidth);
		return 0;
	}
	return 0;
}


typedef struct mtd_info *cfi_cmdset_fn_t(struct map_info *, int);

extern cfi_cmdset_fn_t cfi_cmdset_0001;
extern cfi_cmdset_fn_t cfi_cmdset_0002;

static inline struct mtd_info *cfi_cmdset_unknown(struct map_info *map, 
						  int primary)
{
	struct cfi_private *cfi = map->fldrv_priv;
	__u16 type = primary?cfi->cfiq->P_ID:cfi->cfiq->A_ID;

	/* nothing to do */    

	printk("Support for command set %04X not present\n",
	       type);

	return NULL;
}

static struct mtd_info *check_cmd_set(struct map_info *map, int primary)
{
	struct cfi_private *cfi = map->fldrv_priv;
	__u16 type = primary?cfi->cfiq->P_ID:cfi->cfiq->A_ID;
	
	if (type == P_ID_NONE || type == P_ID_RESERVED)
		return NULL;

	switch(type){
		/* Urgh. Ifdefs. The version with weak symbols was
		 * _much_ nicer. Shame it didn't seem to work on
		 * anything but x86, really.
		 * But we can't rely in inter_module_get() because
		 * that'd mean we depend on link order.
		 */
#ifdef CONFIG_MTD_CFI_INTELEXT
	case 0x0001:
	case 0x0003:
		return cfi_cmdset_0001(map, primary);
#endif
#ifdef CONFIG_MTD_CFI_AMDSTD
	case 0x0002:
		return cfi_cmdset_0002(map, primary);
#endif
	}

	return cfi_cmdset_unknown(map, primary);
}
