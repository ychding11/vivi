#include "config.h"
#include "s3c2410.h"

/*
 * Memory map
 */
#include "sizes.h"

/* Porocessor memory map */
#define ROM_BASE0		0x00000000      /* base address of rom bank 0 */
#define ROM_BASE1		0x08000000      /* base address of rom bank 1 */
#define DRAM_BASE0		0x30000000      /* base address of dram bank 0 */
#define DRAM_BASE1		0x38000000	/* base address of dram bank 1 */

#ifdef CONFIG_MTD_CFI
/* Flash */
#define FLASH_BASE		ROM_BASE0
#define FLASH_SIZE		SZ_32M
#define FLASH_UNCACHED_BASE	0x10000000	/* to mapping flash memory */
#define FLASH_BUSWIDTH		4
/* ROM */
#define VIVI_ROM_BASE		0x00000000
#define VIVI_PRIV_ROM_BASE	0x01FC0000
#endif /* CONFIG_MTD_CFI */
#ifdef CONFIG_S3C2410_NAND_BOOT
/* Flash */
#define FLASH_BASE		ROM_BASE0
#define FLASH_SIZE		SZ_16M
#define FLASH_UNCACHED_BASE	0x10000000	/* to mapping flash memory */
#define FLASH_BUSWIDTH		4
/* ROM */
#define VIVI_ROM_BASE		0x00000000
#define VIVI_PRIV_ROM_BASE	0x01FC0000
#endif /* CONFIG_S3C2410_NAND_BOOT */
#ifdef CONFIG_S3C2410_AMD_BOOT
/* Flash */
#define FLASH_BASE		ROM_BASE0
#define FLASH_SIZE		SZ_1M
#define FLASH_UNCACHED_BASE	0x10000000	/* to mapping flash memory */
#define FLASH_BUSWIDTH		2		/* 16-bit bus */
/* ROM */
#define VIVI_ROM_BASE		0x00000000
#define VIVI_PRIV_ROM_BASE	0x01FC0000
#endif /* CONFIG_S3C2410_AMD_BOOT */

#define DRAM_BASE		DRAM_BASE0
#define DRAM_SIZE		SZ_8M

#define MTD_PART_SIZE		SZ_16K
#define MTD_PART_OFFSET		0x00000000
#define PARAMETER_TLB_SIZE	SZ_16K
#define PARAMETER_TLB_OFFSET	0x00004000
#define LINUX_CMD_SIZE		SZ_16K
#define LINUX_CMD_OFFSET	0x00008000
#define VIVI_PRIV_SIZE		(MTD_PART_SIZE + PARAMETER_TLB_SIZE + LINUX_CMD_SIZE)


/* RAM */
#define VIVI_RAM_SIZE		SZ_1M
#define VIVI_RAM_BASE		(DRAM_BASE + DRAM_SIZE - VIVI_RAM_SIZE)
#define HEAP_SIZE		SZ_1M
#define HEAP_BASE		(VIVI_RAM_BASE - HEAP_SIZE)
#define MMU_TABLE_SIZE		SZ_16K
#define MMU_TABLE_BASE		(HEAP_BASE - MMU_TABLE_SIZE)
#define VIVI_PRIV_RAM_BASE	(MMU_TABLE_BASE - VIVI_PRIV_SIZE)
#define STACK_SIZE		SZ_32K
#define STACK_BASE		(VIVI_PRIV_RAM_BASE - STACK_SIZE)
#define RAM_SIZE		(STACK_BASE - DRAM_BASE)
#define RAM_BASE		DRAM_BASE

/*
 * Architecture magic and machine type
 */
#include "architecture.h"
#define MACH_TYPE		193	
#define ARCHITECTURE_MAGIC	((ARM_PLATFORM << 24) | (ARM_S3C2410_CPU << 16) | \
				  MACH_TYPE)

/*
 * 기타 등등
 */
#define UART_BAUD_RATE		115200

#define FIN			12000000

/*
 * CPU 레지스터 설정 값들
 */

/* CPU clcok */
/* 50.00 MHz */
#define MDIV_50			0x5c
#define PDIV_50			0x4
#define SDIV_50			0x2
/* 200.00 MHz */
//#define MDIV_200		0x5c
//#define PDIV_200		0x4
//#define SDIV_200		0x0
//MCLK=202.8MHz	by threewater
#define S3C2410MCLK		202800000
#define MDIV_200		0xa1
#define PDIV_200		0x3
#define SDIV_200		0x1

/* initial values for DRAM */
/*#define vBWSCON		0x22111110*/
#define vBWSCON			0x11111110	//bank6,7 16bit width
#define vBANKCON0		0x00000700
#define vBANKCON1		0x00000700
#define vBANKCON2		0x00000700
#define vBANKCON3		0x00000700
#define vBANKCON4		0x00000700
#define vBANKCON5		0x00000700
#define vBANKCON6		0x00018004	//column address number=8bit
#define vBANKCON7		0x00018004	//colunm address number=8bit
#define vREFRESH		0x008e0459
#define vBANKSIZE		0xb2
#define vMRSRB6			0x30
#define vMRSRB7			0x30

#define vLOCKTIME		0x00ffffff	/* It's a default value */
#define vCLKCON			0x0000fff8	/* It's a default value */
#define vCLKDIVN		0x3		/* FCLK:HCLK:PCLK = 1:2:4 */
#define vMPLLCON_50		((MDIV_50 << 12) | (PDIV_50 << 4) | (SDIV_50)) 
#define vMPLLCON_200		((MDIV_200 << 12) | (PDIV_200 << 4) | (SDIV_200)) 

/* initial values for serial */
#define vULCON			0x3	/* UART, no parity, one stop bit, 8 bits */
#define vUCON			0x245
#define vUFCON			0x0
#define vUMCON			0x0

/* inital values for GPIOs */
#define vGPACON			0x007fffff
#define vGPBCON			0x00044555
#define vGPBUP			0x000007ff
#define vGPCCON			0x00000000	//for input 0xaaaaaaaa
#define vGPCUP			0x00000000	//enable pull-up 0x0000ffff
#define vGPDCON			0x00000000      //for input 0xaaaaaaaa
#define vGPDUP			0x00000000      //enable pull-up 0x0000ffff
#define vGPECON			0x000156aa	//for iis and output 0xaaaaaaaa
#define vGPEUP			0x0000ffff
#define vGPFCON			0x0000aaaa	//for exint 0x000051aa
#define vGPFUP			0x0000000f	//pull-up[4-7] 0x000000ef
#define vGPGCON			0x05000000	//output for led[12:13] 0xfd95ffba
#define vGPGUP			0x0000cfff	//disable pull-up[12:13] 0x0000efff
#define vGPHCON			0x0016faaa
#define vGPHUP			0x000007ff

#define vEXTINT0		0x22222222
#define vEXTINT1		0x22222222
#define vEXTINT2		0x22222222
