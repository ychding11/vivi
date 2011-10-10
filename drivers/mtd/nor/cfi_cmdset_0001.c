/*
 * vivi/drivers/mtd/cfi-intelext.c: Intel Extended Vendor Command Set
 *
 * Based on linux/drivers/mtd/chips/cfi_cmdset_0001.c
 *
 * $Id: cfi_cmdset_0001.c,v 1.6 2002/10/10 11:52:26 nandy Exp $
 *
 * History
 *
 * 2002-01-16: Nandy Lyu <nandy@mizi.com>
 *    - Initial code
 *
 */

#include "config.h"
#include "mtd/mtd.h"
#include "mtd/cfi.h"
#include "heap.h"
#include "printk.h"
#ifdef CONFIG_MSG_PROGRESS
#include "vivi_lib.h"
#endif
#include <types.h>
#include <errno.h>

static int cfi_intelext_write_words(struct mtd_info *, loff_t, size_t, size_t *, const u_char *);
static int cfi_intelext_write_buffers(struct mtd_info *, loff_t, size_t, size_t *, const u_char *);
static int cfi_intelext_erase_varsize(struct mtd_info *, struct erase_info *);
static int cfi_intelext_lock(struct mtd_info *mtd, loff_t ofs, size_t len);
static int cfi_intelext_unlock(struct mtd_info *mtd, loff_t ofs, size_t len);

struct mtd_info *cfi_cmdset_0001(struct map_info *, int);

static struct mtd_info *cfi_intelext_setup (struct map_info *);


#ifdef CONFIG_DEBUG_CFI
static void cfi_tell_features(struct cfi_pri_intelext *extp)
{
        int i;
        printk("  Feature/Command Support: %4.4X\n", extp->FeatureSupport);
        printk("     - Chip Erase:         %s\n", extp->FeatureSupport&1?"supported":"unsupported");
        printk("     - Suspend Erase:      %s\n", extp->FeatureSupport&2?"supported":"unsupported");
        printk("     - Suspend Program:    %s\n", extp->FeatureSupport&4?"supported":"unsupported");
        printk("     - Legacy Lock/Unlock: %s\n", extp->FeatureSupport&8?"supported":"unsupported");
        printk("     - Queued Erase:       %s\n", extp->FeatureSupport&16?"supported":"unsupported");
        printk("     - Instant block lock: %s\n", extp->FeatureSupport&32?"supported":"unsupported");
        printk("     - Protection Bits:    %s\n", extp->FeatureSupport&64?"supported":"unsupported");
        printk("     - Page-mode read:     %s\n", extp->FeatureSupport&128?"supported":"unsupported");
        printk("     - Synchronous read:   %s\n", extp->FeatureSupport&256?"supported":"unsupported");
        for (i=9; i<32; i++) {
                if (extp->FeatureSupport & (1<<i))
                        printk("     - Unknown Bit %X:      supported\n", i);
        }

        printk("  Supported functions after Suspend: %2.2X\n", extp->SuspendCmdSupport);
        printk("     - Program after Erase Suspend: %s\n", extp->SuspendCmdSupport&1?"supported":"unsupported");
        for (i=1; i<8; i++) {
                if (extp->SuspendCmdSupport & (1<<i))
                        printk("     - Unknown Bit %X:               supported\n", i);
        }

        printk("  Block Status Register Mask: %4.4X\n", extp->BlkStatusRegMask);
        printk("     - Lock Bit Active:      %s\n", extp->BlkStatusRegMask&1?"yes":"no");
        printk("     - Valid Bit Active:     %s\n", extp->BlkStatusRegMask&2?"yes":"no");
        for (i=2; i<16; i++) {
                if (extp->BlkStatusRegMask & (1<<i))
                        printk("     - Unknown Bit %X Active: yes\n",i);
        }

        printk("  Vcc Logic Supply Optimum Program/Erase Voltage: %d.%d V\n",
               extp->VccOptimal >> 8, extp->VccOptimal & 0xf);
        if (extp->VppOptimal)
                printk("  Vpp Programming Supply Optimum Program/Erase Voltage: %d.%d V\n",
                       extp->VppOptimal >> 8, extp->VppOptimal & 0xf);
}
#endif

/* This routine is made available to other mtd code via
 * inter_module_register.  It must only be accessed through
 * inter_module_get which will bump the use count of this module.  The
 * addresses passed back in cfi are valid as long as the use count of
 * this module is non-zero, i.e. between inter_module_get and
 * inter_module_put.  Keith Owens <kaos@ocs.com.au> 29 Oct 2000.
 */
