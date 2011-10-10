/*
 * vivi/include/s3c2400.h
 *
 * Definition of constants related to the S3C2410 microprocessor(based on ARM 290T).
 * This file is based on the S3C2400 User Manual 2002,01,23.
 *
 * Copyright (C) 2002 MIZI Research, Inc.
 *
 *  Author: Janghoon Lyu <nandy@mizi.com>
 *  Date  : $Date: 2002/10/11 02:27:29 $
 *
 *  $Revision: 1.8 $
 */

/*
 * History
 * 
 * 2002-05-14: Janghoon Lyu <nandy@mizi.com>
 *    - Initial code
 */

#include "hardware.h"
#include "bitfield.h"

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

/* Memory Controller */
#define MEM_CTL_BASE		0x48000000
#define bMEMCTL(Nb)		__REGl(MEM_CTL_BASE + (Nb))
/* Offset */
#define oBWSCON			0x00	/* R/W, Bus width and wait status ctrl reg. */
#define oBANKCON0		0x04	/* R/W, Bank 0 control reg. */
#define oBANKCON1		0x08	/* R/W, Bank 1 control reg. */
#define oBANKCON2		0x0C	/* R/W, Bank 2 control reg. */
#define oBANKCON3		0x10	/* R/W, Bank 3 control reg. */
#define oBANKCON4		0x14	/* R/W, Bank 4 control reg. */
#define oBANKCON5		0x18	/* R/W, Bank 5 control reg. */
#define oBANKCON6		0x1C	/* R/W, Bank 6 control reg. */
#define oBANKCON7		0x20	/* R/W, Bank 7 control reg. */
#define oREFRESH		0x24	/* R/W, SDRAM refresh control register */
#define oBANKSIZE		0x28	/* R/W, Flexible bank size register */
#define oMRSRB6			0x2C	/* R/W, Mode register set register bank 6 */
#define oMRSRB7			0x2C	/* R/W, Mode register set register bank 7 */
/* Registers */
#define BWSCON			bMEMCTL(oBWSCON)
#define BANKCON0		bMEMCTL(oBANKCON0)
#define BANKCON1		bMEMCTL(oBANKCON1)
#define BANKCON2		bMEMCTL(oBANKCON2)
#define BANKCON3		bMEMCTL(oBANKCON3)
#define BANKCON4		bMEMCTL(oBANKCON4)
#define BANKCON5		bMEMCTL(oBANKCON5)
#define BANKCON6		bMEMCTL(oBANKCON6)
#define BANKCON7		bMEMCTL(oBANKCON7)
#define REFRESH			bMEMCTL(oREFRESH)
#define BANKSIZE		bMEMCTL(oBANKSIZE)
#define MRSRB6			bMEMCTL(oMRSRB6)
#define MRSRB7			bMEMCTL(oMRSRB7)
/* Bits */
#define SELF_REFRESH		(1 << 22)

/* Clock and Power Management */
#define CLK_CTL_BASE		0x4C000000
#define bCLKCTL(Nb)		__REGl(CLK_CTL_BASE + (Nb))
/* Offset */
#define oLOCKTIME		0x00	/* R/W, PLL lock time count register */
#define oMPLLCON		0x04	/* R/W, MPLL configuration register */
#define oUPLLCON		0x08	/* R/W, UPLL configuration register */
#define oCLKCON			0x0C	/* R/W, Clock generator control reg. */
#define oCLKSLOW		0x10	/* R/W, Slow clock control register */
#define oCLKDIVN		0x14	/* R/W, Clock divider control */
/* Registers */
#define LOCKTIME		bCLKCTL(oLOCKTIME)
#define MPLLCON			bCLKCTL(oMPLLCON)
#define UPLLCON			bCLKCTL(oUPLLCON)
#define CLKCON			bCLKCTL(oCLKCON)
#define CLKSLOW			bCLKCTL(oCLKSLOW)
#define CLKDIVN			bCLKCTL(oCLKDIVN)
/* Fields */
#define fMPLL_MDIV		Fld(8,12)
#define fMPLL_PDIV		Fld(6,4)
#define fMPLL_SDIV		Fld(2,0)
/* macros */
#define GET_MDIV(x)		FExtr(x, fMPLL_MDIV)
#define GET_PDIV(x)		FExtr(x, fMPLL_PDIV)
#define GET_SDIV(x)		FExtr(x, fMPLL_SDIV)

