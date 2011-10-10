/*
 * Copyright 2001 MIZI Research Coporation.
 *
 * Copying or modifying this code for any purpose is permitted,             
 * provided that this copyright notice is preserved in its entirety        
 * in all copies or modifications.  COMPAQ COMPUTER CORPORATION           
 * MAKES NO WARRANTIES, EXPRESSED OR IMPLIED, AS TO THE USEFULNESS       
 * OR CORRECTNESS OF THIS CODE OR ITS FITNESS FOR ANY PARTICULAR        
 * PURPOSE.                                                           
 */
/*
 * Thr 4 Oct 2001
 * Janghoon Lyu <nandy@mizi.com> 
 */

#include "hardware.h"
#include "bitfield.h"

/*
 * Program Status Register 
 */
#define USR26_MODE		0x00
#define FIQ26_MODE		0x01
#define IRQ26_MODE		0x02
#define SVC26_MODE		0x03
#define USR_MODE		0x10
#define FIQ_MODE		0x11
#define IRQ_MODE		0x12
#define SVC_MODE		0x13
#define ABT_MODE		0x17
#define UND_MODE		0x1b
#define SYSTEM_MODE		0x1f
#define MODE_MASK		0x1f
#define F_BIT			0x40
#define I_BIT			0x80
#define CC_V_BIT		(1 << 28)
#define CC_C_BIT		(1 << 29)
#define CC_Z_BIT		(1 << 30)
#define CC_N_BIT		(1 << 31)

/*
 * Memory Controller
 */
#define DRAM_CFG_BASE		0x14000000
#define bDRAM(Nb)		__REG(DRAM_CFG_BASE + (Nb))

#define S3C2400_BWSCON		0x00	/* R/W, Bus witdth & wait status control reg. */
#define S3C2400_BANKCON0	0x04	/* R/W, Bank 0 control reg. */
#define S3C2400_BANKCON1	0x08	/* R/W, Bank 1 control reg. */
#define S3C2400_BANKCON2	0x0C	/* R/W, Bank 2 control reg. */
#define S3C2400_BANKCON3	0x10	/* R/W, Bank 3 control reg. */
#define S3C2400_BANKCON4	0x14	/* R/W, Bank 4 control reg. */
#define S3C2400_BANKCON5	0x18	/* R/W, Bank 5 control reg. */
#define S3C2400_BANKCON6	0x1C	/* R/W, Bank 6 control reg. */
#define S3C2400_BANKCON7	0x20	/* R/W, Bank 7 control reg. */
#define S3C2400_REFRESH		0x24	/* R/W, DRAM/SDRAM refresh control reg. */
#define S3C2400_BANKSIZE	0x28	/* R/W, Flexible bank size register */
#define S3C2400_MRSRB6		0x2C	/* R/W, Mode register set register bank6 */
#define S3C2400_MRSRB7		0x30	/* R/W, Mode register set register bank7 */

#define _BWSCON		bDRAM(0x00)	/* R/W, Bus witdth & wait status control reg. */
#define _BANKCON0	bDRAM(0x04)	/* R/W, Bank 0 control reg. */
#define _BANKCON1	bDRAM(0x08)	/* R/W, Bank 1 control reg. */
#define _BANKCON2	bDRAM(0x0C)	/* R/W, Bank 2 control reg. */
#define _BANKCON3	bDRAM(0x10)	/* R/W, Bank 3 control reg. */
#define _BANKCON4	bDRAM(0x14)	/* R/W, Bank 4 control reg. */
#define _BANKCON5	bDRAM(0x18)	/* R/W, Bank 5 control reg. */
#define _BANKCON6	bDRAM(0x1C)	/* R/W, Bank 6 control reg. */
#define _BANKCON7	bDRAM(0x20)	/* R/W, Bank 7 control reg. */
#define _REFRESH	bDRAM(0x24)	/* R/W, DRAM/SDRAM refresh control reg. */
#define _BANKSIZE	bDRAM(0x28)	/* R/W, Flexible bank size register */
#define _MRSRB6		bDRAM(0x2C)	/* R/W, Mode register set register bank6 */
#define _MRSRB7		bDRAM(0x30)	/* R/W, Mode register set register bank7 */

