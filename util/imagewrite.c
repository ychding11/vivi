/* Nand Flash Image Writing Utility */

/* (C) 2002, Mizi Research Inc.
   Author: Hwang, Chideok <hwang@mizi.co.kr>
*/


#define _GNU_SOURCE

#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include <asm/types.h>
#include <linux/mtd/mtd.h>

#define MAX_PART		9
#define MAX_RETRY		5
#define PARTITION_OFFSET	(~0)

#define IS_MTD	1
#define IS_BON  0

typedef struct {
    ulong offset;
    ulong size;
    ulong flag;
    ulong num_bad_block;
    unsigned short *bad_blocks;
} partition_t;


const char part_magic[8] = {'M', 0, 0, 'I', 0, 'Z', 'I', 0};

partition_t parts[MAX_PART];
int num_part;

/* sector size가 256인 2M이하 NAND device는 write 못함 */
#define NAND_SECTOR_SIZE	512
#define NAND_OOB_SIZE		16
int mtd_fd;
mtd_info_t meminfo;


int is_bad_block(ulong offset);

#include "ecc.c"

ulong
calc_ecc(char *buf, unsigned char *ecc)
{
    nand_calculate_ecc(buf, ecc);
    return 0;
}

int
write_oob(char *buf, ulong offset)
{
    int err;
    struct mtd_oob_buf oob;
    char oob_buf[NAND_OOB_SIZE];
    unsigned char ecc[3];


    oob.start = offset;
    oob.ptr = oob_buf;
    oob.length = NAND_OOB_SIZE;

    memset(oob_buf, 0xFF, NAND_OOB_SIZE);

    calc_ecc(buf, ecc);
    memcpy(oob_buf + 8, ecc, 3);

    calc_ecc(buf + 256, ecc);
    memcpy(oob_buf + 11, ecc, 3);

    err = ioctl(mtd_fd, MEMWRITEOOB, &oob);
    if (err) {
	return 1;
    }
    return 0;
}

int
write_block(ulong offset, char *buf)
{
    int i;

    if (is_bad_block(offset)) return 1;

    for(i=0; i < (meminfo.erasesize/NAND_SECTOR_SIZE); i++) {
	if (pwrite(mtd_fd, buf, NAND_SECTOR_SIZE, offset) != NAND_SECTOR_SIZE)
	  return 1;
	if (write_oob(buf, offset)) return 1;
	offset += NAND_SECTOR_SIZE;
	buf += NAND_SECTOR_SIZE;
    }
    return 0;
}

int
write_image(ulong offset, int fd, long size)
{
    int bad_block_nr = 0;
    /* NAND_SECTOR_SIZE*32 > meminfo.erasesize */
    char buf[NAND_SECTOR_SIZE*32];
    ulong block;
    unsigned short *bad;
    int i;
    printf("size = %ld\n", size);

    if (offset % meminfo.erasesize) {
	printf("bad alignment \n");
	return -1;
    }

    for(i=0;i<num_part;i++) {
	if (parts[i].offset >= offset) break;
    }
    if (i == num_part) i = num_part - 1;
    if (offset + size > parts[i].offset + parts[i].size) {
	printf("image is too big for this partition\n");
	return -1;
    }
    block = (offset - parts[i].offset) / meminfo.erasesize;
    bad = parts[i].bad_blocks;
    if (bad) {
	while(*bad++ <= block) block++;
    }
    offset = parts[i].offset + block * meminfo.erasesize;

    read(fd, buf, meminfo.erasesize);
    while(size > 0) {
	if (write_block(offset, buf) == 0) {
	    size -= meminfo.erasesize;
	    if (size > 0)
	      read(fd, buf, meminfo.erasesize);
	} else {
	    int k;
	    int block_nr = (offset - parts[i].offset)/ meminfo.erasesize;
	    for(k=0;k<parts[i].num_bad_block;k++) {
		if (block_nr == parts[i].bad_blocks[k]) break;
	    }
	    if (k == parts[i].num_bad_block) {
		printf("**** warning: new bad block in %d\n", block_nr);
		exit(1);
	    }
	    bad_block_nr++;
	}
	offset += meminfo.erasesize;
    }
    printf("bad_block = %d\n", bad_block_nr);
    return 0;
}

ulong
read_size(char *s)
{
    ulong size = 0;
    while(isdigit(*s))  {
	size = size * 10 + *s - '0';
	s++;
	}
    if (*s == 'M' || *s == 'm') size *= 1024*1024;
    else if (*s == 'K' || *s == 'k') size *= 1024;
    else if (*s) {
	printf("hmm bad size %s\n", s);
    }
    return size;
}

ulong
read_flag(char *s)
{
    ulong flag = 0;
    
    while ( *s && *s != ':' )
	s++;

    if (*s == 0) 
	return IS_BON;

    s++;

    if (*s == 'm' || *s == 'M')
	flag |= IS_MTD;

    return flag;
}

