/*
 * proc_pxa250.h
 *
 * processor specified definitions
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2002/09/03 07:43:44 $ 
 *
 * $Revision: 1.5 $
 *
   Mon Jul 22 2002 Yong-iL Joh <tolkien@mizi.com>
   - initial

 */

#ifndef _PROC_PXA_H_
#define _PROC_PXA_H_

#include "pxa250.h"

/*
 * UART
 *
 * You may be define six function.
 *  SERIAL_READ_READY(), SERIAL_READ_CHAR(), SERIAL_READ_STATUS(),
 *  SERIAL_WRITE_READY(), SERIAL_WRITE_CHAR(), SERIAL_WRITE_STATUS()
 */
#ifdef CONFIG_SERIAL_UART0
#define SERIAL_CHAR_READY()	(FFLSR & SIO_LSR_DR)
#define SERIAL_READ_CHAR()	(FFRBR)
#define SERIAL_READ_STATUS()	((__u8)(FFLSR & SIO_LSR_ERR))
#define SERIAL_WRITE_READY()	(FFLSR & SIO_LSR_THRE)
#define SERIAL_WRITE_CHAR(c)	(FFTHR = (c))
#define SERIAL_WRITE_STATUS()	((__u8)(FFLSR & SIO_LSR_ERR))

#elif defined(CONFIG_SERIAL_UART1)
#define SERIAL_CHAR_READY()	(BTLSR & SIO_LSR_DR)
#define SERIAL_READ_CHAR()	(BTRBR)
#define SERIAL_READ_STATUS()	((__u8)(BTLSR & SIO_LSR_ERR))
#define SERIAL_WRITE_READY()	(BTLSR & SIO_LSR_THRE)
#define SERIAL_WRITE_CHAR(c)	(BTTHR = (c))
#define SERIAL_WRITE_STATUS()	((__u8)(BTLSR & SIO_LSR_ERR))

#elif defined(CONFIG_SERIAL_UART2)
#define SERIAL_CHAR_READY()	(STLSR & SIO_LSR_DR)
#define SERIAL_READ_CHAR()	(STRBR)
#define SERIAL_READ_STATUS()	((__u8)(STLSR & SIO_LSR_ERR))
#define SERIAL_WRITE_READY()	(STLSR & SIO_LSR_THRE)
#define SERIAL_WRITE_CHAR(c)	(STTHR = (c))
#define SERIAL_WRITE_STATUS()	((__u8)(STLSR & SIO_LSR_ERR))

#else
#error not support this serial port
#endif

#endif	/* _PROC_PXA_H_ */
/*
 | $Id: proc_pxa250.h,v 1.5 2002/09/03 07:43:44 tolkien Exp $
 |
 | Local Variables:
 | mode: c
 | mode: font-lock
 | version-control: t
 | delete-old-versions: t
 | End:
 |
 | -*- End-Of-File -*-
 */
