#ifndef _VIVI_BONFS_H_
#define _VIVI_BONFS_H_

typedef struct {
	ulong offset;
	ulong size;
	ulong flag;
	ulong num_bad_block;
	unsigned short *bad_blocks;
} bon_partition_t;

int write_bon_image(struct mtd_info *mtd, ulong offset, char *src, long size);
int read_bon_partition(struct mtd_info *mtd);

#endif /* _VIVI_BONFS_H_ */
