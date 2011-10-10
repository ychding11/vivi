/*
 * vivi/misc.c: functions used in vivi
 *
 * Copyright (C) MIZI Research, Inc. 2002
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "config.h"
#include "machine.h"
#include "cap.h"
#include "version.h"
#include "printk.h"
#include "vivi_string.h"
#include <types.h>


/*
 * dump hex
 */
#define BL_ISPRINT(ch)		(((ch) >= ' ') && ((ch) < 128))
void hex_dump(unsigned char *data, size_t num)
{
	int i;
	long oldNum;
	char buf[90];
	char *bufp;
	int line_resid;

	while (num) {
		bufp = buf;
		binarytohex(bufp, (unsigned long)data, 4);
		bufp += 8;
		*bufp++ = ':';
		*bufp++ = ' ';

		oldNum = num;

		for (i = 0; i < 16 && num; i++, num--) {
			binarytohex(bufp, (unsigned long)data[i], 1);
			bufp += 2;
			*bufp++ = (i == 7) ? '-' : ' ';
		}

		line_resid = (16 - i) * 3;
		if (line_resid) {
			memset(bufp, ' ', line_resid);
			bufp += line_resid;
		}

		memcpy(bufp, "| ", 2);
		bufp += 2;

		for (i = 0; i < 16 && oldNum; i++, oldNum--)
			*bufp++ = BL_ISPRINT(data[i]) ? data[i] : '.';

		line_resid = 16 - i;
		if (line_resid) {
			memset(bufp, ' ', 16 - i);
			bufp += 16 - i;
		}

		*bufp++ = '\r';
		*bufp++ = '\n';
		*bufp++ = '\0';
		putstr(buf);
		data += 16;
	}
}

unsigned int bsd_sum_memory(unsigned long img_src, size_t img_size) 
{
	unsigned long checksum = 0;   /* The checksum mod 2^16. */
	unsigned char *pch;       /* Each character read. */
	size_t i;

	pch = (unsigned char *)img_src;
	for (i = 1; i <= img_size; i++) {
		/* Do a right rotate */
		if (checksum & 01)
			checksum = (checksum >>	1) + 0x8000; 
		else    
			checksum >>= 1;
		checksum += *pch;      /* add the value to the checksum */
		checksum &= 0xffff;  /* Keep it within bounds. */
		pch++;  
	}
	return(checksum & 0xffff);
}

void progress_bar(unsigned long cur, unsigned long max)
{
        int percent, full_percent, count = 0, i;

        full_percent = (cur * 100) / max;
        percent = full_percent - (full_percent % 5);


	/* |====================|(   %) */

	if (cur) {
		for (i = 0; i < 28; i++) {
			printk("\b");
		}
	}

	printk("|");
	count = percent / 5;
	for (i = 0;  i < count; i++) 
		printk("=");
	for (i = 0; i < (20 - count); i++)
		printk(" ");
	printk("|(");
	printk("%3ld", full_percent);
	printk("%)");
}


void print_banner(void)
{
	long armrev = 0;
	long cpsr = 0;

	__asm__("mrc p15, 0, %0, c0, c0, 0" : "=r" (armrev));
	__asm__("mrs %0, cpsr" : "=r" (cpsr));
	putstr("Processor information\r\n");
	if (armrev == 0x41129200) {
	        putstr("  Processor: SAMSUNG S3C2400 (ARM920T core)\r\n");
	} else if (armrev == 0x6901b110) {
	        putstr("  Processor: INTEL SA1110\r\n");
	}
	//("  ARM Processor Revision: ", armrev);
	//putLabeledWord("  Processor CPSR: ", cpsr);
}

/* print (to serial) vivi capabilities */
extern unsigned long flash_base_addr;
void print_cap(void)
{
	unsigned long caps = *(volatile unsigned long *)(VIVI_ROM_BASE + 0x30);
	
	print_banner();

	putstr("VIVI capabilities\r\n");
	if (vivicap_canwakeup(caps))
		putstr("  Support power management\r\n");
	if (vivicap_canpartitions(caps))
		putstr("  Support MTD partition management\r\n");
	if (vivicap_canmtd(caps))
		putstr("  Support MTD management\r\n");
#ifdef CONFIG_S3C2400_GAMEPARK
	if (vivicap_canosswitch(caps))
		putstr("  Support OS Switching\r\n");
	if (vivicap_canloadusr(caps))
		putstr("  Support loading uer image\r\n");
#endif
}

void print_disk_size(unsigned int _size, const char *ppad, const char *npad)
{
    unsigned int size;

    _size /= 1024;
    if (_size) {
      size = _size / 1024;
      if (size) {
	/* unit = 'M'; */
	_size %= 1024;
	if (_size)
	  printk("%s%3dM+%3dk%s", ppad, size, _size, npad);
	else
	  printk("%s%3dM%s", ppad, size, npad);
      } else {
	/* unit = 'k'; */
	printk("%s%3dk%s", ppad, _size, npad);
      }
    } else {
      /* unit = ' '; */
      printk("%s%s", ppad, npad);
    }
}
