/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 */

#include "vivi_string.h"
#include "printk.h"
#include <types.h>

#define FALSE			0
#define TRUE			1

#define TESTPAT1		0xAA55AA55
#define TESTPAT2		0x55AA55AA
#define TEST_PASSED		0
#define TEST_FAILED		1
#define MEGABYTE		(1024*1024)


typedef struct {
	volatile unsigned long pat1;
	volatile unsigned long pat2;
} pattern_addr_t;

typedef struct {
	volatile unsigned long addr;
} addrline_t;



static __inline unsigned long swap_32(unsigned long val)
{
	return(((val << 16) & 0xFFFF0000) | ((val >> 16) & 0x0000FFFF));
}

void write_error(int mode, unsigned long addr, 
                 unsigned long expected, unsigned long actual)
{
	if (!mode)
		printk("    ERROR at 0x%08lX: (exp: 0x%08lX act: 0x%08lX)\n", 
			addr, expected, actual);
}


/*
 * fills the memblock of <size> bytes from <startaddr> with pat1 and pat2
 */
void write_pattern(unsigned long startaddr, unsigned long size,
                    unsigned long pat1, unsigned long pat2)
{
	pattern_addr_t *p, *pe;

	p = (pattern_addr_t *)startaddr;
	pe = (pattern_addr_t *)(startaddr + size);

	while (p < pe) {
		p->pat1 = pat1;
		p->pat2 = pat2;
		p++;
	}
}

/*
 * checks the memblock of <size> bytes from <startaddr> with pat1 and pat2
 * returns the address of the first error or NULL if all is well
 */
void *check_pattern(int mode, unsigned long startaddr, unsigned long size,
                    unsigned long pat1, unsigned long pat2)
{
	pattern_addr_t *p, *pe;
	unsigned long actual1, actual2;

	p = (pattern_addr_t *)startaddr;
	pe = (pattern_addr_t *)(startaddr + size);

	while (p < pe) {
		actual1 = p->pat1;
		actual2 = p->pat2;

		if (actual1 != pat1) {
			write_error(mode, (unsigned long)&(p->pat1), pat1, actual1);
			return((void *)&(p->pat1));
		}
		if (actual2 != pat2) {
			write_error(mode, (unsigned long)&(p->pat2), pat2, actual2);
			return((void *)&(p->pat2));
		}
		p++;
	}
	return NULL;
}


/*
 * fills the memblock of <size> bytes from <startaddr> with the address
 */
void write_addrline(unsigned long startaddr, unsigned long size, int swapped)
{
	addrline_t *p, *pe;

	p = (addrline_t *)startaddr;
	pe = (addrline_t *)(startaddr + size);

	if (!swapped) {
		while (p < pe) {
			p->addr = (unsigned long)p;
			p++;
		}
	} else {
		while (p < pe) {
			p->addr = swap_32((unsigned long)p);
			p++;
		}
	}
}


/*
 * checks the memblock of <size> bytes from <startaddr>
 * returns the address of the error or NULL if all is well
 */
void *check_addrline(int mode, unsigned long startaddr, unsigned long size, 
                     int swapped)
{
	addrline_t *p, *pe;
	unsigned long actual, expected;

	p = (addrline_t *)startaddr;
	pe = (addrline_t *)(startaddr + size);

	if (!swapped) {
		while (p < pe) {
			actual = p->addr;
			expected = (unsigned long)p;
			if (actual != expected) {
				write_error(mode, (unsigned long)&(p->addr), expected, actual);
				return((void *)&(p->addr));
			}
			p++;
		}
	} else {
		while (p < pe) {
			actual = p->addr;
			expected = swap_32((unsigned long)p);
			if (actual != expected) {
				write_error(mode, (unsigned long)&(p->addr), expected, actual);
				return((void *)&(p->addr));
			}
			p++;
		}
	}
	return NULL;
}


/*
 * checks the memblock of <size> bytes from <startaddr+size>
 * returns the address of the error or NULL if all is well
 */