/*
 * Clock
 */
#define CLOCK_BASE	0x14800000
#define bCLOCK(Nb)	__REG(CLOCK_BASE + (Nb))
#define _LOCKTIME	bCLOCK(0x00)	/* R/W, PLL lock time count reg. */
#define _MPLLCON	bCLOCK(0x04)	/* R/W, MPLL configuration reg. */
#define _UPLLCON	bCLOCK(0x08)	/* R/W, UPLL configruation reg. */
#define _CLKCON		bCLOCK(0x0C)	/* R/W, Clock generator contron reg. */
#define _CLKSLOW	bCLOCK(0x10)	/* R/W, Slow clock control reg. */
#define _CLKDIVN	bCLOCK(0x14)	/* R/W, Clock divider control reg. */
#define S3C2400_LOCKTIME	0x00	/* R/W, PLL lock time count reg. */
#define S3C2400_MPLLCON		0x04	/* R/W, MPLL configuration reg. */
#define S3C2400_UPLLCON		0x08	/* R/W, UPLL configruation reg. */
#define S3C2400_CLKCON		0x0C	/* R/W, Clock generator contron reg. */
#define S3C2400_CLKSLOW		0x10	/* R/W, Slow clock control reg. */
#define S3C2400_CLKDIVN		0x14	/* R/W, Clock divider control reg. */

#define fPLLCON_MDIV	Fld(8,12)       /* Main divider */
#define PLLCON_MDIV	FMsk(fPLLCON_MDIV)
#define sPLLCON_MDIV(x)	FInsrt((x), fPLLCON_MDIV)
#define gPLLCON_MDIV(x)	FExtr((x), fPLLCON_MDIV)

#define fPLLCON_PDIV	Fld(6,4)        /* Pre divider */
#define PLLCON_PDIV	FMsk(fPLLCON_PDIV)
#define sPLLCON_PDIV(x)	FInsrt((x), fPLLCON_PDIV)
#define gPLLCON_PDIV(x)	FExtr((x), fPLLCON_PDIV)

#define fPLLCON_SDIV	Fld(2,0)        /* Post divider */
#define PLLCON_SDIV	FMsk(fPLLCON_SDIV)
#define sPLLCON_SDIV(x)	FInsrt((x), fPLLCON_SDIV)
#define gPLLCON_SDIV(x)	FExtr((x), fPLLCON_SDIV)

#define SPICLK		(1 << 15)       /* PCLK into SPI */
#define SPICLK_ON	(SPICLK*1)
#define SPICLK_OFF	(SPICLK*0)
#define IISCLK		(1 << 14)       /* PCLK into IIS */
#define IISCLK_ON	(IISCLK*1)
#define IISCLK_OFF	(IISCLK*0)
#define IICCLK		(1 << 13)       /* PCLK into IIC */
#define IICCLK_ON	(IICCLK*1)
#define IICCLK_OFF	(IICCLK*0)
#define ADCCLK		(1 << 12)       /* PCLK into ADC */
#define ADCCLK_ON	(ADCCLK*1)
#define ADCCLK_OFF	(ADCCLK*0)
#define RTCCLK		(1 << 11)       /* PCLK into RTC */
#define RTCCLK_ON	(RTCCLK*1)
#define RTCCLK_OFF	(RTCCLK*0)
#define GPIOCLK		(1 << 10)       /* PCLK into GPIO */
#define GPIOCLK_ON	(GPIOCLK*1)
#define GPIOCLK_OFF	(GPIOCLK*0)
#define UART1CLK	(1 << 9)        /* PCLK into UART1 */
#define UART1LCK_ON	(UART1CLK*1)
#define UART1CLK_OFF	(UART1CLK*0)
#define UART0CLK	(1 << 8)        /* PCLK into UART0 */
#define UART0CLK_ON	(UART0CLK*1)
#define UART0CLK_OFF	(UART0CLK*0)
#define MMCCLK		(1 << 7)        /* PCLK into MMC */
#define MMCCLK_ON	(MMCCLK*1)
#define MMCCLK_OFF	(MMCCLK*1)
#define PWMTIMERCLK	(1 << 6)        /* PCLK into PWMTIMER */
#define PWMTIMERCLK_ON	(PWMTIMERCLK*1)
#define PWMTIMERCLK_OFF	(PWMTIMERCLK*0)
#define USBDCLK		(1 << 5)        /* PCLK into USB device */
#define USBDCLK_ON	(USBDCLK*1)
#define USBDCLK_OFF	(USBDCLK*0)
#define USBHCLK		(1 << 4)        /* HCLK into USB Host */
#define USBHCLK_ON	(USBHCLK*1)
#define USBHCLK_OFF	(USBHCLK*0)
#define LCDCCLK		(1 << 3)        /* HCLK into LCDC */
#define LCDCCLK_ON	(LCDCCLK*1)
#define LCDCCLK_OFF	(LCDCCLK*0)
#define IDLE_MODE	(1 << 2)        /* Enter IDLE mode */
#define SL_IDLE_MODE	(1 << 1)        /* Enter SL_IDLE mode (set CLKCON to 0xe) */
#define STOP_MODE	(1 << 0)        /* Enter STOP mode */


