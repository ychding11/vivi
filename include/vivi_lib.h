#ifndef _VIVI_LIB_H_
#define _VIVI_LIB_H_

#include <types.h>

void progress_bar(unsigned long cur, unsigned long max);
void hex_dump(unsigned char *data, size_t num);
void print_disk_size(unsigned int _size, const char *ppad, const char *npad);

#endif /* _VIVI_LIB_H_ */
