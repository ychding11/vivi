#include <printk.h>

typedef unsigned long u32;

#define GPGCON *(volatile u32*)(0x56000060)
#define GPGDAT *(volatile u32*)(0x56000064)
#define GPGUP *(volatile u32*)(0x56000068)
#define GPBCON *(volatile u32*)(0x56000010)
#define GPBDAT *(volatile u32*)(0x56000014)
#define GPBUP *(volatile u32*)(0x56000018)
#define TCON *(volatile u32*)(0x51000008)
#define TCFG0 *(volatile u32*)(0x51000000)
#define TCFG1 *(volatile u32*)(0x51000004)
#define TCNTB0 *(volatile u32*)(0x5100000c)
#define TCMPB0 *(volatile u32*)(0x51000010)
#define TCNTO0 *(volatile u32*)(0x51000014)

#define TCON_0_START	(1UL<<0)
#define TCON_0_MAN	(1UL<<1)
#define TCON_0_INV (1UL<<2)
#define TCON_0_AUTO (1UL<<3)

void mdelay(u32 msec)
{
	u32 i,j,k;

	for (i=0; i<msec; i++)
		for (j=0;j<300;j++)
			k = j;
}


int main(int argc, char *argv[])
{
	// timer0 off
	TCON &= ~TCON_0_START;
	
	// GPG4 : output, high
	// 00xx  0000  0000
	GPGCON &= 0xfffffcff;
	GPGCON |= 0x00000100;
	// 000x 0000
	GPGUP |= 0x00000010;
	// 000x 0000
	GPGDAT |= 0x00000010;

#if 1 // TIMER 0
	// GPB0 : alt-func. 
	// 00xx
	GPBCON &= 0xfffffffc;
	GPBCON |= 0x00000002;
	// 0001
	GPBUP |= 0x00000001;

	TCFG0 = 199; // 50M -> 250,000 Hz

	TCFG1 = 1; // 1/4 :  250,000 -> 62,500 Hz

#if 1
	TCMPB0 = 209; // 50% duty

	TCNTB0 = 417; // 62,500 -> 149 Hz
#else
	TCNTB0 = 20800;  // 3Hz
	TCMPB0 = 10400;  //  50 % duty
//	TCNTO0 = TCNTB0; // ??
#endif

	// timer0 start
#if 1
	TCON = TCON_0_MAN | ~(TCON_0_START);
	TCON = TCON_0_MAN | TCON_0_AUTO | TCON_0_START;
#else
	TCON = TCON_0_AUTO | TCON_0_START;

#endif

	printk("TCON = 0x%08lx\n", TCON);

	while (1) {
//		printk("0x%08lx\n", TCNTO0);
//		printk("0x%08lx\n", TCNTB0);
	}
#else // GPIO control
	// GPB0 : output
	GPBCON &= 0xfffffffc;
	GPBCON |= 0x00000001;
	GPBUP |= 0x00000001;

	while(1) {
		GPBDAT |= 0x00000001;
		mdelay(200);
		GPBDAT &= 0xfffffffe;
		mdelay(200);
	}

#endif

	return 0;
}