/*
 * UART
 */
#define UART_CTL_BASE		0x15000000
#define UART0_CTL_BASE		0x15000000	/* UART channel 0 */
#define UART1_CTL_BASE		0x15004000	/* UART channel 1 */
#define bUART(x, Nb)		__REGl(UART_CTL_BASE + (x)*0x4000 + (Nb))
#define bUARTb(x, Nb)		__REGb(UART_CTL_BASE + (x)*0x4000 + (Nb))
#if 0
#define S3C2400_ULCON		0x00	/* R/W */
#define S3C2400_UCON		0x04	/* R/W */
#define S3C2400_UFCON		0x08	/* R/W */
#define S3C2400_UMCON		0x0C	/* R/W */
#define S3C2400_UTRSTAT		0x10	/* R */
#define S3C2400_UERSTAT		0x14	/* R */
#define S3C2400_UFSTAT		0x18	/* R */
#define S3C2400_UMSTAT		0x1C	/* R */
#define S3C2400_UTXH		0x20	/* W, by byte, little endian */
#define S3C2400_URXH		0x24	/* R, by byte, little endian */
#define S3C2400_UBRDIV		0x28	/* R/W */
#endif
/* Offset */
#define oULCON			0x00	/* R/W */
#define oUCON			0x04	/* R/W */
#define oUFCON			0x08	/* R/W */
#define oUMCON			0x0C	/* R/W */
#define oUTRSTAT		0x10	/* R */
#define oUERSTAT		0x14	/* R */
#define oUFSTAT			0x18	/* R */
#define oUMSTAT			0x1C	/* R */
#define oUTXH			0x20	/* W, by byte, little endian */
#define oURXH			0x24	/* R, by byte, little endian */
#define oUBRDIV			0x28	/* R/W */

#if 0
#define S3C2400_ULCON_5BITS	0x0
#define S3C2400_ULCON_6BITS	0x1
#define S3C2400_ULCON_7BITS	0x2
#define S3C2400_ULCON_8BITS	0x3
#endif

#if 0
#define UCON_TXINT_PULSE	(0 << 9)
#define UCON_TXINT_LEVEL	(1 << 9)
#define UCON_RXINT_PULSE	(0 << 8)
#define UCON_RXINT_LEVEL	(1 << 8)
#define UCON_RX_TIMEOUT_EN	(1 << 7)
#define UCON_ERR_INT_EN		(1 << 6)
#define UCON_LOOPBACK		(1 << 5)
#define UCON_BRKSIG		(1 << 4)
#define UCON_TM_DISABLE		0x0 
#define UCON_TM_INT		0x4 
#define UCON_TM_DMA0		0x8
#define UCON_TM_DMA1		0xC
#define UCON_RM_DISABLE		0x0
#define UCON_RM_INT		0x1
#define UCON_RM_DMA0		0x2
#define UCON_RM_DMA1		0x3