struct mtd_info *cfi_cmdset_0001(struct map_info *map, int primary)
{
        struct cfi_private *cfi = map->fldrv_priv;
        int i;
        __u32 base = cfi->chips[0].start;

        if (cfi->cfi_mode == CFI_MODE_CFI) {
                /* 
                 * It's a real CFI chip, not one for which the probe
                 * routine faked a CFI structure. So we read the feature
                 * table from it.
                 */
                __u16 adr = primary?cfi->cfiq->P_ADR:cfi->cfiq->A_ADR;
                struct cfi_pri_intelext *extp;
                int ofs_factor = cfi->interleave * cfi->device_type;

                //printk(" Intel/Sharp Extended Query Table at 0x%4.4X\n", adr);
                if (!adr)
                        return NULL;

                /* Switch it into Query Mode */
                cfi_send_gen_cmd(0x98, 0x55, base, map, cfi, cfi->device_type, NULL);

                extp = mmalloc(sizeof(*extp));
                if (!extp) {
                        printk("Failed to allocate memory\n");
                        return NULL;
                }
         
                /* Read in the Extended Query Table */
                for (i=0; i<sizeof(*extp); i++) {
                        ((unsigned char *)extp)[i] = 
                                cfi_read_query(map, (base+((adr+i)*ofs_factor)));
                }
         
                if (extp->MajorVersion != '1' ||  
                    (extp->MinorVersion < '0' || extp->MinorVersion > '2')) { 
                        printk("  Unknown IntelExt Extended Query "
                               "version %c.%c.\n",  extp->MajorVersion,
                               extp->MinorVersion);
                        mfree(extp);
                        return NULL;
                }
         
                /* Do some byteswapping if necessary */
                extp->FeatureSupport = extp->FeatureSupport;
                extp->BlkStatusRegMask = extp->BlkStatusRegMask;
                extp->ProtRegAddr = extp->ProtRegAddr;
         
#ifdef CONFIG_DEBUG_CFI
                /* Tell the user about it in lots of lovely detail */
                cfi_tell_features(extp);
#endif

                /* Install our own private info structure */
                cfi->cmdset_priv = extp;
        }

        for (i=0; i< cfi->numchips; i++) {
                cfi->chips[i].word_write_time = 128;
                cfi->chips[i].buffer_write_time = 128;
                cfi->chips[i].erase_time = 1024;
        }

        /* Make sure it's in read mode */
        cfi_send_gen_cmd(0xff, 0x55, base, map, cfi, cfi->device_type, NULL);
        return cfi_intelext_setup(map);
}

static struct mtd_info *cfi_intelext_setup(struct map_info *map)
{
	struct cfi_private *cfi = map->fldrv_priv;
	struct mtd_info *mtd;
	unsigned long offset = 0;
	int i, j;
	unsigned long devsize = (1 << cfi->cfiq->DevSize) * cfi->interleave;

	mtd = mmalloc(sizeof(*mtd));

	if (!mtd) {
		printk("Failed to allocate memory for MTD device\n");
		mfree(cfi->cmdset_priv);
		return NULL;
	}

	memset(mtd, 0, sizeof(*mtd));
	mtd->priv = map;
	mtd->type = MTD_NORFLASH;
	mtd->size = devsize * cfi->numchips;
	
	mtd->numeraseregions = cfi->cfiq->NumEraseRegions * cfi->numchips;
	mtd->eraseregions = mmalloc(sizeof(struct mtd_erase_region_info) 
				    * mtd->numeraseregions);
	if (!mtd->eraseregions) { 
		printk("Failed to allocate memory for MTD erase region info\n");
		mfree(cfi->cmdset_priv);
		return NULL;
	}

	for (i = 0; i < cfi->cfiq->NumEraseRegions; i++) {
		unsigned long ernum, ersize;
		ersize = ((cfi->cfiq->EraseRegionInfo[i] >> 8) & ~0xff) * cfi->interleave;
		ernum = (cfi->cfiq->EraseRegionInfo[i] & 0xffff) + 1;

		if (mtd->erasesize < ersize) {
			mtd->erasesize = ersize;
		}
		for (j = 0; j < cfi->numchips; j++) {
			mtd->eraseregions[(j*cfi->cfiq->NumEraseRegions)+i].offset = (j*devsize)+offset;
			mtd->eraseregions[(j*cfi->cfiq->NumEraseRegions)+i].erasesize = ersize;
			mtd->eraseregions[(j*cfi->cfiq->NumEraseRegions)+i].numblocks = ernum;
		}
		offset += (ersize * ernum);
	}

