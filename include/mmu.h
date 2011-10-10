/*
 * History
 *
 * 2002-01-26: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 */

#ifndef _VIVI_MMU_H_
#define _VIVI_MMU_H_

/* 
 * MMU Level 1 Page Table Constants
 * Section descriptor 
 */ 
#define MMU_FULL_ACCESS		(3 << 10)	/* access permission bits */
#define MMU_DOMAIN		(0 << 5)	/* domain control bits */
#define MMU_SPECIAL		(1 << 4)	/* must be 1 */
#define MMU_CACHEABLE		(1 << 3)	/* cacheable */
#define MMU_BUFFERABLE		(1 << 2)	/* bufferable */
#define MMU_SECTION		(2) /* indicates that this is a section descriptor */
#define MMU_SECDESC		(MMU_FULL_ACCESS | MMU_DOMAIN | \
				 MMU_SPECIAL | MMU_SECTION)
#define MMU_SECTION_SIZE	0x00100000

void cache_clean_invalidate(void);
void tlb_invalidate(void);
void mmu_init(void);
void mem_map_init(void);

#endif /* _VIVI_MMU_H_ */
