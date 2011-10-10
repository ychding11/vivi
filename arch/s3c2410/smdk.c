/*
 * vivi/arch/s3c2410/smdk.c:
 *    S3C2410 개발 보드에 관련된 것들.
 *
 *  This code is GPL.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/10/22 08:10:39 $
 *
 * $Revision: 1.9 $
 * 
 */

#include "config.h"
#include "machine.h"
#include "vivi.h"
#include "priv_data.h"
#include "mtd/map.h"
#include "boot_kernel.h"
#include "command.h"
#include "time.h"

#ifdef CONFIG_S3C2410_NAND_BOOT
mtd_partition_t default_mtd_partitions[] = {
	{
		name:		"vivi",
		offset:		0,
		size:		0x00020000,
		flag:		0
	}, {
		name:		"param",
		offset:		0x00020000,
		size:		0x00010000,
		flag:		0
	}, {
		name:		"kernel",
		offset:		0x00030000,
		//by threewater
		size:		0x000D0000,
		//size:		0x000C0000,
		flag:		0
	}, {
		name:		"root",
		offset:		0x00100000,
		size:		0x00200000,
		flag:		MF_BONFS
	}, {
		name:		"jffs2",
		offset:		0x00300000,
		size:		0x03D00000,
		flag:		MF_JFFS2
	}
};
#endif
#ifdef CONFIG_S3C2410_AMD_BOOT
mtd_partition_t default_mtd_partitions[] = {
	{
		name:		"vivi",
		offset:		0,
		size:		0x00020000,
		flag:		0
	}, {
		name:		"param",
		offset:		0x00020000,
		size:		0x00010000,
		flag:		0
	}, {
		name:		"kernel",
		offset:		0x00030000,
		size:		0x000C0000,
		flag:		0
	}, {
		name:		"root",
		offset:		0x00100000,
		size:		0x00140000,
		flag:		MF_BONFS
	}
};
#endif
int default_nb_part = ARRAY_SIZE(default_mtd_partitions);

#ifdef CONFIG_S3C2410_NAND_BOOT
#define MT_S3C2410	MT_SMC_S3C2410
#endif
#ifdef CONFIG_S3C2410_AMD_BOOT
#define MT_S3C2410	MT_NOR_FLASH
#endif
vivi_parameter_t default_vivi_parameters[] = {
	{ "mach_type",			MACH_TYPE,	NULL },
	{ "media_type",			MT_S3C2410,	NULL },
	{ "boot_mem_base",		0x30000000,	NULL },
	{ "baudrate",			UART_BAUD_RATE,	NULL },
	{ "xmodem_one_nak",		0,		NULL },
	{ "xmodem_initial_timeout",	300000,		NULL },
	{ "xmodem_timeout",		1000000,	NULL },
	{ "ymodem_initial_timeout",	1500000,	NULL },
	{ "boot_delay",			0x1000000,	NULL }
};
int default_nb_params = ARRAY_SIZE(default_vivi_parameters);

//by threewater
char linux_cmd[] = "noinitrd root=/dev/bon/3 init=/linuxrc console=ttyS0";
//char linux_cmd[] = "noinitrd root=/dev/bon/2 init=/linuxrc console=ttyS0";

void set_vpp(struct map_info *map, int vpp)
{
}

void set_gpios(void)
{
	GPACON  = vGPACON;
	GPBCON  = vGPBCON;
	GPBUP   = vGPBUP; 
	GPCCON  = vGPCCON;
	GPCUP   = vGPCUP; 
	GPDCON  = vGPDCON;
	GPDUP   = vGPDUP; 
	GPECON  = vGPECON;
	GPEUP   = vGPEUP; 
	GPFCON  = vGPFCON;
	GPFUP   = vGPFUP; 
	GPGCON  = vGPGCON;
	GPGUP   = vGPGUP; 
	GPGDAT |= (1<<12);
	GPHCON  = vGPHCON;
	GPHUP   = vGPHUP; 
	EXTINT0 = vEXTINT0;
	EXTINT1 = vEXTINT1;
	EXTINT2 = vEXTINT2;
}

int board_init(void)
{
	init_time();
	set_gpios();

	return 0;
}

extern user_command_t cpu_cmd;
int misc(void)
{
	add_command(&cpu_cmd);
	return 0;
}