/* GPIO */
#define GPIO_CTL_BASE		0x56000000
#define bGPIO(p,o)		__REGl(GPIO_CTL_BASE + (p) + (o))
/* Offset */
#define oGPIO_CON		0x0	/* R/W, Configures the pins of the port */
#define oGPIO_DAT		0x4	/* R/W,	Data register for port */
#define oGPIO_UP		0x8	/* R/W, Pull-up disable register */
#define oGPIO_RESERVED		0xC	/* R/W, Reserved */
#define oGPIO_A			0x00
#define oGPIO_B			0x10
#define oGPIO_C			0x20
#define oGPIO_D			0x30
#define oGPIO_E			0x40
#define oGPIO_F			0x50
#define oGPIO_G			0x60
#define oGPIO_H			0x70
#define oMISCCR			0x80	/* R/W, Miscellaneous control register */
#define oDCLKCON		0x84	/* R/W, DCLK0/1 control register */
#define oEXTINT0		0x88	/* R/W, External interrupt control reg. 0 */
#define oEXTINT1		0x8C	/* R/W, External interrupt control reg. 1 */
#define oEXTINT2		0x90	/* R/W, External interrupt control reg. 2 */
#define oEINTFLT0		0x94	/* R/W, Reserved */
#define oEINTFLT1		0x98	/* R/W, Reserved */
#define oEINTFLT2		0x9C	/* R/W, External interrupt control reg. 2 */
#define oEINTFLT3		0xA0	/* R/W, External interrupt control reg. 3 */
#define oEINTMASK		0xA4	/* R/W, External interrupt mask register */
#define oEINTPEND		0xA8	/* R/W, External interrupt pending reg. */
/* Registers */
#define GPACON			bGPIO(oGPIO_A, oGPIO_CON)
#define GPADAT			bGPIO(oGPIO_A, oGPIO_DAT)
#define GPBCON			bGPIO(oGPIO_B, oGPIO_CON)
#define GPBDAT			bGPIO(oGPIO_B, oGPIO_DAT)
#define GPBUP			bGPIO(oGPIO_B, oGPIO_UP)
#define GPCCON			bGPIO(oGPIO_C, oGPIO_CON)
#define GPCDAT			bGPIO(oGPIO_C, oGPIO_DAT)
#define GPCUP			bGPIO(oGPIO_C, oGPIO_UP)
#define GPDCON			bGPIO(oGPIO_D, oGPIO_CON)
#define GPDDAT			bGPIO(oGPIO_D, oGPIO_DAT)
#define GPDUP			bGPIO(oGPIO_D, oGPIO_UP)
#define GPECON			bGPIO(oGPIO_E, oGPIO_CON)
#define GPEDAT			bGPIO(oGPIO_E, oGPIO_DAT)
#define GPEUP			bGPIO(oGPIO_E, oGPIO_UP)
#define GPFCON			bGPIO(oGPIO_F, oGPIO_CON)
#define GPFDAT			bGPIO(oGPIO_F, oGPIO_DAT)
#define GPFUP			bGPIO(oGPIO_F, oGPIO_UP)
#define GPGCON			bGPIO(oGPIO_G, oGPIO_CON)
#define GPGDAT			bGPIO(oGPIO_G, oGPIO_DAT)
#define GPGUP			bGPIO(oGPIO_G, oGPIO_UP)
#define GPHCON			bGPIO(oGPIO_H, oGPIO_CON)
#define GPHDAT			bGPIO(oGPIO_H, oGPIO_DAT)
#define GPHUP			bGPIO(oGPIO_H, oGPIO_UP)
#define MISCCR			bGPIO(oMISCCR, 0)
#define DCLKCON			bGPIO(oDCLKCON, 0)
#define EXTINT0			bGPIO(oEXTINT0, 0)
#define EXTINT1			bGPIO(oEXTINT1, 0)
#define EXTINT2			bGPIO(oEXTINT2, 0)
#define EINTFLT0		bGPIO(oEINTFLT0, 0)
#define EINTFLT1		bGPIO(oEINTFLT1, 0)
#define EINTFLT2		bGPIO(oEINTFLT2, 0)
#define EINTFLT3		bGPIO(oEINTFLT3, 0)
#define EINTMASK		bGPIO(oEINTMASK, 0)
#define EINTPEND		bGPIO(oEINTPEND, 0)

