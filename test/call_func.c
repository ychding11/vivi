/*
 *  try to call the VIVI's builtin function
 *
 *
 *  only for engineer test
 *
 *    bushi@mizi.com
 *
 */

#include "function.map"

int main()
{
	unsigned char a;

	printk("Hit anykey to boot!!\n");

	a = getc();

	command_boot(1, 0);

	return 0;
}
