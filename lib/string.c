/*
 * vivi/lib/string.c: 비비에서만 사용하는 스트링 관련 함수들.
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/08/20 08:11:15 $
 *
 * $Revision: 1.3 $
 * $Id: string.c,v 1.3 2002/08/20 08:11:15 nandy Exp $
 *
 *
 */

#include "config.h"
#include "vivi_string.h"
#include "serial.h"
#include "printk.h"
#include <types.h>

#ifdef DEBUG_STRING
#define DPRINTK(args...)	printk(##args)
#else
#define DPRINTK(args...)
#endif

static char hex_to_ascii_table[16] = "0123456789ABCDEF";

void u32todecimal(char *buf, unsigned long x)
{
        int i = 0;
        int j = 0;
        char localbuf[16];

        if (x != 0) {
                while (x > 0) {
                        unsigned long rem = x % 10;
                        localbuf[i++] = hex_to_ascii_table[rem];
                        x /= 10;
                }
                /* now reverse the characters into buf */
                while (i > 0) {
                        i--;
                        buf[j++] = localbuf[i];
                }
                buf[j] = '\0';
        } else {
                buf[0] = '0';
                buf[1] = '\0';
        }
}

void binarytohex(char *buf, long x, int nbytes)
{
        int i;
        int s = 4*(2*nbytes - 1);
        if (hex_to_ascii_table[0] != '0')
                putstr("hex_to_ascii_table corrupted\r\n");
        for (i = 0; i < 2*nbytes; i++){
                buf[i] = hex_to_ascii_table[(x >> s) & 0xf];
                s -= 4;
        }
        buf[2*nbytes] = 0;
}


/*
 *
 */
unsigned long strtoul(const char *str, char **endptr, int requestedbase, int *ret)
{
        unsigned long num = 0;
        char c;
        __u8 digit;
        int base = 10;
        int nchars = 0;
        int leadingZero = 0;

        *ret = 0;

        while ((c = *str) != 0) {
                if (nchars == 0 && c == '0') {
                        leadingZero = 1;
                        DPRINTK("strtoul(): leadingZero nchar=%d", nchars);
                        goto step;
                } else if (leadingZero && nchars == 1) {
                        if (c == 'x') {
                                base = 16;
                                DPRINTK("strtoul(): base 16 nchars=%d", nchars);
                                goto step;
                        } else if (c == 'o') {
                                base = 8;
                                DPRINTK("strtoul(): base8 nchars=%d", nchars);
                                goto step;
                        }
                }
                DPRINTK("strtoul: c=%c", c);
                if (c >= '0' && c <= '9') {
                        digit = c - '0';
                } else if (c >= 'a' && c <= 'z') {
                        digit = c - 'a' + 10;
                } else if (c >= 'A' && c <= 'Z') {
                        digit = c - 'A' + 10;
                } else {
                        *ret = 3;
                        return 0;
                }
                if (digit >= base) {
                        *ret = 4;
                        return 0;
                }
                num *= base;
                num += digit;
step:
                str++;
                nchars++;

        }
        return num;
}


/*
 * Simple print string
 */

void putnstr(const char *str, size_t n)
{
	if (str == NULL)
		return;

	while (n && *str != '\0') {
		putc(*str);
		str++;
		n--;
	}
}

void putstr(const char *str)
{
	putnstr(str, strlen(str));
}

void putstr_hex(const char *str, unsigned long value)
{
	char buf[9]; 
	binarytohex(buf, value, 4);
	putstr(str);
	putstr(buf);
	putstr("\r\n");
}