/* UART */
#define UART_CTL_BASE		0x50000000
#define UART0_CTL_BASE		UART_CTL_BASE
#define UART1_CTL_BASE		UART_CTL_BASE + 0x4000
#define UART2_CTL_BASE		UART_CTL_BASE + 0x8000
#define bUART(x, Nb)		__REGl(UART_CTL_BASE + (x)*0x4000 + (Nb))
#define bUARTb(x, Nb)		__REGb(UART_CTL_BASE + (x)*0x4000 + (Nb))
/* Offset */
#define oULCON			0x00	/* R/W, UART line control register */
#define oUCON			0x04	/* R/W, UART control register */
#define oUFCON			0x08	/* R/W, UART FIFO control register */
#define oUMCON			0x0C	/* R/W, UART modem control register */
#define oUTRSTAT		0x10	/* R  , UART Tx/Rx status register */
#define oUERSTAT		0x14	/* R  , UART Rx error status register */
#define oUFSTAT			0x18	/* R  , UART FIFO status register */
#define oUMSTAT			0x1C	/* R  , UART Modem status register */
#define oUTXHL			0x20	/*   W, UART transmit(little-end) buffer */
#define oUTXHB			0x23	/*   W, UART transmit(big-end) buffer */
#define oURXHL			0x24	/* R  , UART receive(little-end) buffer */
#define oURXHB			0x27	/* R  , UART receive(big-end) buffer */
#define oUBRDIV			0x28	/* R/W, Baud rate divisor register */
/* Registers */
#define ULCON0			bUART(0, oULCON)
#define UCON0			bUART(0, oUCON)
#define UFCON0			bUART(0, oUFCON)
#define UMCON0			bUART(0, oUMCON)
#define UTRSTAT0		bUART(0, oUTRSTAT)
#define UERSTAT0		bUART(0, oUERSTAT)
#define UFSTAT0			bUART(0, oUFSTAT)
#define UMSTAT0			bUART(0, oUMSTAT)
#define UTXH0			bUARTb(0, oUTXHL)
#define URXH0			bUARTb(0, oURXHL)
#define UBRDIV0			bUART(0, oUBRDIV)
#define ULCON1			bUART(1, oULCON)
#define UCON1			bUART(1, oUCON)
#define UFCON1			bUART(1, oUFCON)
#define UMCON1			bUART(1, oUMCON)
#define UTRSTAT1		bUART(1, oUTRSTAT)
#define UERSTAT1		bUART(1, oUERSTAT)
#define UFSTAT1			bUART(1, oUFSTAT)
#define UMSTAT1			bUART(1, oUMSTAT)
#define UTXH1			bUARTb(1, oUTXHL)
#define URXH1			bUARTb(1, oURXHL)
#define UBRDIV1			bUART(1, oUBRDIV)
#define ULCON2			bUART(2, oULCON)
#define UCON2			bUART(2, oUCON)
#define UFCON2			bUART(2, oUFCON)
#define UMCON2			bUART(2, oUMCON)
#define UTRSTAT2		bUART(2, oUTRSTAT)
#define UERSTAT2		bUART(2, oUERSTAT)
#define UFSTAT2			bUART(2, oUFSTAT)
#define UMSTAT2			bUART(2, oUMSTAT)
#define UTXH2			bUARTb(2, oUTXHL)
#define URXH2			bUARTb(2, oURXHL)
#define UBRDIV2			bUART(2, oUBRDIV)
/* ... */
#define UTRSTAT_TX_EMPTY	(1 << 2)
#define UTRSTAT_RX_READY	(1 << 0)
#define UART_ERR_MASK		0xF 

/* Interrupts */
#define INT_CTL_BASE		0x4A000000
#define bINT_CTL(Nb)		__REG(INT_CTL_BASE + (Nb))
/* Offset */
#define oSRCPND			0x00
#define oINTMOD			0x04
#define oINTMSK			0x08
#define oPRIORITY		0x0a
#define oINTPND			0x10
#define oINTOFFSET		0x14
#define oSUBSRCPND		0x18
#define oINTSUBMSK		0x1C
/* Registers */
#define SRCPND			bINT_CTL(oSRCPND)
#define INTMOD			bINT_CTL(oINTMOD)
#define INTMSK			bINT_CTL(oINTMSK)
#define PRIORITY		bINT_CTL(oPRIORITY)
#define INTPND			bINT_CTL(oINTPND)
#define INTOFFSET		bINT_CTL(oINTOFFSET)
#define SUBSRCPND		bINT_CTL(oSUBSRCPND)
#define INTSUBMSK		bINT_CTL(oINTSUBMSK)

