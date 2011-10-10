/*
 * vivi/deriver/serialxmodem.c:
 *     - an implementation of the xmodem protocol from the spec.
 *
 * Based on bootldr/xmodem.c (C) 2000 Compaq Computer Coporation.
 *
 * Copyright (C) 2001 MIZI Research, Inc.
 *
 *
 * 1999-01-xx: Edwin Foo <efoo@crl.dec.com>
 *    - Initial code
 *
 * 2001-10-04: Janghoon Lyu <nandy@mizi.com>
 *    - Modified a little bit.
 *
 * 2002-07-04: Janghoon Lyu <nandy@mizi.com>
 *    - get_param_value() 인터페이스가 바뀌어서 이에 맞게 수정.
 */

#include "config.h"
#include "machine.h"
#include "serial.h"
#include "priv_data.h"
#include <types.h>

/* XMODEM parameters */
#define BLOCK_SIZE	128	/* size of transmit blocks */
#define RETRIES		20	/* maximum number of RETRIES */

/* Line control codes */
#define SOH		0x01	/* start of header */
#define ACK		0x06	/* Acknowledge */
#define NAK		0x15	/* Negative acknowledge */
#define CAN		0x18	/* Cancel */
#define EOT		0x04	/* end of text */

#define GET_BYTE_TIMEOUT 10000000

/* global error variable */
char *xmodem_errtxt = NULL;
int get_byte_err = 0;
__u8 volatile rbuffer[BLOCK_SIZE];

/* prototypes of helper functions */
int get_record(void);
__u8 get_byte(void);

enum
{
	SAC_SEND_NAK = 0,
	SAC_SENT_NAK = 1,
	SAC_PAST_START_NAK = 2
};

static volatile int seen_a_char = SAC_SEND_NAK;

static int one_nak = 0;
static unsigned long xmodem_timeout = GET_BYTE_TIMEOUT;

char debugbuf[4096];
int db_idx = 0;

void bufputs(char *s)
{
	size_t len = strlen(s) + 1;

	if (len + db_idx > sizeof(debugbuf))
		len = sizeof(debugbuf) - db_idx;

	if (len) {
		memcpy(&debugbuf[db_idx], s, len);
		db_idx += len;
	}
}

void reset_debugbuf(void)
{
	memset(debugbuf, 0x2a, sizeof(debugbuf));
	db_idx = 0;
}

__u32 xmodem_receive(char *dldaddr, size_t len)
{
	char ochr;
	int r = 0, rx_block_num = 0, error_count = 0;
	__u32 foffset = 0;
	int i;
	int ret;

	xmodem_errtxt = NULL;
	seen_a_char = 0;

	one_nak = get_param_value("xmodem_one_nak", &ret);
	if (ret) one_nak = 0;

	xmodem_timeout = get_param_value("xmodem_initial_timeout", &ret);
	if (ret) xmodem_timeout = GET_BYTE_TIMEOUT;

	rx_block_num = 1;
	error_count = RETRIES;

	do {
		if ((r = get_record()) == (rx_block_num & 255)) {
			error_count = RETRIES;
			for (i = 0; i <BLOCK_SIZE; i++)
				*(__u8 *)(dldaddr+foffset+i) = rbuffer[i];
			xmodem_errtxt = "RX PACKET";
			rx_block_num++;
			ochr = ACK;
			foffset += BLOCK_SIZE;
		} else {
			switch (r) {
			case -1: /* TIMEOUT */
				xmodem_errtxt = "TIMEOUT";
				ochr = NAK;
				break;
			case -2: /* Bad block */
				xmodem_errtxt = "BAD BLOCK#";
				/* eat teh rest of the block */
				get_byte_err = 0;
				while (get_byte_err != -1) get_byte();
				ochr = NAK;
				break;
			case -3: /* Bad checksum */
				xmodem_errtxt = "BAD CHKSUM";
				ochr = NAK;
				break;
			case -4: /* End of file */
				xmodem_errtxt = "DONE";
				ochr = ACK;
				break;
			case -5: /* Cancel */
				xmodem_errtxt = "ABORTED";
				ochr = ACK;
				break;
			default: /* Block out of sequence */
				xmodem_errtxt = "WRONG BLK";
				ochr = NAK;
			}
			error_count--;
		}
		putc(ochr);
	} while ((r > -3) && error_count);

	if ((!error_count) || (r != -4)) {
		foffset = 0;	/* indicate failure to caller */
		/*printk("x-modem error: %s\n", xmodem_errtxt); */
	}

	return foffset;
}

/*
 * Read a record in the XMODEM protocol, return the block number
 * (0-255) if successful, or one of the following return codes:
 * 	-1 = Bad byte
 * 	-2 = Bad block number
 * 	-3 = Bad block checksum
 * 	-4 = End of file
 * 	-5 = Canceled by remote
 */
int get_record(void)
{
	int c, block_num = 0;
	int i;
	__u32 check_sum;

	/* clear the buffer */
	for (i = 0; i < BLOCK_SIZE; i++)
		rbuffer[i] = 0x00;

	check_sum = 0;
	i = -2;
	c = get_byte();
	if (get_byte_err)
		return -1;

	switch (c) {
	case SOH:	/* Receive packet */
		for (;;) {
			c = get_byte();
			if (get_byte_err)
				return -1;

			switch (i) {
			case -2: 
				block_num = c;
				break;
			case -1:
#if 0
#ifdef CHECK_NEGATED_SECTNUM
				if (c != (-block_num -1))
					return -2;
#endif
#endif
				break;
			case BLOCK_SIZE:
				if ((check_sum & 0xff) != c)
					return -3;
				else
					return block_num;
				break;
			default:
				rbuffer[i] = c;
				check_sum += c;
			}
			i++;
		}
	case EOT:	/* end of file encountered */
		return -4;
	case CAN:	/* cancel protocol */
		return -5;
	default:
		return -5;
	}
}

/* get_byte should use one of the timer's for a CPU clock independent timeout */
__u8 get_byte()
{
	int c, ret;

again:
	c = 0;
	get_byte_err = 0; /* reset errno */
	c = awaitkey(xmodem_timeout, &get_byte_err);

	if (get_byte_err) {
		if (seen_a_char == SAC_SEND_NAK || !one_nak) {
			bufputs("timeout nak");
			putc(NAK);	/* make the sender go */
		}

		if (seen_a_char < SAC_PAST_START_NAK) {
			bufputs("goto again");
			seen_a_char = SAC_SENT_NAK;

			xmodem_timeout = get_param_value("xmodem_timeout", &ret);
			if (ret) xmodem_timeout = GET_BYTE_TIMEOUT;
			goto again;
		}
	}

	if (get_byte_err == 0)
		seen_a_char = SAC_PAST_START_NAK;

	return (c);
}