#define UFCON_TX_TRG_EMPTY	0x00
#define UFCON_TX_TRG_4BYTE	0x40
#define UFCON_TX_TRG_8BYTE	0x80
#define UFCON_TX_TRG_12BYTE	0xC0
#define UFCON_RX_TRG_4BYTE	0x00
#define UFCON_RX_TRG_8BYTE	0x10
#define UFCON_RX_TRG_12BYTE	0x20
#define UFCON_RX_TRG_16BYTE	0x30
#define UFCON_FIFO_EN		0x1

#define UNCON_AFC_EN		(1 << 4)
#define UFCON_RQT_SEND		0x1

#define UTRSTAT_TX_EMPTY	(1 << 2)
#define UTRSTAT_TX_BUF_EMPTY	(1 << 1)
#define UTRSTAT_RX_READY	(1 << 0)
#define S3C2400_UTRSTAT_ERROR_MASK	0xF
#endif

#if 0
#define UART0BASE	0x15000000	/* UART channel 0 */
#define UART1BASE	0x15004000	/* UART channel 1 */
#define bUART(Nb,x)     (0x15000000 + (Nb)*0x4000 + (x))
#endif
#define ULCON0          bUART(0,0x00)
#define UCON0           bUART(0,0x04)
#define UFCON0          bUART(0,0x08)
#define UMCON0          bUART(0,0x0c)
#define UTRSTAT0        bUART(0,0x10)
#define UERSTAT0        bUART(0,0x14)
#define UFSTAT0         bUART(0,0x18)
#define UMSTAT0         bUART(0,0x1c)
#define UTXH0           bUARTb(0,0x20)
#define URXH0           bUARTb(0,0x24)
#define UBRDIV0         bUART(0,0x28)
#define ULCON1          bUART(1,0x00)
#define UCON1           bUART(1,0x04)
#define UFCON1          bUART(1,0x08)
#define UMCON1          bUART(1,0x0c)
#define UTRSTAT1        bUART(1,0x10)
#define UERSTAT1        bUART(1,0x14)
#define UFSTAT1         bUART(1,0x18)
#define UMSTAT1         bUART(1,0x1c)
#define UTXH1           bUARTb(1,0x20)
#define URXH1           bUARTb(1,0x24)
#define UBRDIV1         bUART(1,0x28)
/* Status */
#if 0
#define UTRSTAT_TX_EMPTY	(1 << 2)
#define UTRSTAT_TX_BUF_EMPTY	(1 << 1)
#define UTRSTAT_RX_READY	(1 << 0)
#define S3C2400_UTRSTAT_ERROR_MASK	0xF
#endif
#define UTRSTAT_TX_EMPTY	(1 << 2)
#define UTRSTAT_RX_READY	(1 << 0)
#define UART_ERROR_MASK		0xF

/*
 * Interrupt Controller
 */
#define S3C2400_INT_CFG_BASE	0x14400000
#define bINT(Nb)	__REG(S3C2400_INT_CFG_BASE + (Nb))
#define S3C2400_SRCPND		0x00	/* R/W, Indicates the interrupt request status */
#define S3C2400_INTMOD		0x04	/* W, Intterupt mode reg. 0=IRQ, 1=FIQ */	
#define S3C2400_INTMSK		0x08	/* R/W, Intterupt mask */
#define S3C2400_PRIORITY	0x0C	/* W, IRQ priority control reg. */
#define S3C2400_INTPND		0x10	/* R/W, Indicates the irq rqt status */
#define S3C2400_INTOFFSET	0x14	/* R,  */

#define _SRCPND		bINT(0x00)	/* R/W, Indicates the interrupt request status */
#define _INTMOD		bINT(0x04)	/* W, Intterupt mode reg. 0=IRQ, 1=FIQ */	
#define _INTMSK		bINT(0x08)	/* R/W, Intterupt mask */
#define _PRIORITY	bINT(0x0C)	/* W, IRQ priority control reg. */
#define _INTPND		bINT(0x10)	/* R/W, Indicates the irq rqt status */
#define _INTOFFSET	bINT(0x14)	/* R,  */