#define INT_ADCTC		(1 << 31)	/* ADC EOC interrupt */
#define INT_RTC			(1 << 30)	/* RTC alarm interrupt */
#define INT_SPI1		(1 << 29)	/* UART1 transmit interrupt */
#define INT_UART0		(1 << 28)	/* UART0 transmit interrupt */
#define INT_IIC			(1 << 27)	/* IIC interrupt */
#define INT_USBH		(1 << 26)	/* USB host interrupt */
#define INT_USBD		(1 << 25)	/* USB device interrupt */
#define INT_RESERVED24		(1 << 24)
#define INT_UART1		(1 << 23)	/* UART1 receive interrupt */
#define INT_SPI0		(1 << 22)	/* SPI interrupt */
#define INT_MMC			(1 << 21)	/* MMC interrupt */
#define INT_DMA3		(1 << 20)	/* DMA channel 3 interrupt */
#define INT_DMA2		(1 << 19)	/* DMA channel 2 interrupt */
#define INT_DMA1		(1 << 18)	/* DMA channel 1 interrupt */
#define INT_DMA0		(1 << 17)	/* DMA channel 0 interrupt */
#define INT_LCD			(1 << 16)	/* reserved for future use */
#define INT_UART2		(1 << 15)	/* UART 2 interrupt  */
#define INT_TIMER4		(1 << 14)	/* Timer 4 interrupt */
#define INT_TIMER3		(1 << 13)	/* Timer 3 interrupt */
#define INT_TIMER2		(1 << 12)	/* Timer 2 interrupt */
#define INT_TIMER1		(1 << 11)	/* Timer 1 interrupt */
#define INT_TIMER0		(1 << 10)	/* Timer 0 interrupt */
#define INT_WDT			(1 << 9)	/* Watch-Dog timer interrupt */
#define INT_TICK		(1 << 8)	/* RTC time tick interrupt  */
#define INT_BAT_FLT		(1 << 7)
#define INT_RESERVED6		(1 << 6)	/* Reserved for future use */
#define INT_EINT8_23		(1 << 5)	/* External interrupt 8 ~ 23 */
#define INT_EINT4_7		(1 << 4)	/* External interrupt 4 ~ 7 */
#define INT_EINT3		(1 << 3)	/* External interrupt 3 */
#define INT_EINT2		(1 << 2)	/* External interrupt 2 */
#define INT_EINT1		(1 << 1)	/* External interrupt 1 */
#define INT_EINT0		(1 << 0)	/* External interrupt 0 */

#define INT_ADC			(1 << 10)
#define INT_TC			(1 << 9)
#define INT_ERR2		(1 << 8)
#define INT_TXD2		(1 << 7)
#define INT_RXD2		(1 << 6)
#define INT_ERR1		(1 << 5)
#define INT_TXD1		(1 << 4)
#define INT_RXD1		(1 << 3)
#define INT_ERR0		(1 << 2)
#define INT_TXD0		(1 << 1)
#define INT_RXD0		(1 << 0)

/* NAND Flash Controller */
#define NAND_CTL_BASE		0x4E000000
#define bINT_CTL(Nb)		__REG(INT_CTL_BASE + (Nb))
/* Offset */
#define oNFCONF			0x00
#define oNFCMD			0x04
#define oNFADDR			0x08
#define oNFDATA			0x0c
#define oNFSTAT			0x10
#define oNFECC			0x14


