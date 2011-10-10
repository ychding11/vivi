#ifndef _HARDWARE_H_
#define _HARDWARE_H_

#define CTL_REG_READ(addr)		(*(volatile unsigned long *)(addr))
#define CTL_REG_WRITE(addr, val)	(*(volatile unsigned long *)(addr) = (val))

#define CTL_REG_READ_BYTE(addr)		(*(volatile unsigned char *)(addr))
#define CTL_REG_WRITE_BYTE(addr, val)	(*(volatile unsigned char *)(addr) = (val))
	
#ifndef __ASSEMBLY__

#define __REG(x)	(*(volatile unsigned long *)(x))
#define __REGl(x)	(*(volatile unsigned long *)(x))
#define __REGw(x)	(*(volatile unsigned short *)(x))
#define __REGb(x)	(*(volatile unsigned char *)(x))

#else

#define __REG(x)	(x)
#define __REGl(x)	(x)
#define __REGw(x)	(x)
#define __REGb(x)	(x)

#endif

#endif /* _HARDWARE_H_ */