	if (offset != devsize) {
		/* Argh */
		printk("Sum of region (%lx) != total size of set of interleave chips (%lx)\n", offset, devsize);
		mfree(mtd->eraseregions);
		mfree(cfi->cmdset_priv);
		return NULL;
	}

	for (i = 0; i < mtd->numeraseregions; i++) {
		printk("%d: offset = 0x%x, size = 0x%x, blocks = %d\n", 
			i, mtd->eraseregions[i].offset,
			mtd->eraseregions[i].erasesize,
			mtd->eraseregions[i].numblocks);
	}

	/* Also select the correct geometry setup too */
	mtd->erase = cfi_intelext_erase_varsize;
#ifndef FORCE_WORD_WRTE
	if (cfi->cfiq->BufWriteTimeoutTyp) {
		printk("Using buffer write method\n");
		mtd->write = cfi_intelext_write_buffers;
	} else {
#else
	}
#endif
		printk("Using word write method\n");
		mtd->write = cfi_intelext_write_words;
	}

	mtd->lock = cfi_intelext_lock;
	mtd->unlock = cfi_intelext_unlock;
	mtd->flags = MTD_CAP_NORFLASH;
	mtd->name = map->name;
	return mtd;
}

int do_write_oneword(struct map_info *map, struct flchip *chip, 
                     unsigned long adr, cfi_word datum)
{
	struct cfi_private *cfi = map->fldrv_priv;
	cfi_word status = 0, status_OK;
	unsigned long timeo;

	adr += chip->start;

	/* Let's determine this according to the interleave only once */
	status_OK = CMD(0x80);

	/* 그냥 사용 했음 - 2002-06-25 nandy */
	timeo = cfi->cfiq->WordWriteTimeoutMax * 1000;

	ENABLE_VPP(map);
	cfi_write(map, CMD(0x40), adr);
	cfi_write(map, datum, adr);

	/* wait for it to be programmed */
	while (timeo > 0) {
		status = cfi_read(map, adr);
		if ((status & status_OK) == status_OK)
			break;
		timeo--;
	}

	/* done */
	DISABLE_VPP(map);
	/* clear status */
	cfi_write(map, CMD(0x50), adr);

	/* report errors */
	if (timeo <= 0) {
		printk("\nFailed do_write_oneword() operation\n");
		printk("  Offset = 0x%08lx, datum = 0x%08lx, status = 0x%08lx\n",
			adr, datum, status);
		cfi_write(map, CMD(0xff), adr);
		return -EIO;
	}

	cfi_write(map, CMD(0xff), adr);
	return 0;
}