/* PWM Timer */
#define bPWM_TIMER(Nb)          __REG(0x51000000 + (Nb))
#define bPWM_BUFn(Nb,x)         bPWM_TIMER(0x0c + (Nb)*0x0c + (x))
/* Registers */
#define TCFG0                   bPWM_TIMER(0x00)
#define TCFG1                   bPWM_TIMER(0x04)
#define TCON                    bPWM_TIMER(0x08)
#define TCNTB0                  bPWM_BUFn(0,0x0)
#define TCMPB0                  bPWM_BUFn(0,0x4)
#define TCNTO0                  bPWM_BUFn(0,0x8)
#define TCNTB1                  bPWM_BUFn(1,0x0)
#define TCMPB1                  bPWM_BUFn(1,0x4)
#define TCNTO1                  bPWM_BUFn(1,0x8)
#define TCNTB2                  bPWM_BUFn(2,0x0)
#define TCMPB2                  bPWM_BUFn(2,0x4)
#define TCNTO2                  bPWM_BUFn(2,0x8)
#define TCNTB3                  bPWM_BUFn(3,0x0)
#define TCMPB3                  bPWM_BUFn(3,0x4)
#define TCNTO3                  bPWM_BUFn(3,0x8)
#define TCNTB4                  bPWM_BUFn(4,0x0)
#define TCNTO4                  bPWM_BUFn(4,0x4)
/* Fields */
#define fTCFG0_DZONE            Fld(8,16)       /* the dead zone length (= timer 0) */
#define fTCFG0_PRE1             Fld(8,8)        /* prescaler value for time 2,3,4 */
#define fTCFG0_PRE0             Fld(8,0)        /* prescaler value for time 0,1 */
#define fTCFG1_MUX4		Fld(4,16)
/* bits */
#define TCFG0_DZONE(x)          FInsrt((x), fTCFG0_DZONE)
#define TCFG0_PRE1(x)           FInsrt((x), fTCFG0_PRE1)
#define TCFG0_PRE0(x)           FInsrt((x), fTCFG0_PRE0)
#define TCON_4_AUTO             (1 << 22)       /* auto reload on/off for Timer 4 */
#define TCON_4_UPDATE           (1 << 21)       /* manual Update TCNTB4 */
#define TCON_4_ONOFF            (1 << 20)       /* 0: Stop, 1: start Timer 4 */
#define COUNT_4_ON              (TCON_4_ONOFF*1)
#define COUNT_4_OFF             (TCON_4_ONOFF*0)
#define TCON_3_AUTO     (1 << 19)       /* auto reload on/off for Timer 3 */
#define TIMER3_ATLOAD_ON        (TCON_3_AUTO*1)
#define TIMER3_ATLAOD_OFF       FClrBit(TCON, TCON_3_AUTO)
#define TCON_3_INVERT   (1 << 18)       /* 1: Inverter on for TOUT3 */
#define TIMER3_IVT_ON   (TCON_3_INVERT*1)
#define TIMER3_IVT_OFF  (FClrBit(TCON, TCON_3_INVERT))
#define TCON_3_MAN      (1 << 17)       /* manual Update TCNTB3,TCMPB3 */
#define TIMER3_MANUP    (TCON_3_MAN*1)
#define TIMER3_NOP      (FClrBit(TCON, TCON_3_MAN))
#define TCON_3_ONOFF    (1 << 16)       /* 0: Stop, 1: start Timer 3 */
#define TIMER3_ON       (TCON_3_ONOFF*1)
#define TIMER3_OFF      (FClrBit(TCON, TCON_3_ONOFF))
/* macros */
#define GET_PRESCALE_TIMER4(x)	FExtr((x), fTCFG0_PRE1)
#define GET_DIVIDER_TIMER4(x)	FExtr((x), fTCFG1_MUX4)

/*
 * NAND Flash Controller (Page 6-1 ~ 6-8)
 *
 * Register
   NFCONF   NAND Flash Configuration    [word, R/W, 0x00000000]
   NFCMD    NAND Flash Command Set      [word, R/W, 0x00000000]
   NFADDR   NAND Flash Address Set      [word, R/W, 0x00000000]
   NFDATA   NAND Flash Data             [word, R/W, 0x00000000]
   NFSTAT   NAND Flash Status           [word, R, 0x00000000]
   NFECC    NAND Flash ECC              [3 bytes, R, 0x00000000]
 *
 */
#define bNAND_CTL(Nb)   __REG(0x4e000000 + (Nb))
#define NFCONF          bNAND_CTL(0x00)
#define NFCMD       bNAND_CTL(0x04)
#define NFADDR      bNAND_CTL(0x08)
#define NFDATA      bNAND_CTL(0x0c)
#define NFSTAT      bNAND_CTL(0x10)
#define NFECC       bNAND_CTL(0x14)

