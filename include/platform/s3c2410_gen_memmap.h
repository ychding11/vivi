/* 
 * vivi/include/sa1100_gen_memmap.h: generic memory map for SA1100
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/07/19 08:02:35 $
 *
 * $Revision: 1.1.1.1 $
 *
 *
 * Specification of the machine
 *   - CPU: SA-1110
 *   - MEM: 32MB (bank 0)
 *   - ROM: 32MB (Inte Starata Flash Memory)
 *   - got UART ports
 *
 *
 *  RAM (DRAM)
 *
 *  0xC200 0000 +------------------------------------+
 *              |   boot loader (size: 1M)           |
 *  0xC1F0 0000 +------------------------------------+
 *              |   heap area (size: 1M)             |
 *  0xC1E0 0000 +------------------------------------+
 *              |   mmu table (size: 16k)            |
 *  0xC1DF C000 +------------------------------------+ -----\
 *              |   linux command line  (size: 16k)  |       |
 *  0xC1DF 8000 +------------------------------------+       |
 *              |   vivi parameter table (size: 16k) |   vivi private data
 *  0xC1DF 4000 +------------------------------------+       |
 *              |   mtd partition table (size: 16k)  |       |
 *  0xC1DF 0000 +------------------------------------+ -----/
 *              |   stack area (size: 32k)           |
 *  0xC1DE 8000 +------------------------------------+
 *              |                                    |
 *              |                                    |
 *              |   free memory                      |
 *              |                                    |
 *              |                                    |
 *  0xC000 0000 +------------------------------------+
 *
 *
 *
 *  ROM (NOR Flash)
 *
 *  0x0200 0000 +-----------------------------+
 *              |  (stored) vivi private data |
 *  0x01fc 0000 +-----------------------------+
 *              |                             |
 *              |   usr (size: 29.5M)         |
 *              |                             |
 *  0x0024 0000 +-----------------------------+
 *              |   root (size: 1289k)        |
 *  0x0010 0000 +-----------------------------+
 *              |   kernel (size: 728k)       |
 *  0x0004 0000 +-----------------------------+
 *              |   boot loader (size: 256k)  |
 *  0x0000 0000 +-----------------------------+
 *
 */

#include "sizes.h"

#define ROM_BASE0		0x00000000      /* base address of rom bank 0 */
#define ROM_BASE1		0x08000000      /* base address of rom bank 1 */
#define DRAM_BASE0		0x30000000      /* base address of dram bank 0 */
#define DRAM_BASE1		0x38000000	/* base address of dram bank 1 */

/* Global definitions */
#define FLASH_BASE		ROM_BASE0
#define FLASH_SIZE		SZ_32M
#define FLASH_UNCACHED_BASE	0x10000000	/* to mapping flash memory */
#define FLASH_BUSWIDTH		4

#define DRAM_BASE		DRAM_BASE0
#define DRAM_SIZE		SZ_64M

#define MTD_PART_SIZE		SZ_16K
#define MTD_PART_OFFSET		0x00000000
#define PARAMETER_TLB_SIZE	SZ_16K
#define PARAMETER_TLB_OFFSET	0x00004000
#define LINUX_CMD_SIZE		SZ_16K
#define LINUX_CMD_OFFSET	0x00008000
#define VIVI_PRIV_SIZE		(MTD_PART_SIZE + PARAMETER_TLB_SIZE + LINUX_CMD_SIZE)

/* ROM */
#define VIVI_ROM_BASE		0x00000000
#define VIVI_PRIV_ROM_BASE	0x01FC0000

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
