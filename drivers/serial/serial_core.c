#include "config.h"
#include "machine.h"
#include "serial.h"
#include "processor.h"
#include <types.h>

unsigned char
do_getc_with_timeout(unsigned long timeout, int*stat)
{
	unsigned char c;
	int rxstat;

	while (!SERIAL_CHAR_READY()) {
		if (--timeout == 0) break;
	}

	if (!timeout) {
		c = 0;
		*stat = -1;
	} else {
		c = SERIAL_READ_CHAR();
		*stat = SERIAL_READ_STATUS();	
	}

	return (c);
}

int getc_errno = 0;
__u8 do_getc(vfuncp idler, unsigned long timeout, int *statp)
{
	__u8	c, rxstat;
	int	do_timeout = timeout != 0;

	getc_errno = 0;	/* reste errno */

	while(!SERIAL_CHAR_READY()) {

		if (do_timeout) {
			if (!timeout)
				break;
			timeout--;
		}

		if (idler)
			idler();
	}

	if (do_timeout && timeout == 0) {
		c = 0;
		rxstat = -1;
	} else {
		c = SERIAL_READ_CHAR();
		rxstat = SERIAL_READ_STATUS();
	}

	if (rxstat) {
		getc_errno = rxstat;
		/*printk("RXSTAT error. status = 0x%08lx", rxstat);*/
		if (statp)
			*statp = rxstat;
	}
	return (c);
}

char getc(void)
{
	char c;
	unsigned long rxstat;

	while (!SERIAL_CHAR_READY()) /* nothing */;

	c = SERIAL_READ_CHAR();
	/* FIXME: 여기서 에러 핸들링이 필요함 */
	rxstat = SERIAL_READ_STATUS();

	return c;
}

void putc(char c)
{
	PROC_SERIAL_PUTC(c);
}

/*
 * Reads and returns a character from the serial port
 *   - Times out after delay iterations checking for presence of character
 *   - Sets *error_p to UART error bits or - on timeout
 *   - On timeout, sets *error_p to -1 and returns 0
 */
char awaitkey(unsigned long delay, int* error_p)
{
	return (do_getc(NULL, delay, error_p));
}

