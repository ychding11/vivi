/*
 * vivi/drivers/serial/getcmd.c
 *
 * Based on bootldr/bootldr.c
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/08/20 08:17:09 $
 *
 * $Revision: 1.1 $
 */

#include <config.h>
#include <serial.h>
#include "getcmd.h"


void getcmd(char *cmd_buf, unsigned int len)
{
	char curpos = 0;	/* current position - index into cmd_buf */
	char c;
	int cmd_echo = 1;

	/* Clear out the buffer */
	memset(cmd_buf, 0, MAX_CMDBUF_SIZE);

	for (;;) {
		c = getc();
		switch (c) {
		case 0x08:
		case 0x06:
		case 0x07:
		case 0x7E:
		case 0x7F:	/* backspace or delete */
			/* we're not at the beginning of the line */
			if (curpos) {
				curpos--;
				putc(0x08);	/* go backwards */
				putc(' ');	/* overwrite the char */
				putc(0x08);	/* go back again */
			}
			cmd_buf[curpos] = '\0';
			break;
		case '\r':
		case '\n':
		case '\0':
			putc('\r');
			putc('\n');
			goto end_cmd;
		case CTL_CH('x'):
			curpos = 0;
			break;

		default:
			if (curpos < MAX_CMDBUF_SIZE) {
				cmd_buf[curpos] = c;
				/* echo it back out to the screen */
				if (cmd_echo)
					putc(c);
				curpos++;
			}
			break;
		}
	}
end_cmd:
	DPRINTK("COMMAND: %s\n", cmd_buf);
}
