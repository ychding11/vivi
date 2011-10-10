/* 
 * YMODEM 
 * 
 * Copyright (C) 2001  John G Dorsey
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The author may be contacted via electronic mail at <john+@cs.cmu.edu>,
 * or at the following address:
 *
 *   John Dorsey
 *   Carnegie Mellon University
 *   HbH2201 - ICES
 *   5000 Forbes Avenue
 *   Pittsburgh, PA  15213
 *
 *
 * Notes:
 * ^^^^^^
 * Tested against lsz (`sb') from within Minicom. The YMODEM spec says
 * that the receiver should just keep sending CRC/NAKs to the sender 
 * until the transfer begins, but something breaks if a CRC/NAK is
 * sent out while the user is typing in a filename to Minicom. Best
 * solution currently is just to repeat the transfer.
 *
 * History:
 * ^^^^^^^^
 * 12 March, 2001 - created. (jd)
 *
 */

/* 
 * Hacked to work with vivi by Janghoon Lyu <nandy@mizi.com>, Oct 16 2002 
 * 
 * - The vivi does not use CRC or checksum for error dectecion anymore
 *   because these days the sereial communication is very stable.
 */

#include <config.h>
#include <machine.h>
#include <serial.h>
#include <types.h>
#include <printk.h>
#include <vivi_string.h>
#include <time.h>
#include <priv_data.h>

#include "ymodem.h"

static int
receive_byte(char *c, unsigned long timeout)
{
	int error = 0;

	*c = do_getc_with_timeout(timeout, &error);

	return error ? -1 : 0;
}


/* Returns 0 on success, 1 on corrupt packet, -1 on error (timeout): */
static int 
receive_packet(char *data, int *length, unsigned long timeout)
{
	int i;
	unsigned int packet_size;
	char c;

	*length = 0;

	if(receive_byte(&c, timeout) < 0)
		return -1;

	switch (c) {
	case SOH:
		packet_size = PACKET_SIZE;
		break;

	case STX:
		packet_size = PACKET_1K_SIZE;
		break;

	case EOT:
		return 0;

	case CAN:
		if (receive_byte(&c, timeout) == 0 && c == CAN) {
			*length = -1;
			return 0;
		}

	/* 
	 * This case could be the result of corruption on the first octet
	 * of the packet, but it's more likely that it's the user banging
	 * on the terminal trying to abort a transfer. Technically, the
	 * former case deserves a NAK, but for now we'll just treat this
	 * as an abort case.
	 */
	default:
		*length = -1;
		return 0;
	}

	*data = c;

	/*
	 * Note by nandy:
	 * I assumed that the minicom send checksum bits. So the receiver(the vivi)
	 * receive checksum bits.
	 */
	for (i = 1; i < (packet_size + PACKET_OVERHEAD); ++i)
		if (receive_byte(data + i, timeout) < 0)
			return -1;

#if 0 /* Note by nandy: Below codes must be disabled in the vivi. Why? */
	/* 
	 * Just a sanity check on the sequence number/complement value. 
	 * Caller should check for in-order arrival.
 	 */
	if (data[PACKET_SEQNO_INDEX] != (data[PACKET_SEQNO_COMP_INDEX] ^ 0xff) & 0xff)
		return 1;
#endif

	*length = packet_size;

	return 0;
}

/* Returns the length of the file received, or 0 on error: */
int
ymodem_receive(char *buf, unsigned int length, char *name)
{
	unsigned char packet_data[PACKET_1K_SIZE + PACKET_OVERHEAD];
	int packet_length, i, file_done, session_done;
	unsigned int packets_received, errors;
	char file_name[FILE_NAME_LENGTH], file_size[FILE_SIZE_LENGTH], *file_ptr;
	char *buf_ptr;
	unsigned long size = 0;
	unsigned long timeout, initial_timeout, nak_timeout; 
	int ret;

	initial_timeout = get_param_value("ymodem_initial_timeout", &ret);
	if (ret) initial_timeout = INITIAL_TIMEOUT;
	nak_timeout = NAK_TIMEOUT;

	/* Give the user time to frantically type in the file name: */
	timeout = initial_timeout;

	for (session_done = 0, errors = 0; ; ) {


		for (packets_received = 0, file_done = 0, buf_ptr = buf; ; ) {

			switch (receive_packet(packet_data, &packet_length, timeout)) {
			case 0:
				errors = 0;

				switch (packet_length) {
				case -1:  /* abort */
					putc(ACK);
					return 0;
	  
				case 0:   /* end of transmission */
					putc(ACK);
	  
					/* 
					 * Should add some sort of sanity check on the 
					 * number of packets received and the advertised 
					 * file length.  
					 */ 
					file_done = 1;
					break;
	  
				default:  /* normal packet */
					if ((packet_data[PACKET_SEQNO_INDEX] & 0xff) !=
						(packets_received & 0xff)) {
						putc(NAK);
					} else {
						if (packets_received == 0) {
						/* The spec suggests that the whole data 
						 * section should be zeroed, but I don't 
						 * think all senders do this. If we have 
						 * a NULL filename and the first few digits 
						 * of the file length are zero, we'll call 
						 * it empty. */
							for (i = PACKET_HEADER; i < PACKET_HEADER + 4; ++i)
								if(packet_data[i] != 0)
									break;

							if (i < PACKET_HEADER + 4) {  /* filename packet has data */
								for (file_ptr = packet_data + PACKET_HEADER, i = 0; *file_ptr && i < FILE_NAME_LENGTH;)
									file_name[i++] = *file_ptr++;
								file_name[i++] = '\0';
								if (name != NULL)
									strcpy(name, file_name);
								for (++file_ptr, i = 0; *file_ptr != ' ' && i < FILE_SIZE_LENGTH;)
									file_size[i++] = *file_ptr++;
								file_size[i++] = '\0';
								size = strtoul(file_size, NULL, 0, NULL);

								if (size > length) {
									putc(CAN);
									putc(CAN);
									mdelay(3000);

									printk("Receive buffer too small (%d) to accept file size %d\n", length, size);
									return 0;
								}
		
								putc(ACK);
								/* Note by nandy: Why? */
								putc(NAK);
							} else {  /* filename packet is empty; end session */
								putc(ACK);

								file_done = 1;
								session_done = 1;
								break;
							}
						} else {
						/* This shouldn't happen, but we check 
						 * anyway in case the * sender lied in 
						 * its filename packet: */
							if ((buf_ptr + packet_length) - buf > length) {
								putc(CAN);
								putc(CAN);

								mdelay(3000);

								printk("Sender exceeded size of receive buffer: %d", length);
								return 0;
							}

							memcpy(buf_ptr, packet_data + PACKET_HEADER, packet_length);
							buf_ptr += packet_length;
							putc(ACK);
						}
	    
						++packets_received;
	    
					}  /* sequence number ok */
				}
				break;

			default:
				if (++errors >= ((packets_received == 0 ? MAX_CRC_TRIES : 0) + MAX_ERRORS)) {
					putc(CAN);
					putc(CAN);

					mdelay(1000);

					printk("Too many errors during receive; giving up.\n");
					return 0;

				}

				if (packets_received == 0) {
					timeout = nak_timeout;
				}
				/* Note by nandy: Why? */
				putc(NAK);
			}
      
			if (file_done)
				break;
		}  /* receive packets */

		if(session_done)
			break;
	}  /* receive files */

	return size;
}
