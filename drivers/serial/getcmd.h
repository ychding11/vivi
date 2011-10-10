#ifndef _VIVI_GETCMD_H_
#define _VIVI_GETCMD_H_


#define CTL_CH(c)		((c) - 'a' + 1)

#define MAX_CMDBUF_SIZE		256

#define CTL_BACKSPACE		('\b')
#define DEL			((char)255)
#define DEL7			((char)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)        putc(ch)
#define getcmd_getch()          getc()
#define getcmd_cbeep()          getcmd_putch('\a')

#define HIST_MAX		20
#define HIST_SIZE		MAX_CMDBUF_SIZE

void getcmd(char *, unsigned int);

/* debugging macros */
#undef DEBUG_GETCMD
#ifdef DEBUG_GETCMD
#include <printk.h>
#define DPRINTK(args...)	printk(##args)
#else
#define DPRINTK(args...)
#endif

#endif /* _VIVI_GETCMD_H_ */