#define IRQ_ADC		(1 << 31) /* ADC EOC interrupt        */
#define IRQ_RTC		(1 << 30) /* RTC alarm interrupt      */
#define IRQ_UTXD1	(1 << 29) /* UART1 transmit interrupt */
#define IRQ_UTXD0	(1 << 28) /* UART0 transmit interrupt */
#define IRQ_IIC		(1 << 27) /* IIC interrupt        */
#define IRQ_USBH	(1 << 26) /* USB host interrupt       */
#define IRQ_USBD	(1 << 25) /* USB device interrupt     */
#define IRQ_URXD1	(1 << 24) /* UART1 receive interrupt  */
#define IRQ_URXD0	(1 << 23) /* UART0 receive interrupt  */
#define IRQ_SPI		(1 << 22) /* SPI interrupt        */
#define IRQ_MMC		(1 << 21) /* MMC interrupt        */
#define IRQ_DMA3	(1 << 20) /* DMA channel 3 interrupt  */
#define IRQ_DMA2	(1 << 19) /* DMA channel 2 interrupt  */
#define IRQ_DMA1	(1 << 18) /* DMA channel 1 interrupt  */
#define IRQ_DMA0	(1 << 17) /* DMA channel 0 interrupt  */
#define IRQ_RESERVED	(1 << 16) /* reserved for future use  */
#define IRQ_UERR01	(1 << 15) /* UART 0/1 interrupt       */
#define IRQ_TIMER4	(1 << 14) /* Timer 4 interrupt        */
#define IRQ_TIMER3	(1 << 13) /* Timer 3 interrupt        */
#define IRQ_TIMER2	(1 << 12) /* Timer 2 interrupt        */
#define IRQ_TIMER1	(1 << 11) /* Timer 1 interrupt        */
#define IRQ_TIMER0	(1 << 10) /* Timer 0 interrupt        */
#define IRQ_WDT		(1 <<  9) /* Watch-Dog timer interrupt    */
#define IRQ_TICK	(1 <<  8) /* RTC time tick interrupt  */
#define IRQ_EINT7	(1 <<  7) /* External interrupt 7     */
#define IRQ_EINT6	(1 <<  6) /* External interrupt 6     */
#define IRQ_EINT5	(1 <<  5) /* External interrupt 5     */
#define IRQ_EINT4	(1 <<  4) /* External interrupt 4     */
#define IRQ_EINT3	(1 <<  3) /* External interrupt 3     */
#define IRQ_EINT2	(1 <<  2) /* External interrupt 2     */
#define IRQ_EINT1	(1 <<  1) /* External interrupt 1     */
#define IRQ_EINT0	(1 <<  0) /* External interrupt 0     */
/*
 * Watch-dog timer
 */
#define S3C2400_WTCON		0x15300000	/* R/W, Watch-dog timer control reg. */
#define S3C2400_WTDAT		0x15300004	/* R/W, Watch-dog timer data reg. */
#define S3C2400_WTCNT		0x15300008	/* R/W, Watch-dog tiemr count value for reload */

/*
 * I/O Ports
 */
#define S3C2400_IOPORT_BASE_ADDR	0x15600000
#define bIOPORT(Nb)	__REG(S3C2400_IOPORT_BASE_ADDR + (Nb))
#define S3C2400_PACON		0x00	/* R/W, Configures the pins of port A */
#define S3C2400_PADAT		0x04	/* R/W, Data register for port A */
#define S3C2400_PBCON		0x08	/* R/W, Configures the pins of port B */
#define S3C2400_PBDAT		0x0c	/* R/W, Data register for port B */
#define S3C2400_PBUP		0x10	/* R/W, pull-up disable register for port B */
#define S3C2400_PCCON		0x14	/* R/W, configures the pins of port C */
#define S3C2400_PCDAT		0x18	/* R/W, data register for port C */
#define S3C2400_PCUP		0x1c	/* R/W, pull-up disable register for port C */
#define S3C2400_PDCON		0x20	/* R/W, configure the pins of port D */
#define S3C2400_PDDAT		0x24	/* R/W, data register for port D */
#define S3C2400_PDUP		0x28	/* R/W, pull-up disable register for port D */
#define S3C2400_PECON		0x2c	/* R/W, configures the pins of port E */
#define S3C2400_PEDAT		0x30	/* R/W, data register for port E */
#define S3C2400_PEUP		0x34	/* R/W, pull-up disable register for port E */
#define S3C2400_PFCON		0x38	/* R/W, configure the pins of port F */
#define S3C2400_PFDAT		0x3c	/* R/W, data register for port F */
#define S3C2400_PFUP		0x40	/* R/W, pull-up disable register for port F */
#define S3C2400_PGCON		0x44	/* R/W, configures the pins of port G */
#define S3C2400_PGDAT		0x48	/* R/W, data register for port G */
#define S3C2400_PGUP		0x4c	/* R/W, pull-up disable register for port G */
#define S3C2400_OPENCR		0x50	/* R/W, open-drain enable register */
#define S3C2400_MISCCR		0x54	/* R/W, miscellaneous control register */
#define S3C2400_EXTINT		0x58	/* R/W, external interrupt control register */

