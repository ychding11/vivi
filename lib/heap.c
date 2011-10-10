/*
 * vivi/lib/heap.c: simple dynamic memory allocation routine
 *
 * Based on bootldr/heap.c
 *
 */

#include "config.h"
#include "machine.h"
#include "printk.h"

#ifdef DEBUG_HEAP
#define DPRINTK(args...)	printk(##args)
#else
#define DPRINTK(args...)
#endif

#define BLOCKHEAD_SIGNATURE 0x0000F00D

typedef long Int32;
typedef char Bool;
#define NULL 0
#define TRUE (1 == 1)
#define FALSE (0 == 1)

typedef struct blockhead_t {
	Int32 signature;
	Bool allocated;
	unsigned long size;
	struct blockhead_t *next;
	struct blockhead_t *prev;
} blockhead;

static blockhead *gHeapBase = NULL;

static inline int mmalloc_init(unsigned char *heap, unsigned long size)
{
	if (gHeapBase != NULL) return -1;

	DPRINTK("malloc_init(): initialize heap area at 0x%08lx, size = 0x%08lx\n",
		heap, size);

	gHeapBase = (blockhead *)(heap);
	gHeapBase->allocated=FALSE;
	gHeapBase->signature=BLOCKHEAD_SIGNATURE;
	gHeapBase->next=NULL;
	gHeapBase->prev=NULL;
	gHeapBase->size = size - sizeof(blockhead);

	return 0;
}

int heap_init(void)
{
	return mmalloc_init((unsigned char *)(HEAP_BASE), HEAP_SIZE);	
}

static int compact_heap(void) {
	// return non-zero if heap was compacted
	return 0;
}

void *mmalloc(unsigned long size) 
{
	blockhead *blockptr = gHeapBase;
	blockhead *newblock;
	Bool compacted = FALSE;

	size = (size+7)&~7; /* unsigned long align the size */
	DPRINTK("malloc(): size = 0x%08lx\n", size);

	while (blockptr != NULL) {
		if (blockptr->allocated == FALSE) {
			if (blockptr->size >= size) {
				blockptr->allocated=TRUE;
				if ((blockptr->size - size) > sizeof(blockhead)) {
					newblock = (blockhead *)((unsigned char *)(blockptr) + sizeof(blockhead) + size);
					newblock->signature = BLOCKHEAD_SIGNATURE;
					newblock->prev = blockptr;
					newblock->next = blockptr->next;
					newblock->size = blockptr->size - size - sizeof(blockhead);
					newblock->allocated = FALSE;
					blockptr->next = newblock;
					blockptr->size = size;
				} else {
				}
				break;
			} else {
				if ((blockptr->next == NULL) && (compacted == FALSE)) {
					if (compact_heap()) {
						compacted=TRUE;
						blockptr = gHeapBase;
						continue;
					}
				}
			}
		}
		blockptr = blockptr->next;
	}
	DPRINTK("malloc(): returning blockptr = 0x%08lx\n", blockptr);

	if (blockptr == NULL)
		printk("Error: malloc(), out of storage. size = 0x%x\n", size);

	return (blockptr != NULL) ? ((unsigned char *)(blockptr)+sizeof(blockhead)) : NULL;
}

void mfree(void *block) {
	blockhead *blockptr;

	if (block == NULL) return;

	blockptr = (blockhead *)((unsigned char *)(block) - sizeof(blockhead));

	if (blockptr->signature != BLOCKHEAD_SIGNATURE) return;

	blockptr->allocated=FALSE;
	return;
}