void *check_addrline_reverse(int mode, unsigned long startaddr, unsigned long size,
                             int swapped)
{
	addrline_t *p, *pe;
	unsigned long actual, expected;

	p = (addrline_t *)(startaddr + size - sizeof(p->addr));
	pe = (addrline_t *)startaddr;

	if (!swapped) {
		while (p > pe) {
			actual = p->addr;
			expected = (unsigned long)p;
			if (actual != expected) {
				write_error(mode, (unsigned long)&(p->addr), expected, actual);
				return ((void *)&(p->addr));
			}
			p--;
		}
	} else {
		while (p > pe) {
			actual = p->addr;
			expected = swap_32((unsigned long)p);
			if (actual != expected) {
				write_error(mode, (unsigned long)&(p->addr), expected, actual);
				return ((void *)&(p->addr));
			}
			p--;
		}
	}
	return NULL;
}


/*
 * fills the memblock of <size> bytes from <startaddr> with walking bit pattern
 */
void write_walkbit(unsigned long startaddr, unsigned long size)
{
	volatile unsigned long *p, *pe;
	unsigned long i;

	p = (unsigned long *)startaddr;
	pe = (unsigned long *)(startaddr + size);
	i = 0;

	while (p < pe) {
		*p = 1UL << i;
		i = (i + 1 + (((unsigned long)p) >> 7)) % 32;
		p++;
	}
}

/*
 * checks the memblock of <size> bytes from <startaddr>
 * returns the address of the error or NULL if all is well
 */
void *check_walkbit(int mode, unsigned long startaddr, unsigned long size)
{
	volatile unsigned long *p, *pe;
	unsigned long actual, expected;
	unsigned long i;

	p = (unsigned long *)startaddr;
	pe = (unsigned long *)(startaddr + size);
	i = 0;

	while (p < pe) {
		actual = *p;
		expected = (1UL << i);
		if (actual != expected) {
			write_error(mode, (unsigned long)p, expected, actual);
			return ((void *)p);
		}
		i = (i + 1 + (((unsigned long)p) >> 7)) % 32;
		p++;
	}
	return NULL;
}

/*
 * fills the memblock of <size> bytes from <startaddr> with "random" pattern
 */
void write_randompattern(unsigned long startaddr, unsigned long size, 
                         unsigned long *pat)
{
	unsigned long i, p;

	p = *pat;

	for (i = 0; i < (size / 4); i++) {
		*(unsigned long *)(startaddr + i*4) = p;
		if ((p % 2) > 0) {
			p ^= i;
			p >>= 1;
			p |= 0x80000000;
		} else {
			p ^= ~i;
			p >>= 1;
		}
	}
	*pat = p;
}

/*
 * checks the memblock of <size> bytes from <startaddr>
 * returns the address of the error or NULL if all is well
 */
void *check_randompattern(int mode, unsigned long startaddr, unsigned long size,
                          unsigned long *pat)
{
	void *perr = NULL;
	unsigned long i, p, p1;

	p = *pat;

	for (i = 0; i < (size / 4); i++) {
		p1 = *(unsigned long *)(startaddr + i*4);
		if (p1 != p) {
			if (perr == NULL) {
				write_error(mode, startaddr + i*4, p, p1);
				perr = (void *)(startaddr + i*4);
			}
		}

		if ((p % 2) > 0) {
			p ^= i;
			p >>= 1;
			p |= 0x80000000;
		} else {
			p ^= ~i;
			p >>= 1;
		}
	}
	*pat = p;
	return perr;
}


void test_writedata1(unsigned long startaddr, unsigned long size, unsigned long *pat)
{
	write_pattern(startaddr, size, TESTPAT1, TESTPAT2);
}

void *test_checkdata1(int mode, unsigned long startaddr, unsigned long size, unsigned long *pat)
{
	return (check_pattern(mode, startaddr, size, TESTPAT1, TESTPAT2));
}

void test_writedata2(unsigned long startaddr, unsigned long size, unsigned long *pat)
{
	write_pattern(startaddr, size, TESTPAT2, TESTPAT1);
}

void *test_checkdata2(int mode, unsigned long startaddr, unsigned long size, unsigned long *pat)
{
	return (check_pattern(mode, startaddr, size, TESTPAT2, TESTPAT1));
}

void test_writeaddr1(unsigned long startaddr, unsigned long size, unsigned long *pat)
{
	write_addrline(startaddr, size, FALSE);
}

