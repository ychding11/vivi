#ifndef _VIVI_TIME_H_
#define _VIVI_TIME_H_

#include "config.h"

#ifndef __ASSEMBLY__
void init_time(void);
void mdelay(unsigned int);
void udelay(unsigned int);
#endif

#endif /* _VIVI_TIME_H_ */