#define _PACON		bIOPORT(0x00)	/* R/W, Configures the pins of port A */
#define _PADAT		bIOPORT(0x04)	/* R/W, Data register for port A */
#define _PBCON		bIOPORT(0x08)	/* R/W, Configures the pins of port B */
#define _PBDAT		bIOPORT(0x0c)	/* R/W, Data register for port B */
#define _PBUP		bIOPORT(0x10)	/* R/W, pull-up disable register for port B */
#define _PCCON		bIOPORT(0x14)	/* R/W, configures the pins of port C */
#define _PCDAT		bIOPORT(0x18)	/* R/W, data register for port C */
#define _PCUP		bIOPORT(0x1c)	/* R/W, pull-up disable register for port C */
#define _PDCON		bIOPORT(0x20)	/* R/W, configure the pins of port D */
#define _PDDAT		bIOPORT(0x24)	/* R/W, data register for port D */
#define _PDUP		bIOPORT(0x28)	/* R/W, pull-up disable register for port D */
#define _PECON		bIOPORT(0x2c)	/* R/W, configures the pins of port E */
#define _PEDAT		bIOPORT(0x30)	/* R/W, data register for port E */
#define _PEUP		bIOPORT(0x34)	/* R/W, pull-up disable register for port E */
#define _PFCON		bIOPORT(0x38)	/* R/W, configure the pins of port F */
#define _PFDAT		bIOPORT(0x3c)	/* R/W, data register for port F */
#define _PFUP		bIOPORT(0x40)	/* R/W, pull-up disable register for port F */
#define _PGCON		bIOPORT(0x44)	/* R/W, configures the pins of port G */
#define _PGDAT		bIOPORT(0x48)	/* R/W, data register for port G */
#define _PGUP		bIOPORT(0x4c)	/* R/W, pull-up disable register for port G */
#define _OPENCR		bIOPORT(0x50)	/* R/W, open-drain enable register */
#define _MISCCR		bIOPORT(0x54)	/* R/W, miscellaneous control register */
#define _EXTINT		bIOPORT(0x58)	/* R/W, external interrupt control register */

/*
 * Real Time Clock (RTC)
 */
#define _RTCCON		(*(volatile unsigned char *)0x15700040)
#define _TICNT		(*(volatile unsigned char *)0x15700044)
#define _RTCALM		(*(volatile unsigned char *)0x15700050)
#define _BCDSEC		(*(volatile unsigned char *)0x15700070)
#define _BCDMIN		(*(volatile unsigned char *)0x15700074)
#define _BCDHOUR	(*(volatile unsigned char *)0x15700078)
#define _BCDDAY		(*(volatile unsigned char *)0x1570007C)
#define _BCDDATE	(*(volatile unsigned char *)0x15700080)
#define _BCDMON		(*(volatile unsigned char *)0x15700084)
#define _BCDYEAR	(*(volatile unsigned char *)0x15700088)

/*
 * Timer
 */
#define _TCFG0		(*(volatile unsigned long *)0x15100000)
#define _TCON		(*(volatile unsigned long *)0x15100008)
#define _TCNTB4		(*(volatile unsigned long *)0x1510003C)
#define _TCNTO4		(*(volatile unsigned long *)0x15100040)