static int cfi_intelext_write_words(struct mtd_info *mtd,  loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	struct map_info *map = mtd->priv;
	struct cfi_private *cfi = map->fldrv_priv;
	int ret = 0;
	int chipnum;
	unsigned long ofs;
	
	*retlen = 0;

	if (!len)
		return 0;

	chipnum = to >> cfi->chipshift;
	ofs = to - (chipnum << cfi->chipshift);

	/* It it's not bus-aligned, do the first byte write */
	if (ofs & (CFIDEV_BUSWIDTH-1)) {
		unsigned long bus_ofs = ofs & ~(CFIDEV_BUSWIDTH-1);
		int gap = ofs - bus_ofs;
		int i = 0, n = 0;
		u_char tmp_buf[8];
		cfi_word datum;

		while (gap--)
			tmp_buf[i++] = 0xff;
		while (len && i < CFIDEV_BUSWIDTH)
			tmp_buf[i++] = buf[n++], len--;
		while (i < CFIDEV_BUSWIDTH)
			tmp_buf[i++] = 0xff;

		if (cfi_buswidth_is_2()) {
			datum = *(__u16*)tmp_buf;
		} else if (cfi_buswidth_is_4()) {
			datum = *(__u32*)tmp_buf;
		} else if (cfi_buswidth_is_8()) {
			datum = *(__u64*)tmp_buf;
		} else {
			return -EINVAL;	/* should never happen, but be safe */
		}

		ret = do_write_oneword(map, &cfi->chips[chipnum], bus_ofs, datum);
		if (ret) return ret;

		ofs += n;
		buf += n;
		(*retlen) += n;

		if (ofs >> cfi->chipshift) {
			chipnum++;
			ofs = 0;
			if (chipnum == cfi->numchips) 
				return 0;
		}
	}

	while (len >= CFIDEV_BUSWIDTH) {
		cfi_word datum;

		if (cfi_buswidth_is_1()) {
			datum = *(__u8*)buf;
		} else if (cfi_buswidth_is_2()) {
			datum = *(__u16*)buf;
		} else if (cfi_buswidth_is_4()) {
			datum = *(__u32*)buf;
		} else if (cfi_buswidth_is_8()) {
			datum = *(__u64*)buf;
		} else {
			return -EINVAL;
		}

		ret = do_write_oneword(map, &cfi->chips[chipnum], ofs, datum);

		if (ret) return ret;

		ofs += CFIDEV_BUSWIDTH;
		buf += CFIDEV_BUSWIDTH;
		(*retlen) += CFIDEV_BUSWIDTH;
		len -= CFIDEV_BUSWIDTH;

		if (ofs >> cfi->chipshift) {
			chipnum++;
			ofs = 0;
			if (chipnum == cfi->numchips)
				return 0;
		}
	}

	if (len & (CFIDEV_BUSWIDTH-1)) {
		int i = 0, n = 0;
		u_char tmp_buf[8];
		cfi_word datum;

		while (len--)
			tmp_buf[i++] = buf[n++];
		while (i < CFIDEV_BUSWIDTH)
			tmp_buf[i++] = 0xff;

		if (cfi_buswidth_is_2()) {
			datum = *(__u16*)tmp_buf;
		} else if (cfi_buswidth_is_4()) {
			datum = *(__u32*)tmp_buf;
		} else if (cfi_buswidth_is_8()) {
			datum = *(__u64*)tmp_buf;
		} else {
			return -EINVAL;	/* should never happen, but be safe */
		}

		ret = do_write_oneword(map, &cfi->chips[chipnum], ofs, datum);

		if (ret)
			return ret;

		(*retlen) += n;
	}

	return 0;
}

static int do_write_buffer(struct map_info *map, struct flchip *chip,
                           unsigned long adr, const u_char *buf, int len)
{
	struct cfi_private *cfi = map->fldrv_priv;
	cfi_word status, status_OK;
	unsigned long cmd_adr, timeo;
	int wbufsize, z;

	wbufsize = CFIDEV_INTERLEAVE << cfi->cfiq->MaxBufWriteSize;
	adr += chip->start;
	cmd_adr = adr & ~(wbufsize - 1);

	/* Let's determinc this according to the interleave only once */
	status_OK = CMD(0x80);

	timeo = cfi->cfiq->BufWriteTimeoutMax * 1000;

	ENABLE_VPP(map);
	cfi_write(map, CMD(0xe8), cmd_adr);

	for (;;) {
		status = cfi_read(map, cmd_adr);
		if ((status & status_OK) == status_OK)
			break;

		if (timeo < 0) {
			/* Argh. Not ready for write to buffer */
			cfi_write(map, CMD(0x70), cmd_adr);
			DISABLE_VPP(map);
			printk("Chip not ready for buffer write. Xstatus = 0x%llx, status = %llx\n", (__u64)status, (__u64)cfi_read(map, cmd_adr));
			/* Odd. Clear status bits */
			cfi_write(map, CMD(0x50), cmd_adr);
			cfi_write(map, CMD(0x70), cmd_adr);
			cfi_write(map, CMD(0xff), adr);
			return -EIO;
		}
	}

	/* Write length of data to come */
	cfi_write(map, CMD(len/CFIDEV_BUSWIDTH-1), cmd_adr);

	/* Write data */
	for (z = 0; z < len; z += CFIDEV_BUSWIDTH) {
		if (cfi_buswidth_is_1()) {
			map->write8(map, *((__u8*)buf)++, adr+z);
		} else if (cfi_buswidth_is_2()) {
			map->write16(map, *((__u16*)buf)++, adr+z);
		} else if (cfi_buswidth_is_4()) {
			map->write32(map, *((__u32*)buf)++, adr+z);
		} else if (cfi_buswidth_is_8()) {
			map->write64(map, *((__u64*)buf)++, adr+z);
		} else {
			DISABLE_VPP(map);
			cfi_write(map, CMD(0xff), adr);
			return -EINVAL;
		}
	}
	/* GO GO GO */
	cfi_write(map, CMD(0xd0), cmd_adr);

	timeo = cfi->cfiq->BufWriteTimeoutMax * 1000;

	for (;;) {
		status = cfi_read(map, cmd_adr);
		if ((status & status_OK) == status_OK)
			break;

		if (timeo-- < 0) {
			printk("Waiting for chip to be ready timed out in bufwrite\n");
			cfi_write(map, CMD(0xff), adr);
			return -EIO;
		}
	}

	/* Done and happy. */
	DISABLE_VPP(map);
	/* check for lock bit */
	if (status & CMD(0x02)) {
		/* clear status */
		cfi_write(map, CMD(0x50), cmd_adr);
		/* put back into read status register mode */
		cfi_write(map, CMD(0x70), adr);
		cfi_write(map, CMD(0xff), adr);
		return -EROFS;
	}

	cfi_write(map, CMD(0xff), adr);
	return 0;
}

