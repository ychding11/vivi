#include <printk.h>

#define NFCONF ((volatile unsigned char *)0x4e000000)

#define CMD 4
#define ADDR 8
#define DATA 12
#define ECC0 20
#define ECC1 21
#define ECC2 22

#define BUSY 1

static void
wait_idle(void)
{
    int i;
	volatile unsigned char *ptr = NFCONF + 16;

    while(!(*ptr & BUSY)) for(i=0; i<10; i++);
}

static void
read_one_page(unsigned int *buf)
{
    volatile unsigned char *ptr = NFCONF + DATA;
    int i;
    for(i=0;i<512/4;i++) {
	unsigned int val = *ptr;
	val |= *ptr << 8;
	val |= *ptr << 16;
	val |= *ptr << 24;
	*buf++ = val;
    }
}

static int
nand_read_page(char *buf, int page)
{
    volatile unsigned char *nfconf = NFCONF;
    volatile unsigned int *nfconf4 = (unsigned int *)NFCONF;
    int i;

    *nfconf4 &= ~0x800;   /* clear nFCE */
	for(i=0; i<10; i++);

    nfconf[CMD] = 0; /* READ0 */

    /* Write Address */
    nfconf[ADDR] = 0; 
    nfconf[ADDR] = page & 0xff;
    nfconf[ADDR] = (page >> 8) & 0xff;
    nfconf[ADDR] = (page >> 16) & 0xff;

    wait_idle();
    
    read_one_page((unsigned int *)buf);

    *nfconf4 |= 0x800; /* chip disable */

	return 0;
}

#define NAND_PAGES_PER_BLOCK	16 // 8M
#define NAND_BLOCK_SIZE (NAND_PAGES_PER_BLOCK*512)
#define NAND_BLOCK_MASK (NAND_BLOCK_SIZE - 1)

/* low level nand read function */
int
nand_read_ll(char *buf, unsigned long start_addr, int size)
{
    unsigned long page;
    unsigned long start_block;
    int num_bl;
    int i, j;

    if ((start_addr & NAND_BLOCK_MASK) || (size & NAND_BLOCK_MASK)) {
	/* invalid alignment */
	return -1;
    }

    start_block = start_addr / NAND_BLOCK_SIZE;
    num_bl = size / NAND_BLOCK_SIZE;

    for(i=0;i<num_bl;i++) {
	page = (start_block + i) * NAND_PAGES_PER_BLOCK;
	for(j=0;j<NAND_PAGES_PER_BLOCK;j++) {
	    int err = nand_read_page(buf, page);
	    if (err) {
		if (j == 0) break; /* bad block */
		return -2; /* unrecoverable error */
	    }
	    buf += 512;
	    page++;
	}
    }
    return 0;
}

int main()
{
	char buf[128<<10]; // 128K
	char *buf_p = (char*)&buf[0];
	int ret;

	printk("buf_p = 0x%p\n", buf_p);

	ret = nand_read_ll(buf_p, 0, sizeof(buf));

	switch (ret) {
		case -1 :
			printk("invalid alignment\n");
		break;
		case -2 :
			printk("unrecoverable error\n");
		break;
		case 0 :
			printk("ok! read %d bytes\n", sizeof(buf));
		break;
		default :
			printk("unknown error\n");
			return 0;
	}

	printk("use the 'dump' command, start_addr = 0x%p\n", buf_p);

	return 0;
}