#define fNFCONF_TWRPH1   Fld(3,0)
#define NFCONF_TWRPH1    FMsk(fNFCONF_TWRPH1)
#define NFCONF_TWRPH1_0  FInsrt(0x0, fNFCONF_TWRPH1) /* 0 */
#define fNFCONF_TWRPH0   Fld(3,4)
#define NFCONF_TWRPH0    FMsk(fNFCONF_TWRPH0)
#define NFCONF_TWRPH0_3  FInsrt(0x3, fNFCONF_TWRPH0) /* 3 */
#define fNFCONF_TACLS    Fld(3,8)
#define NFCONF_TACLS     FMsk(fNFCONF_TACLS)
#define NFCONF_TACLS_0   FInsrt(0x0, fNFCONF_TACLS) /* 0 */
#define fNFCONF_nFCE     Fld(1,11)
#define NFCONF_nFCE      FMsk(fNFCONF_nFCE)
#define NFCONF_nFCE_LOW  FInsrt(0x0, fNFCONF_nFCE) /* active */
#define NFCONF_nFCE_HIGH FInsrt(0x1, fNFCONF_nFCE) /* inactive */
#define fNFCONF_ECC      Fld(1,12)
#define NFCONF_ECC       FMsk(fNFCONF_ECC)
#define NFCONF_ECC_NINIT FInsrt(0x0, fNFCONF_ECC) /* not initialize */
#define NFCONF_ECC_INIT  FInsrt(0x1, fNFCONF_ECC)    /* initialize */
#define fNFCONF_ADDRSTEP Fld(1,13)                 /* Addressing Step */
#define NFCONF_ADDRSTEP  FMsk(fNFCONF_ADDRSTEP)
#define fNFCONF_PAGESIZE Fld(1,14)
#define NFCONF_PAGESIZE  FMsk(fNFCONF_PAGESIZE)
#define NFCONF_PAGESIZE_256  FInsrt(0x0, fNFCONF_PAGESIZE) /* 256 bytes */
#define NFCONF_PAGESIZE_512  FInsrt(0x1, fNFCONF_PAGESIZE) /* 512 bytes */
#define fNFCONF_FCTRL    Fld(1,15)  /* Flash controller enable/disable */
#define NFCONF_FCTRL     FMsk(fNFCONF_FCTRL)
#define NFCONF_FCTRL_DIS FInsrt(0x0, fNFCONF_FCTRL) /* Disable */
#define NFCONF_FCTRL_EN  FInsrt(0x1, fNFCONF_FCTRL) /* Enable */

#define NFSTAT_RnB      (1 << 0)
#define NFSTAT_nFWE     (1 << 8)
#define NFSTAT_nFRE     (1 << 9)
#define NFSTAT_ALE      (1 << 10)
#define NFSTAT_CLE      (1 << 11)
#define NFSTAT_AUTOBOOT (1 << 15)


/*
 * Power Management
 */
#define SPI_CLK		(1 << 18)
#define IIS_CLK		(1 << 17)
#define IIC_CLK		(1 << 16
#define ADC_CLK		(1 << 15)
#define RTC_CLK		(1 << 14)
#define GPIO_CLK	(1 << 13)
#define UART2_CLK	(1 << 12)
#define UART1_CLK	(1 << 11)
#define UART0_CLK	(1 << 10)
#define SDI_CLK		(1 << 9)
#define PWM_CLK		(1 << 8)
#define USBSLAVE_CLK	(1 << 7)
#define USBHOST_CLK	(1 << 6)
#define LCDC_CLK	(1 << 5)
#define NANDCTL_CLK	(1 << 4)
#define SLEEP_ON	(1 << 3)
#define IDLE		(1 << 2)

#define GSTATUS(Nb)	__REG(0x560000AC + (Nb*4))
#define GSTATUS0	GSTATUS(0)
#define GSTATUS1	GSTATUS(1)
#define GSTATUS2	GSTATUS(2)
#define GSTATUS3	GSTATUS(3)
#define GSTATUS4	GSTATUS(4)
#define PMST		GSTATUS2
#define PMSR0		GSTATUS3
#define PMSR1		GSTATUS4
#define PMCTL0		CLKCON
#define PMCTL1		MISCCR
#define SCLKE		(1 << 19)
#define SCLK1		(1 << 18)
#define SCLK0		(1 << 17)
#define USBSPD1		(1 << 13)
#define USBSPD0		(1 << 12)
#define PMST_HWR	(1 << 0)
#define PMST_SMR	(1 << 1)
#define PMST_WDR	(1 << 2)

/*
 * Watch-dog tiemr
 */
#define WTCON		__REG(0x53000000)
#define WTDAT		__REG(0x53000004)
#define WTCNT		__REG(0x53000008)

