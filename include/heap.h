/*
 * vivi/include/heap.h
 *
 * Based on bootldr/heap.h
 *
 */
#ifndef _VIVI_HEAP_H_
#define _VIVI_HEAP_H_

#include <types.h>

int heap_init(void);
void *mmalloc(__u32 size);
void mfree(void *block);

#endif /* _VIVI_HEAP_H_ */
