/*
 * vivi/include/proc/proc_sa1100.h: 
 *   SA1100에 의존적인 녀석들을 정의하는 곳.
 *   vivi/include/processor.h에 인클루드 됨. 다른 어떤 곳에서도 요놈을 인클루드
 *   하지 않음
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2002/07/19 08:02:35 $ 
 *
 * $Revision: 1.1.1.1 $
 * $Id: proc_sa1100.h,v 1.1.1.1 2002/07/19 08:02:35 nandy Exp $
 *
 *
 * History
 *
 * 2002-07-08: Janghoon Lyu <nandy@mizi.com>
 *     - 처음 이 파일을 만들었음.
 *
 */

#ifndef _PROC_SA1100_H_
#define _PROC_SA1100_H_

#include "sa1100.h"

/*
 * UART
 *
 * You may be define six function.
 *  SERIAL_READ_READY(), SERIAL_READ_CHAR(), SERIAL_READ_STATUS(),
 *  SERIAL_WRITE_READY(), SERIAL_WRITE_CHAR(), SERIAL_WRITE_STATUS()
 */


#ifdef CONFIG_SERIAL_UART0
#define SERIAL_CHAR_READY()	((*(volatile long *)SA1100_UART0_UTSR1) & SA1100_UTSR1_RNE)
   /* must read UARTDR_REG before RXSTAT_REG */
#define SERIAL_READ_CHAR()	(*(volatile unsigned char *)SA1100_UART0_UTDR)
#define SERIAL_READ_STATUS()	((*(volatile unsigned char *)SA1100_UART0_UTSR1) & SA1100_UTSR1_ERROR_MASK)

#elif defined(CONFIG_SERIAL_UART1)
#define SERIAL_CHAR_READY()	((*(volatile long *)SA1100_UART1_UTSR1) & SA1100_UTSR1_RNE)
   /* must read UARTDR_REG before RXSTAT_REG */
#define SERIAL_READ_CHAR()	(*(volatile unsigned char *)SA1100_UART1_UTDR)
#define SERIAL_READ_STATUS()	((*(volatile unsigned char *)SA1100_UART1_UTSR1) & SA1100_UTSR1_ERROR_MASK)
#else
#error not support this serial port
#endif


#ifdef CONFIG_SERIAL_UART0
#define SERIAL_WRITE_STATUS()	(*(volatile long *)(SA1100_UART0_UTSR1))
#define SERIAL_WRITE_READY()	((*(volatile long *)(SA1100_UART0_UTSR1)) & SA1100_UTSR1_TNF)
#define SERIAL_WRITE_CHAR(c)	((*(volatile char *)(SA1100_UART0_UTDR)) = (c))
#elif defined(CONFIG_SERIAL_UART1)
#define SERIAL_WRITE_STATUS()	(*(volatile long *)(SA1100_UART1_UTSR1))
#define SERIAL_WRITE_READY()	((*(volatile long *)(SA1100_UART1_UTSR1)) & SA1100_UTSR1_TNF)
#define SERIAL_WRITE_CHAR(c)	((*(volatile char *)(SA1100_UART1_UTDR)) = (c))
#else
#error not support this serial port
#endif

#endif /* _PROC_SA1100_H_ */
