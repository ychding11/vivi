#ifndef _VIVI_STRING_H_
#define _VIVI_STRING_H_

void u32todecimal(char *, unsigned long);
void binarytohex(char *, long, int);
unsigned long strtoul(const char *str, char **endptr, int requestedbase, int *ret);

void putstr_hex(const char *str, unsigned long value);
void putstr(const char *str);

#endif /* _VIVI_STRING_H_ */