static int cfi_intelext_write_buffers(struct mtd_info *mtd, loff_t to,
                                      size_t len, size_t *retlen, const u_char *buf)
{
	struct map_info *map = mtd->priv;
	struct cfi_private *cfi = map->fldrv_priv;
	int wbufsize = CFIDEV_INTERLEAVE << cfi->cfiq->MaxBufWriteSize;
	int ret = 0;
	int chipnum;
	unsigned long ofs;

	*retlen = 0;
	if (!len) return 0;

	chipnum = to >> cfi->chipshift;
	ofs = to - (chipnum << cfi->chipshift);

#ifdef CONFIG_MSG_PROGRESS
	progress_bar(0, 100);
#endif
	/* If it's not bus-aligned, do the first word write */
	if (ofs & (CFIDEV_BUSWIDTH-1)) {
		size_t local_len = (-ofs)&(CFIDEV_BUSWIDTH-1);
		if (local_len > len)
			local_len = len;
		ret = cfi_intelext_write_words(mtd, to, local_len, retlen, buf);
		if (ret) return ret;
		ofs += local_len;
		buf += local_len;
		len -= local_len;

		if (ofs >> cfi->chipshift) {
			chipnum++;
			ofs = 0;
			if (chipnum == cfi->numchips)
				return 0;
		}
	}

	/* Write buffer is worth it only if more than one word to write... */
	while (len > CFIDEV_BUSWIDTH) {
		/* We must not cross write block boundaries */
		int size = wbufsize - (ofs & (wbufsize-1));

		if (size > len )
			size = len & ~(CFIDEV_BUSWIDTH - 1);
		ret = do_write_buffer(map, &cfi->chips[chipnum], ofs, buf, size);
		if (ret) return ret;

		ofs += size;
		buf += size;
		(*retlen) += size;
		len -= size;

		if (ofs >> cfi->chipshift) {
			chipnum++;
			ofs = 0;
			if (chipnum == cfi->numchips)
				return 0;
		}
#ifdef CONFIG_MSG_PROGRESS
		if (!(*retlen % 0x20000))
			progress_bar(*retlen, len + *retlen);
#endif
	}

	/* ... and write the remaining bytes */
	if (len > 0) {
		size_t local_retlen;
		ret = cfi_intelext_write_words(mtd, ofs + (chipnum << cfi->chipshift),
                                               len, &local_retlen, buf);
		
		if (ret) return ret;
		(*retlen) += local_retlen;
	}
#ifdef CONFIG_MSG_PROGRESS
	progress_bar(100, 100);
#endif

	return 0;
}

typedef int (*varsize_frob_t)(struct map_info *map, struct flchip *chip,
                              unsigned long adr, void *thunk);