char *
parse_filename(char *filename, ulong *offset)
{
    char *delim;
    delim = strchr(filename, ':');
    if (delim == NULL) {
	*offset = 0;
    } else {
	*delim++ = 0;
	*offset = 0;
	while(isdigit(*delim)) {
	    *offset = *offset * 10 + *delim - '0';
		delim++;
	}
	if (*delim == 'M' || *delim == 'm') *offset *= 1024*1024;
	else if (*delim == 'K' || *delim == 'k') *offset *= 1024;
	else if (*delim) {
	    printf("hmm bad file offset %s\n", filename);
	}
    }
    return filename;
}


void
mark_bad(ulong offset)
{
    char oobbuf[NAND_OOB_SIZE];
    struct mtd_oob_buf oob;

    oob.start = offset;
    oob.length = NAND_OOB_SIZE;
    oob.ptr = oobbuf;
    memset(oobbuf, 0xff, NAND_OOB_SIZE);
    oobbuf[5] = 0;
    ioctl(mtd_fd, MEMWRITEOOB, &oob);
}

int
read_partition(void)
{
    int i, k;
    struct mtd_oob_buf oob;
    unsigned char oobbuf[NAND_OOB_SIZE];
    char buf[NAND_SECTOR_SIZE];
    unsigned int *s;
    ulong offset = PARTITION_OFFSET;
    int retry = MAX_RETRY;

    if (offset > meminfo.size - meminfo.erasesize) 
	offset = meminfo.size - meminfo.erasesize;

    while(retry-- > 0) {
	oob.start = offset;
	oob.length = NAND_OOB_SIZE;
	oob.ptr = oobbuf;
	if (ioctl(mtd_fd, MEMREADOOB, &oob) || oobbuf[5] != 0xFF) {
	    offset -= meminfo.erasesize;
	    continue;
	}
	if (pread(mtd_fd, buf, NAND_SECTOR_SIZE, offset) != NAND_SECTOR_SIZE) {
	    offset -= meminfo.erasesize;
	    continue;
	}
	if (strncmp(buf, part_magic, 8) != 0) {
	    return -1;
	}
	break;
    }

    if (retry <= 0) return -1;

    s = (unsigned int *)(buf+8);
    num_part = *s++;
    for(i=0;i<num_part;i++) {
	parts[i].offset = *s++;
	parts[i].size = *s++;
	parts[i].flag = *s++;
    }
    for(i=0;i<num_part;i++) {
	parts[i].num_bad_block = *s++;
	if (parts[i].num_bad_block) {
	    parts[i].bad_blocks = malloc(parts[i].num_bad_block * sizeof(unsigned int));
	    for(k=0;k<parts[i].num_bad_block;k++) {
		parts[i].bad_blocks[k] = *s++;
	    }
	} else {
		parts[i].bad_blocks = 0;
	}
    }
    return 0;
}


int
write_partition(ulong offset)
{
    unsigned char oobbuf[NAND_OOB_SIZE];
    char buf[NAND_SECTOR_SIZE];
    erase_info_t erase;
    unsigned int *s;
    struct mtd_oob_buf oob;
    int i, k;

    oob.start = offset;
    oob.length = NAND_OOB_SIZE;
    oob.ptr = oobbuf;

    if (ioctl(mtd_fd, MEMREADOOB, &oob) || oobbuf[5] != 0xFF) return -1;
    if (pread(mtd_fd, buf, NAND_SECTOR_SIZE, offset) != NAND_SECTOR_SIZE) {
	printf("read error: mark bad: offset = %lX\n", offset);
	mark_bad(offset);
	return -1;
    }


    erase.start = offset;
    erase.length = meminfo.erasesize;
    if (ioctl(mtd_fd, MEMERASE, &erase) < 0) {
	printf("erase error: mark bad: offset = %lX\n", offset);
	mark_bad(offset);
	return -1;
    }

    memcpy(buf, part_magic, 8);
    s = (unsigned int *)(buf+8);
    *s++ = num_part;
    for(i=0;i<num_part;i++) {
	*s++ = parts[i].offset;
	*s++ = parts[i].size;
	*s++ = parts[i].flag;
    }
    for(i=0;i<num_part;i++) {
	*s++ = parts[i].num_bad_block;
	for(k=0;k<parts[i].num_bad_block;k++) {
	    *s++ = parts[i].bad_blocks[k];
		printf("k = %d block = %d\n", k, parts[i].bad_blocks[k]);
	}
    }
    
    if (pwrite(mtd_fd, buf, NAND_SECTOR_SIZE, offset) != NAND_SECTOR_SIZE) {
	printf("write error: offset = %lu\n", offset);
	mark_bad(offset);
	return -1;
    }
    return 0;
}

