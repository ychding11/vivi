#ifndef _VIVI_IO_H_
#define _VIVI_IO_H_

#define readb(x)	*(volatile unsigned char *)(x)
#define readw(x)	*(volatile unsigned short *)(x)
#define readl(x)	*(volatile unsigned long *)(x)

#define writeb(v,x)	*(volatile unsigned char *)(x) = (v)
#define writew(v,x)	*(volatile unsigned short *)(x) = (v)
#define writel(v,x)	*(volatile unsigned long *)(x) = (v)

#endif /* _VIVI_IO_H_ */