static int cfi_intelext_varsize_frob(struct mtd_info *mtd, varsize_frob_t frob,
                                     loff_t ofs, size_t len, void *thunk)
{
        struct map_info *map = mtd->priv;
        struct cfi_private *cfi = map->fldrv_priv;
        unsigned long adr;
        int chipnum, ret = 0;
        int i, first;
        struct mtd_erase_region_info *regions = mtd->eraseregions;

        if (ofs > mtd->size)
                return -EINVAL;

        if ((len + ofs) > mtd->size)
                return -EINVAL;

        /* Check that both start and end of the requested erase are
         * aligned with the erasesize at the appropriate addresses.
         */

        i = 0;

        /* Skip all erase regions which are ended before the start of 
           the requested erase. Actually, to save on the calculations,
           we skip to the first erase region which starts after the
           start of the requested erase, and then go back one.
        */

        while (i < mtd->numeraseregions && ofs >= regions[i].offset)
               i++;
        i--;

        /* OK, now i is pointing at the erase region in which this 
           erase request starts. Check the start of the requested
           erase range is aligned with the erase size which is in
           effect here.
        */

        if (ofs & (regions[i].erasesize-1))
                return -EINVAL;

        /* Remember the erase region we start on */
        first = i;

        /* Next, check that the end of the requested erase is aligned
         * with the erase region at that address.
         */

        while (i<mtd->numeraseregions && (ofs + len) >= regions[i].offset)
                i++;

        /* As before, drop back one to point at the region in which
           the address actually falls
        */
        i--;

        if ((ofs + len) & (regions[i].erasesize-1))
                return -EINVAL;

        chipnum = ofs >> cfi->chipshift;
        adr = ofs - (chipnum << cfi->chipshift);

        i=first;

#ifdef CONFIG_MSG_PROGRESS
	progress_bar(adr - ofs, (len + adr) - ofs);
#endif
        while(len) {
                ret = (*frob)(map, &cfi->chips[chipnum], adr, thunk);

                if (ret)
                        return ret;

                adr += regions[i].erasesize;
                len -= regions[i].erasesize;

                if (adr % (1<< cfi->chipshift) == ((regions[i].offset + (regions[i].erasesize * regions[i].numblocks)) %( 1<< cfi->chipshift)))
                        i++;

                if (adr >> cfi->chipshift) {
                        adr = 0;
                        chipnum++;

                        if (chipnum >= cfi->numchips)
                        break;
                }
#ifdef CONFIG_MSG_PROGRESS
	progress_bar(adr - ofs, (len + adr) - ofs);
#endif
        }
#ifdef CONFIG_MSG_PROGRESS
	progress_bar(100, 100);
#endif

        return 0;
}


static int do_erase_oneblock(struct map_info *map, struct flchip *chip, 
                             unsigned long adr, void *thunk)
{
	struct cfi_private *cfi = map->fldrv_priv;
	cfi_word status, status_OK;
	unsigned long timeo;
	int ret = 0;

	adr += chip->start;

	/* Let's determine this according to the interleave only once */
	status_OK = CMD(0x80);

	/* 왜 이 값이냐고요? 그냥. 난디 */
	timeo = 0x4000 * 1000;

	ENABLE_VPP(map);
	/* Clear the status register first */
	cfi_write(map, CMD(0x50), adr);

	/* Now erase */
	cfi_write(map, CMD(0x20), adr);
	cfi_write(map, CMD(0xD0), adr);
	
	while (timeo > 0) {
		status = cfi_read(map, adr);
		if ((status & status_OK) == status_OK)
			break;
		timeo--;
	}

	DISABLE_VPP(map);

	/* We've broken this before. It doesn't hurt to be safe */
	cfi_write(map, CMD(0x70), adr);
	status = cfi_read(map, adr);

	ret = 0;
	/* check for lock bit */
	if ((timeo <= 0) || (status & CMD(0x3a))) {
		unsigned char chipstatus = status;
		if (status != CMD(status & 0xff)) {
			int i;
			for (i = 1; i < CFIDEV_INTERLEAVE; i++) {
				chipstatus |= status >> (cfi->device_type * 8);
			}
			printk("Status is not identical for all chips: 0x%llx, Mergint to give 0x%02x\n", (__u64)status, chipstatus);
		}
		/* Reset the error bits */
		cfi_write(map, CMD(0x50), adr);
		cfi_write(map, CMD(0x70), adr);

		printk("\n");
		if ((chipstatus & 0x30) == 0x30) {
			printk("Chip reports improper command sequence: status 0x%llx\n", (__u64)status);
			ret = -EIO;
		} else if (chipstatus & 0x02) {
			/* Protection bit set */
			printk("Protection bit set: status 0x%llx\n", (__u64)status);
			ret = -EROFS;
		} else if (chipstatus & 0x8) {
			/* Voltage */
			printk("Chip reports voltage low on erase: status 0x%llx\n", (__u64)status);
			ret = -EIO;
		} else if (chipstatus & 0x20) {
			printk("Chip erase failed at 0x%08lx: status 0x%llx\n", adr, (__u64)status);
			ret = -EIO;
		}
	}

	cfi_write(map, CMD(0xff), adr);
	return ret;
}

