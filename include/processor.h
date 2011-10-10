/*
 * vivi/include/processor.h:
 *   - 프로세서에 의존적인 요소들을 정의합니다.
 *
 *
 */
#ifndef _VIVI_PROC_H_
#define _VIVI_PROC_H_

#include "config.h"

#include "proc/proc_s3c2410.h"

#define PROC_SERIAL_PUTC(c)	\
	({ while (!SERIAL_WRITE_READY()); \
	   SERIAL_WRITE_CHAR(c); })

#endif /* _VIVI_PROC_H_ */