int
is_bad_block(ulong offset)
{
    unsigned char oobbuf[NAND_OOB_SIZE];
    char buf[NAND_SECTOR_SIZE];
    struct mtd_oob_buf oob;
    erase_info_t erase;

    erase.start = offset;
    erase.length = meminfo.erasesize;

    if (ioctl(mtd_fd, MEMERASE, &erase) < 0) return 1;

    oob.start = offset;
    oob.length = NAND_OOB_SIZE;
    oob.ptr = oobbuf;
    if (ioctl(mtd_fd, MEMREADOOB, &oob) || oobbuf[5] != 0xFF) return -1;
    if (pread(mtd_fd, buf, NAND_SECTOR_SIZE, offset) != NAND_SECTOR_SIZE) {
      printf("read error\n");
      mark_bad(offset);
      return -1;
    }
    return 0;
}

void
check_bad_block(void)
{
    int i;
    unsigned short bad_block[1024];
    for(i=0;i<num_part;i++) {
	ulong offset = parts[i].offset;
	ulong end = (i + 1 < num_part) ? parts[i+1].offset : meminfo.size;
	int num_bad = 0;
	int bad;
	printf("part = %d end = %ld\n", i, end);
	while (offset < end) {
	    bad = is_bad_block(offset);
	    if (bad) {
		bad_block[num_bad] = (offset - parts[i].offset) / meminfo.erasesize;
		num_bad++;
		printf("%lX: is bad\n", offset);
	    }
	    offset += meminfo.erasesize;
	}
	parts[i].num_bad_block = num_bad;
#if 0 // ???? - bushi
	parts[i].flag = 0;
#endif
	if (num_bad) {
	    parts[i].bad_blocks = malloc(num_bad * sizeof(unsigned short));
	    memcpy(parts[i].bad_blocks, bad_block, num_bad * 2);
	}
	parts[i].size = end - parts[i].offset - num_bad * meminfo.erasesize;
    }
    parts[num_part - 1].size -= meminfo.erasesize;
}

int
write_partition_table(void)
{
    int i, k;
    ulong offset = PARTITION_OFFSET;
    int retry = MAX_RETRY;

    if (offset > meminfo.size - meminfo.erasesize)
        offset = meminfo.size - meminfo.erasesize;

    while(retry-- > 0) {
	if (write_partition(offset) == 0) break;
	offset -= meminfo.erasesize;
    }
    if (retry == 0) {
	printf("too many bad block in this flash\n");
	return -1;
    }
    for(i=0;i<num_part;i++) {
	printf("part%d:\n", i);
	printf("\toffset = %ld\n", parts[i].offset);
	printf("\tsize = %ld\n", parts[i].size);
	printf("\tbad_block = %ld\n", parts[i].num_bad_block);
	for(k=0;k<parts[i].num_bad_block;k++) {
	    printf(" %d\n", parts[i].bad_blocks[k]);
	}
    }
    return 0;
}

/* imagewrite -part 0 128K 1M; 
   imagewrite vivi:0
*/

int
main(int argc, char *argv[])
{
    int fd;
    char *fname;
    ulong offset, size;
    int i;

    mtd_fd = open(argv[1], O_RDWR);
    if (mtd_fd < 0) {
	printf("try to open %s:", argv[1]);
	perror("");
	return 1;
    }
    if (ioctl(mtd_fd, MEMGETINFO, &meminfo)) {
	perror("MEMGETINFO");
	return 1;
    }
    if ((meminfo.oobsize != NAND_OOB_SIZE) || 
	(meminfo.oobblock != NAND_SECTOR_SIZE)) {
	printf("hmm, oob size is ....\n");
	return 1;
    }
    printf("meminfo size = %d\n", meminfo.size);

    if (strcmp(argv[2], "-part") == 0) {
	int i;
	num_part = (argc - 3);
	printf("doing partition \n");
	for(i=0;i<num_part;i++) {
	    parts[i].offset = read_size(argv[3+i]);
	    printf("offset = %ld\n", parts[i].offset);
	    parts[i].flag = read_flag(argv[3+i]);
	    printf("flag = 0x%08lx\n", parts[i].flag);
	}
	printf("check bad block\n");
	check_bad_block();
	write_partition_table();
	return 0;
    }
    if (read_partition()) {
	printf("invalid partition table info\n");
	printf("first write partition!!!\n");
	return -1;
    }

    /* write image */
    for(i=2; i<argc;i++) {
	struct stat statbuf;
	fname = parse_filename(argv[i], &offset);
	fd = open(fname, O_RDONLY);
	if (fd < 0) {
	    printf("cannot open file(%s)\n", fname);
	    continue;
	}
	fstat(fd, &statbuf);
	size = statbuf.st_size;
	printf("size = %ld\n", size);
	write_image(offset, fd, size);
    }
    return 0;
}

