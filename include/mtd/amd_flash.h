/*  $Id: amd_flash.h,v 1.3 2002/10/08 01:46:33 nandy Exp $ */
#ifndef _VIVI_AMDFLASH_H_
#define _VIVI_AMDFLASH_H_


/* There's no limit. It exists only to avoid realloc. */
#define MAX_AMD_CHIPS 8

#define DEVICE_TYPE_X8  (8 / 8)
#define DEVICE_TYPE_X16 (16 / 8)
#define DEVICE_TYPE_X32 (32 / 8)

/* Addresses */
#define ADDR_MANUFACTURER               0x0000
#define ADDR_DEVICE_ID                  0x0001
#define ADDR_SECTOR_LOCK                0x0002
#define ADDR_HANDSHAKE                  0x0003
#define ADDR_UNLOCK_1                   0x0555
#define ADDR_UNLOCK_2                   0x02AA

/* Commands */
#define CMD_UNLOCK_DATA_1               0x00AA
#define CMD_UNLOCK_DATA_2               0x0055
#define CMD_MANUFACTURER_UNLOCK_DATA    0x0090
#define CMD_UNLOCK_BYPASS_MODE          0x0020
#define CMD_PROGRAM_UNLOCK_DATA         0x00A0
#define CMD_RESET_DATA                  0x00F0
#define CMD_SECTOR_ERASE_UNLOCK_DATA    0x0080
#define CMD_SECTOR_ERASE_UNLOCK_DATA_2  0x0030

#define CMD_UNLOCK_SECTOR               0x0060

/* Manufacturers */
#define MANUFACTURER_AMD        0x0001
#define MANUFACTURER_ATMEL      0x001F
#define MANUFACTURER_FUJITSU    0x0004
#define MANUFACTURER_ST         0x0020
#define MANUFACTURER_SST        0x00BF
#define MANUFACTURER_TOSHIBA    0x0098
#define MANUFACTURER_HYNIX	0x00AD

/* AMD */
#define AM29F800BB      0x2258
#define AM29F800BT      0x22D6
#define AM29LV800BB     0x225B
#define AM29LV800BT     0x22DA
#define AM29LV160DT     0x22C4
#define AM29LV160DB     0x2249
#define AM29BDS323D     0x22D1
#define AM29BDS643D     0x227E

/* Atmel */
#define AT49xV16x       0x00C0
#define AT49xV16xT      0x00C2

/* Fujitsu */
#define MBM29LV160TE    0x22C4
#define MBM29LV160BE    0x2249

/* ST - www.st.com */
#define M29W800T        0x00D7
#define M29W160DT       0x22C4
#define M29W160DB       0x2249

/* SST */
#define SST39LF800      0x2781
#define SST39LF160      0x2782

/* SST */
#define SST39LF800      0x2781
#define SST39LF160      0x2782

/* Toshiba */
#define TC58FVT160      0x00C2
#define TC58FVB160      0x0043

/* Hynix */
#define HY29LV800B	0x225B
#define HY29LV800T	0x22DA

#define D6_MASK 0x40

#define TIMEO		0x100000        /* Just do it */

struct amd_flash_private {
        int device_type;
        int interleave;
        int numchips;
        unsigned long chipshift;
        /* const char *im_name; */
        struct flchip chips[0];
};

struct amd_flash_info {
        const __u16 mfr_id;
        const __u16 dev_id;
        const char *name;
        const u_long size;
        const int numeraseregions;
        const struct mtd_erase_region_info regions[4];
};

static int amd_flash_write(struct mtd_info *, loff_t, size_t, size_t *,
                           const u_char *);
static int amd_flash_erase(struct mtd_info *, struct erase_info *);
struct mtd_info *amd_flash_probe(struct map_info *map);


#endif /* _VIVI_AMDFLASH_H_ */