void *test_check1addr1(int mode, unsigned long startaddr, unsigned long size, unsigned long *pat)
{
	return(check_addrline(mode,startaddr,size, FALSE));
}

void *test_check2addr1(int mode, unsigned long startaddr, unsigned long size, unsigned long *pat)
{
	return(check_addrline_reverse(mode,startaddr,size, FALSE));
}

void test_writeaddr2(unsigned long startaddr, unsigned long size, unsigned long *pat)
{
	write_addrline(startaddr, size, TRUE);
}

void *test_check1addr2(int mode, unsigned long startaddr, unsigned long size, unsigned long *pat)
{
	return(check_addrline(mode,startaddr,size, TRUE));
}

void *test_check2addr2(int mode, unsigned long startaddr, unsigned long size, unsigned long *pat)
{
	return(check_addrline_reverse(mode,startaddr,size, TRUE));
}



typedef struct {
	char *test_desc;
	void (*test_write)(unsigned long startaddr, unsigned long size, unsigned long *pat);
	void * (*test_check1)(int mode, unsigned long startaddr, unsigned long size, unsigned long *pat);
	void * (*test_check2)(int mode, unsigned long startaddr, unsigned long size, unsigned long *pat);
} memtest_func_t;


#define TEST_STAGES 5
const memtest_func_t test_stage[TEST_STAGES] = {
	{ 
		"data test 1...",
		test_writedata1, 
		test_checkdata1, 
		NULL
	}, {
		"data test 2...", 
		test_writedata2, 
		test_checkdata2, 
		NULL
	}, {
		"address line test...", 
		test_writeaddr1, 
		test_check1addr1, 
		test_check2addr1
	}, {
		"address line test (swapped)...", 
		test_writeaddr2, 
		test_check1addr2, 
		test_check2addr2
	}, {
		"random data test...", 
		write_randompattern, 
		check_randompattern, 
		NULL
	}
};



int mem_test(unsigned long start, unsigned long ramsize, int quiet)
{
	unsigned long errors,stage;
	unsigned long startaddr, size, i;
	const unsigned long blocksize = 0x80000;      /* check in 512KB blocks */
	unsigned long *perr;
	unsigned long rdatapat;
	int status[TEST_STAGES];
       
	for (stage = 0; stage < TEST_STAGES; stage++)
		status[stage] = TEST_PASSED;

	errors = 0;
	startaddr = start;
	size = ramsize;
	printk("\nMemory Test: addr = 0x%lx size = 0x%lx\n",startaddr,size);
	for (stage=0; stage<TEST_STAGES; stage++){
		printk(test_stage[stage].test_desc); printk("\n");
		/* fill SDRAM */
		rdatapat = 0x12345678;
		printk("writing block     :    ");
		for (i = 0; i < size; i += blocksize) {
			printk("%04lX\b\b\b\b",i / blocksize);
			test_stage[stage].test_write(startaddr+i, blocksize, &rdatapat);
		}
		printk("\n");
		/* check SDRAM */
		rdatapat = 0x12345678;
		printk("checking block    :    ");
		for (i = 0; i < size; i += blocksize) {
			printk("%04lX\b\b\b\b",i / blocksize);
			if ((perr = test_stage[stage].test_check1(quiet,startaddr+i, blocksize,&rdatapat)) != NULL) {
				status[stage] = TEST_FAILED;
			}
		}
		printk("\n");
		if( test_stage[stage].test_check2!=NULL) {
		/* check2 SDRAM */
			printk("2nd checking block:    ");
			rdatapat = 0x12345678;
			for (i = 0; i < size; i += blocksize) {
				printk("%04lX\b\b\b\b",i / blocksize);
				if ((perr = test_stage[stage].test_check2(quiet,startaddr+i, blocksize,&rdatapat)) != NULL) {
					status[stage] = TEST_FAILED;
				}
			}
			printk("\n");
		}

	} /* next stage */

	printk("\nMemory Test Result\n");
	for (stage = 0; stage < TEST_STAGES; stage++) {
		printk("Stage %d: %-33s ", stage+1, test_stage[stage].test_desc);
		if (status[stage] == TEST_FAILED)
			printk("FAIL\n");
		else
			printk("OK\n");
		
	}

	return 0;
}