int cfi_intelext_erase_varsize(struct mtd_info *mtd, struct erase_info *instr)
{
        unsigned long ofs, len;
        int ret;

        ofs = instr->addr;
        len = instr->len;

        ret = cfi_intelext_varsize_frob(mtd, do_erase_oneblock, ofs, len, 0);

        return ret;
}

#ifdef DEBUG_LOCK_BITS
static int do_printlockstatus_oneblock(struct map_info *map, struct flchip *chip, unsigned long adr, void *thunk)
{
        struct cfi_private *cfi = map->fldrv_priv;
        int ofs_factor = cfi->interleave * cfi->device_type;

        cfi_send_gen_cmd(0x90, 0x55, 0, map, cfi, cfi->device_type, NULL);
        printk("block status register for 0x%08lx is %x\n",
               adr, cfi_read_query(map, adr+(2*ofs_factor)));
        cfi_send_gen_cmd(0xff, 0x55, 0, map, cfi, cfi->device_type, NULL);

        return 0;
}
#endif

#define DO_XXLOCK_ONEBLOCK_LOCK         ((void *) 1)
#define DO_XXLOCK_ONEBLOCK_UNLOCK       ((void *) 2)

static int do_xxlock_oneblock(struct map_info *map, struct flchip *chip, unsigned long adr, void *thunk)
{
	struct cfi_private *cfi = map->fldrv_priv;
	cfi_word status, status_OK;
	unsigned long timeo = 0x10000;

	adr += chip->start;

	/* Let's determine this according to the interleave only once */
	status_OK = CMD(0x80);

	ENABLE_VPP(map);
	cfi_write(map, CMD(0x60), adr);

	if (thunk == DO_XXLOCK_ONEBLOCK_LOCK) {
		cfi_write(map, CMD(0x01), adr);
	} else if (thunk == DO_XXLOCK_ONEBLOCK_UNLOCK) {
		cfi_write(map, CMD(0xD0), adr);
	} else {
		/* bug */
	}


	for (;;) {
		status = cfi_read(map, adr);
		if ((status & status_OK) == status_OK)
			break;
		if (timeo-- < 0) {
			cfi_write(map, CMD(0x70), adr);
                        printk("waiting for unlock to complete timed out. Xstatus = %llx, status = %llx.\n", (__u64)status, (__u64)cfi_read(map, adr));
			cfi_write(map, CMD(0xff), adr);
			DISABLE_VPP(map);
			return -EIO;
		}
	}
	/* Done */
	DISABLE_VPP(map);
	cfi_write(map, CMD(0xff), adr);
	return 0;
}

static int cfi_intelext_lock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
        int ret;

#ifdef DEBUG_LOCK_BITS
        printk(__FUNCTION__
               ": lock status before, ofs=0x%08llx, len=0x%08X\n",
               ofs, len);
        cfi_intelext_varsize_frob(mtd, do_printlockstatus_oneblock,
                                  ofs, len, 0);
#endif

        ret = cfi_intelext_varsize_frob(mtd, do_xxlock_oneblock,
                                        ofs, len, DO_XXLOCK_ONEBLOCK_LOCK);

#ifdef DEBUG_LOCK_BITS
        printk(__FUNCTION__
               ": lock status after, ret=%d\n", ret);
        cfi_intelext_varsize_frob(mtd, do_printlockstatus_oneblock,
                                  ofs, len, 0);
#endif

        return ret;
}

static int cfi_intelext_unlock(struct mtd_info *mtd, loff_t ofs, size_t len)
{
        int ret;

#ifdef DEBUG_LOCK_BITS
        printk(__FUNCTION__
               ": lock status before, ofs=0x%08llx, len=0x%08X\n",
               ofs, len);
        cfi_intelext_varsize_frob(mtd, do_printlockstatus_oneblock,
                                  ofs, len, 0);
#endif

        ret = cfi_intelext_varsize_frob(mtd, do_xxlock_oneblock,
                                        ofs, len, DO_XXLOCK_ONEBLOCK_UNLOCK);

#ifdef DEBUG_LOCK_BITS
        printk(__FUNCTION__
               ": lock status after, ret=%d\n", ret);
        cfi_intelext_varsize_frob(mtd, do_printlockstatus_oneblock,
                                  ofs, len, 0);
#endif

        return ret;
}
