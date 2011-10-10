/*
 * vivi/drivers/mtd/chipreg.c: Registration for chip drivers
 *
 * Based on linux/drivers/chipreg.c
 *
 * $Id: chipreg.c,v 1.3 2002/08/29 06:10:23 nandy Exp $
 *
 */

#include <config.h>
#include <mtd/map.h>

extern struct mtd_info *cfi_probe(struct map_info *map);
extern struct mtd_info *amd_flash_probe(struct map_info *map);

static struct mtd_chip_driver chip_drivers[] = {
#ifdef CONFIG_MTD_CFI
	{ cfi_probe, 	"cfi_probe" },  
#endif
#ifdef CONFIG_MTD_AMDSTD
	{ amd_flash_probe, "amd_flash" },
#endif
	{ NULL, 	NULL }
};

static struct mtd_chip_driver *get_mtd_chip_driver (const char *name)
{
	struct mtd_chip_driver *drv = chip_drivers;

	while (drv->name != NULL) {
		if (!strcmp(name, drv->name)) 
			return drv;
		drv++;
	}
	return NULL;
}

struct mtd_info *do_map_probe(const char *name, struct map_info *map)
{
	struct mtd_chip_driver *drv;
	struct mtd_info *ret;

	drv = get_mtd_chip_driver(name);

	if (!drv) return NULL;
	ret = drv->probe(map);
	if (ret) return ret;
	return NULL;
}
