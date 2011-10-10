/*
 * vivi/arch/s3c2410/mmu.c: Simple memory mapping
 *
 * Copyriget (C) 2001,2002 MIZI Research, Inc.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/11/30 20:57:03 $
 *
 * $Revision: 1.6 $
 *
 * Note:
 *   - mmu_init()에서는 putstr()과 같은 출력문을 사용하지 마세요.
 *
 *
 * History
 *
 * 2002-05-15: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 * 
 * 2002-07-15: Janghoon Lyu <nandy@mizi.com>
 *    - 왕창 새로 고침
 */

#include "config.h"
#include "machine.h"
#include "mmu.h"
#include "vivi_string.h"

static unsigned long *mmu_tlb_base = (unsigned long *) MMU_TABLE_BASE;

/*
 * cpu_arm920_cache_clean_invalidate_all()
 *
 * clean and invalidate all cache lines
 *
 */
static inline void cpu_arm920_cache_clean_invalidate_all(void)
{
__asm__(
	"	mov	r1, #0\n"
	"	mov	r1, #7 << 5\n"		  /* 8 segments */
	"1:	orr	r3, r1, #63 << 26\n"	  /* 64 entries */
	"2:	mcr	p15, 0, r3, c7, c14, 2\n" /* clean & invalidate D index */
	"	subs	r3, r3, #1 << 26\n"
	"	bcs	2b\n"			  /* entries 64 to 0 */
	"	subs	r1, r1, #1 << 5\n"
	"	bcs	1b\n"			  /* segments 7 to 0 */
	"	mcr	p15, 0, r1, c7, c5, 0\n"  /* invalidate I cache */
	"	mcr	p15, 0, r1, c7, c10, 4\n" /* drain WB */
	);
}

void cache_clean_invalidate(void)
{
	cpu_arm920_cache_clean_invalidate_all();
}

/*
 * cpu_arm920_tlb_invalidate_all()
 *
 * Invalidate all TLB entries
 */
static inline void cpu_arm920_tlb_invalidate_all(void)
{
	__asm__(
		"mov	r0, #0\n"
		"mcr	p15, 0, r0, c7, c10, 4\n"	/* drain WB */
		"mcr	p15, 0, r0, c8, c7, 0\n"	/* invalidate I & D TLBs */
		);
}

void tlb_invalidate(void)
{
	cpu_arm920_tlb_invalidate_all();
}

static inline void arm920_setup(void)
{
	unsigned long ttb = MMU_TABLE_BASE;

__asm__(
	/* Invalidate caches */
	"mov	r0, #0\n"
	"mcr	p15, 0, r0, c7, c7, 0\n"	/* invalidate I,D caches on v4 */
	"mcr	p15, 0, r0, c7, c10, 4\n"	/* drain write buffer on v4 */
	"mcr	p15, 0, r0, c8, c7, 0\n"	/* invalidate I,D TLBs on v4 */
	/* Load page table pointer */
	"mov	r4, %0\n"
	"mcr	p15, 0, r4, c2, c0, 0\n"	/* load page table pointer */
	/* Write domain id (cp15_r3) */
	"mvn	r0, #0\n"			/* Domains 0, 1 = client */
	"mcr	p15, 0, r0, c3, c0, 0\n"	/* load domain access register */
	/* Set control register v4 */
	"mrc	p15, 0, r0, c1, c0, 0\n"	/* get control register v4 */
	/* Clear out 'unwanted' bits (then put them in if we need them) */
						/* .RVI ..RS B... .CAM */ 
	"bic	r0, r0, #0x3000\n"		/* ..11 .... .... .... */
	"bic	r0, r0, #0x0300\n"		/* .... ..11 .... .... */
	"bic	r0, r0, #0x0087\n"		/* .... .... 1... .111 */
	/* Turn on what we want */
	/* Fault checking enabled */
	"orr	r0, r0, #0x0002\n"		/* .... .... .... ..1. */
#ifdef CONFIG_CPU_D_CACHE_ON
	"orr	r0, r0, #0x0004\n"		/* .... .... .... .1.. */
#endif  
#ifdef CONFIG_CPU_I_CACHE_ON 
	"orr	r0, r0, #0x1000\n"		/* ...1 .... .... .... */
#endif  
	/* MMU enabled */
	"orr	r0, r0, #0x0001\n"		/* .... .... .... ...1 */
	"mcr	p15, 0, r0, c1, c0, 0\n"	/* write control register */
	: /* no outputs */
	: "r" (ttb) );
}

void mmu_init(void)
{
	arm920_setup();
}

#ifndef CONFIG_S3C2410_NAND_BOOT
static void copy_vivi_to_ram(void)
{
	putstr_hex("Evacuating 1MB of Flash to DRAM at 0x", VIVI_RAM_BASE);
	memcpy((void *)VIVI_RAM_BASE, (void *)VIVI_ROM_BASE, VIVI_RAM_SIZE);
}
#endif

static inline void mem_mapping_linear(void)
{
	unsigned long pageoffset, sectionNumber;

	putstr_hex("MMU table base address = 0x", (unsigned long)mmu_tlb_base);
	/* 4G 영역을 1:1로 매핑. not cacacheable, not bufferable */
	for (sectionNumber = 0; sectionNumber < 4096; sectionNumber++) {
		pageoffset = (sectionNumber << 20);
		*(mmu_tlb_base + (pageoffset >> 20)) = pageoffset | MMU_SECDESC;
	}

	/* make dram cacheable */
	for (pageoffset = DRAM_BASE; pageoffset < (DRAM_BASE+DRAM_SIZE); pageoffset += SZ_1M) {
		//DPRINTK(3, "Make DRAM section cacheable: 0x%08lx\n", pageoffset);
		*(mmu_tlb_base + (pageoffset >> 20)) = pageoffset | MMU_SECDESC | MMU_CACHEABLE; 
	}
}

static inline void nor_flash_mapping(void)
{
	unsigned long offset, cached_addr, uncached_addr;

	cached_addr = FLASH_BASE;
	uncached_addr = FLASH_UNCACHED_BASE;

	for (offset = 0; offset < FLASH_SIZE; offset += MMU_SECTION_SIZE) { 
		cached_addr += offset;
		uncached_addr += offset;
		*(mmu_tlb_base + (cached_addr >> 20)) = \
				(cached_addr | MMU_SECDESC | MMU_CACHEABLE);
		*(mmu_tlb_base + (uncached_addr >> 20)) = \
				(cached_addr | MMU_SECDESC);
	}
}

/*
 * PC값의 명시적인 변화 없이 부트로더가 램에서 실행되도록
 * Flash의 부트로더 영역을 RAM의 부트로더 영역으로 매핑시킵니다.
 */
static inline void nor_flash_remapping(void)
{
	putstr_hex("Map flash virtual section to DRAM at 0x", VIVI_RAM_BASE);
	*(mmu_tlb_base + (VIVI_ROM_BASE >> 20)) = \
				(VIVI_RAM_BASE | MMU_SECDESC | MMU_CACHEABLE);
}

void mem_map_nand_boot(void)
{
	mem_mapping_linear();
}

#ifndef CONFIG_S3C2410_NAND_BOOT
void mem_map_nor(void)
{
	copy_vivi_to_ram();
	mem_mapping_linear();
	nor_flash_mapping();
	nor_flash_remapping();
}
#endif

void mem_map_init(void)
{
#ifdef CONFIG_S3C2410_NAND_BOOT
	mem_map_nand_boot();
#else
	mem_map_nor();
#endif
	cache_clean_invalidate();
	tlb_invalidate();
}
