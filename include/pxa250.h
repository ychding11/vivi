/*
 * pxa250.h
	from kernel/include/asm-arm/arch-pxa/pxa-regs.h
		kernel/include/asm-arm/proc-armv/ptrace.h
 *
 * Copyright (C) 2001 MIZI Research, Inc.
 *
 * Author: Yong-iL Joh <tolkien@mizi.com>
 * Date  : $Date: 2002/09/03 07:42:49 $ 
 *
 * $Revision: 1.6 $
 *
   Mon May 23 2002 Yong-iL Joh <tolkien@mizi.com>
   - initial
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#ifndef _PXA250_H_
#define _PXA250_H_

#include "hardware.h"

#define USR26_MODE	0x00
#define FIQ26_MODE	0x01
#define IRQ26_MODE	0x02
#define SVC26_MODE	0x03
#define USR_MODE	0x10
#define FIQ_MODE	0x11
#define IRQ_MODE	0x12
#define SVC_MODE	0x13
#define ABT_MODE	0x17
#define UND_MODE	0x1b
#define SYSTEM_MODE	0x1f
#define MODE_MASK	0x1f
#define F_BIT		0x40
#define I_BIT		0x80
#define CC_V_BIT	(1 << 28)
#define CC_C_BIT	(1 << 29)
#define CC_Z_BIT	(1 << 30)
#define CC_N_BIT	(1 << 31)

/* for MMU */
#define CP15_1_MMU	(1 << 0)	/* MMU */
#define CP15_1_ALIGN	(1 << 1)	/* alignment fault */
#define CP15_1_DCACHE	(1 << 2)	/* Data Cache */
#define CP15_1_NOP	0x78		/* Read/Write as 0b1111 */
#define CP15_1_SYSP	(1 << 8)	/* System protection */
#define CP15_1_ROMP	(1 << 9)	/* ROM protection */
#define CP15_1_BTB	(1 << 11)	/* Branch Target Buffer */
#define CP15_1_ICACHE	(1 << 12)	/* Instruction Cache */
#define CP15_1_VECTREL	(1 << 13)	/* Exception Vector Relocation */

/*
 * PXA Chip selects
 */

#define PXA_CS0_PHYS	0x00000000
#define PXA_CS1_PHYS	0x04000000
#define PXA_CS2_PHYS	0x08000000
#define PXA_CS3_PHYS	0x0C000000
#define PXA_CS4_PHYS	0x10000000
#define PXA_CS5_PHYS	0x14000000


/*
 * UARTs
 */

#define FFUART_BASE	0x40100000
#define BTUART_BASE	0x40200000
#define STUART_BASE	0x40700000
#define UART_RBR	0x00	/* Receive Buffer Register (read only) */
#define UART_THR	0x00	/* Transmit Holding Register (write only) */
#define UART_IER	0x04	/* Interrupt Enable Register */
#define UART_IIR	0x08	/* Interrupt ID Register (read only) */
#define UART_FCR	0x08	/* FIFO Control Register (write only) */
#define UART_LCR	0x0C	/* Line Control Register */
#define UART_MCR	0x10	/* Modem Control Register */
#define UART_LSR	0x14	/* Line Status Register (read only) */
#define UART_SPR	0x1C	/* Scratch Pad Register */
#define UART_ISR	0x20	/* Infrared Selection Register */
#define UART_DLL	0x00	/* Divisor Latch Low Register (DLAB = 1) */
#define UART_DLH	0x04	/* Divisor Latch High Register (DLAB = 1) */

/* The interrupt enable register bits. */
#define SIO_IER_RAVIE   0x01	/* enable received data available irq */
#define SIO_IER_TIE	0x02	/* enable transmit data request interrupt */
#define SIO_IER_RLSE	0x04	/* enable receiver line status irq */
#define SIO_IER_MIE	0x08	/* enable modem status interrupt */
#define SIO_IER_RTOIE	0x10	/* enable Rx timeout interrupt */
#define SIO_IER_NRZE	0x20	/* enable NRZ coding */
#define SIO_IER_UUE	0x40	/* enable the UART unit */
#define SIO_IER_DMAE	0x80	/* enable DMA requests */

/* The interrupt identification register bits. */
#define SIO_IIR_IP	0x01 	/* 0 if interrupt pending */
#define SIO_IIR_ID_MASK	0xff 	/* mask for interrupt ID bits */
#define ISR_Tx		0x02
#define ISR_Rx		0x04

/* The line status register bits. */
#define SIO_LSR_DR	0x01	/* data ready */
#define SIO_LSR_OE	0x02	/* overrun error */
#define SIO_LSR_PE	0x04	/* parity error */
#define SIO_LSR_FE	0x08	/* framing error */
#define SIO_LSR_BI	0x10	/* break interrupt */
#define SIO_LSR_THRE	0x20	/* transmitter holding register empty */
#define SIO_LSR_TEMT	0x40	/* transmitter holding and Tx shift registers empty */
#define SIO_LSR_ERR	0x80	/* any error condition (FIFOE) */

/* The modem status register bits. */
#define SIO_MSR_DCTS	0x01	/* delta clear to send */
#define SIO_MSR_DDSR	0x02	/* delta data set ready */
#define SIO_MSR_TERI	0x04	/* trailing edge ring indicator */
#define SIO_MSR_DDCD	0x08	/* delta data carrier detect */
#define SIO_MSR_CTS	0x10	/* clear to send */
#define SIO_MSR_DSR	0x20	/* data set ready */
#define SIO_MSR_RI	0x40	/* ring indicator */
#define SIO_MSR_DCD	0x80	/* data carrier detect */

/* The line control register bits. */
#define SIO_LCR_WLS0	0x01 	/* word length select bit 0 */
#define SIO_LCR_WLS1	0x02 	/* word length select bit 1 */
#define SIO_LCR_STB	0x04 	/* number of stop bits */
#define SIO_LCR_PEN	0x08 	/* parity enable */
#define SIO_LCR_EPS	0x10 	/* even parity select */
#define SIO_LCR_SP	0x20 	/* stick parity */
#define SIO_LCR_SB	0x40 	/* set break */
#define SIO_LCR_DLAB	0x80 	/* divisor latch access bit */

/* The FIFO control register */
#define SIO_FCR_FCR0	0x01 	/* enable xmit and rcvr fifos */
#define SIO_FCR_FCR1	0x02 	/* clear RCVR FIFO */
#define SIO_FCR_FCR2	0x04 	/* clear XMIT FIFO */
#define SIO_FCR_ITL0	0x40 	/* Interrupt trigger level (ITL) bit 0 */
#define SIO_FCR_ITL1	0x80 	/* Interrupt trigger level (ITL) bit 1 */
#define SIO_FCR_ITL_1BYTE	0x00	/* 1 byte triggers interrupt */

/* Full Function UART (FFUART) */
#define FFUART	FFRBR
#define FFRBR	__REG(0x40100000)  /* Receive Buffer Register (read only) */
#define FFTHR	__REG(0x40100000)  /* Transmit Holding Register (write only) */
#define FFIER	__REG(0x40100004)  /* Interrupt Enable Register */
#define FFIIR	__REG(0x40100008)  /* Interrupt ID Register (read only) */
#define FFFCR	__REG(0x40100008)  /* FIFO Control Register (write only) */
#define FFLCR	__REG(0x4010000C)  /* Line Control Register */
#define FFMCR	__REG(0x40100010)  /* Modem Control Register */
#define FFLSR	__REG(0x40100014)  /* Line Status Register (read only) */
#define FFMSR	__REG(0x40100018)  /* Modem Status Register (read only) */
#define FFSPR	__REG(0x4010001C)  /* Scratch Pad Register */
#define FFISR	__REG(0x40100020)  /* Infrared Selection Register */
#define FFDLL	__REG(0x40100000)  /* Divisor Latch Low Register (DLAB = 1) */
#define FFDLH	__REG(0x40100004)  /* Divisor Latch High Register (DLAB = 1) */

/* Bluetooth UART (BTUART) */
#define BTUART	BTRBR
#define BTRBR	__REG(0x40200000)  /* Receive Buffer Register (read only) */
#define BTTHR	__REG(0x40200000)  /* Transmit Holding Register (write only) */
#define BTIER	__REG(0x40200004)  /* Interrupt Enable Register */
#define BTIIR	__REG(0x40200008)  /* Interrupt ID Register (read only) */
#define BTFCR	__REG(0x40200008)  /* FIFO Control Register (write only) */
#define BTLCR	__REG(0x4020000C)  /* Line Control Register */
#define BTMCR	__REG(0x40200010)  /* Modem Control Register */
#define BTLSR	__REG(0x40200014)  /* Line Status Register (read only) */
#define BTMSR	__REG(0x40200018)  /* Modem Status Register (read only) */
#define BTSPR	__REG(0x4020001C)  /* Scratch Pad Register */
#define BTISR	__REG(0x40200020)  /* Infrared Selection Register */
#define BTDLL	__REG(0x40200000)  /* Divisor Latch Low Register (DLAB = 1) */
#define BTDLH	__REG(0x40200004)  /* Divisor Latch High Register (DLAB = 1) */

/* Standard UART (STUART) */
#define STUART	STRBR
#define STRBR	__REG(0x40700000)  /* Receive Buffer Register (read only) */
#define STTHR	__REG(0x40700000)  /* Transmit Holding Register (write only) */
#define STIER	__REG(0x40700004)  /* Interrupt Enable Register */
#define STIIR	__REG(0x40700008)  /* Interrupt ID Register (read only) */
#define STFCR	__REG(0x40700008)  /* FIFO Control Register (write only) */
#define STLCR	__REG(0x4070000C)  /* Line Control Register */
#define STMCR	__REG(0x40700010)  /* Modem Control Register */
#define STLSR	__REG(0x40700014)  /* Line Status Register (read only) */
#define STMSR	__REG(0x40700018)  /* Reserved */
#define STSPR	__REG(0x4070001C)  /* Scratch Pad Register */
#define STISR	__REG(0x40700020)  /* Infrared Selection Register */
#define STDLL	__REG(0x40700000)  /* Divisor Latch Low Register (DLAB = 1) */
#define STDLH	__REG(0x40700004)  /* Divisor Latch High Register (DLAB = 1) */


/*
 * Real Time Clock
 */

#define RCNR		__REG(0x40900000)  /* RTC Count Register */
#define RTAR		__REG(0x40900004)  /* RTC Alarm Register */
#define RTSR		__REG(0x40900008)  /* RTC Status Register */
#define RTTR		__REG(0x4090000C)  /* RTC Timer Trim Register */

#define RTSR_HZE	(1 << 3)	/* HZ interrupt enable */
#define RTSR_ALE	(1 << 2)	/* RTC alarm interrupt enable */
#define RTSR_HZ		(1 << 1)	/* HZ rising-edge detected */
#define RTSR_AL		(1 << 0)	/* RTC alarm detected */

/*
 * OS Timer & Match Registers
 */

#define OSMR0		__REG(0x40A00000)  /* */
#define OSMR1		__REG(0x40A00004)  /* */
#define OSMR2		__REG(0x40A00008)  /* */
#define OSMR3		__REG(0x40A0000C)  /* */
#define OSCR		__REG(0x40A00010)  /* OS Timer Counter Register */
#define OSSR		__REG(0x40A00014)  /* OS Timer Status Register */
#define OWER		__REG(0x40A00018)  /* OS Timer Watchdog Enable Register */
#define OIER		__REG(0x40A0001C)  /* OS Timer Interrupt Enable Register */

#define OSSR_M3		(1 << 3)	/* Match status channel 3 */
#define OSSR_M2		(1 << 2)	/* Match status channel 2 */
#define OSSR_M1		(1 << 1)	/* Match status channel 1 */
#define OSSR_M0		(1 << 0)	/* Match status channel 0 */

#define OWER_WME	(1 << 0)	/* Watchdog Match Enable */

#define OIER_E3		(1 << 3)	/* Interrupt enable channel 3 */
#define OIER_E2		(1 << 2)	/* Interrupt enable channel 2 */
#define OIER_E1		(1 << 1)	/* Interrupt enable channel 1 */
#define OIER_E0		(1 << 0)	/* Interrupt enable channel 0 */


/*
 * Interrupt Controller
 */

#define ICIP		__REG(0x40D00000)  /* Interrupt Controller IRQ Pending Register */
#define ICMR		__REG(0x40D00004)  /* Interrupt Controller Mask Register */
#define ICLR		__REG(0x40D00008)  /* Interrupt Controller Level Register */
#define ICFP		__REG(0x40D0000C)  /* Interrupt Controller FIQ Pending Register */
#define ICPR		__REG(0x40D00010)  /* Interrupt Controller Pending Register */
#define ICCR		__REG(0x40D00014)  /* Interrupt Controller Control Register */


/*
 * General Purpose I/O
 */

#define PXA250_GPIO_BASE	0x40E00000
#define GPSR0_OFFSET	0x18	/* GPIO Pin Output Set Register GPIO<31:0> */
#define GPSR1_OFFSET	0x1C	/* GPIO Pin Output Set Register GPIO<63:32> */
#define GPSR2_OFFSET	0x20	/* GPIO Pin Output Set Register GPIO<80:64> */
#define GPCR0_OFFSET	0x24	/* GPIO Pin Output Clear Register GPIO<31:0> */
#define GPCR1_OFFSET	0x28	/* GPIO Pin Output Clear Register GPIO <63:32> */
#define GPCR2_OFFSET	0x2C	/* GPIO Pin Output Clear Register GPIO <80:64> */
#define GPDR0_OFFSET	0x0C	/* GPIO Pin Direction Register GPIO<31:0> */
#define GPDR1_OFFSET	0x10	/* GPIO Pin Direction Register GPIO<63:32> */
#define GPDR2_OFFSET	0x14	/* GPIO Pin Direction Register GPIO<80:64> */
#define GAFR0_L_OFFSET	0x54	/* GPIO Alternate Function Select Register GPIO<15:0> */
#define GAFR0_U_OFFSET	0x58	/* GPIO Alternate Function Select Register GPIO<31:16> */
#define GAFR1_L_OFFSET	0x5C	/* GPIO Alternate Function Select Register GPIO<47:32> */
#define GAFR1_U_OFFSET	0x60	/* GPIO Alternate Function Select Register GPIO<63:48> */
#define GAFR2_L_OFFSET	0x64	/* GPIO Alternate Function Select Register GPIO<79:64> */
#define GAFR2_U_OFFSET	0x68	/* GPIO Alternate Function Select Register GPIO 80 */

#define GPLR0		__REG(0x40E00000)  /* GPIO Pin-Level Register GPIO<31:0> */
#define GPLR1		__REG(0x40E00004)  /* GPIO Pin-Level Register GPIO<63:32> */
#define GPLR2		__REG(0x40E00008)  /* GPIO Pin-Level Register GPIO<80:64> */

#define GPDR0		__REG(0x40E0000C)  /* GPIO Pin Direction Register GPIO<31:0> */
#define GPDR1		__REG(0x40E00010)  /* GPIO Pin Direction Register GPIO<63:32> */
#define GPDR2		__REG(0x40E00014)  /* GPIO Pin Direction Register GPIO<80:64> */

#define GPSR0		__REG(0x40E00018)  /* GPIO Pin Output Set Register GPIO<31:0> */
#define GPSR1		__REG(0x40E0001C)  /* GPIO Pin Output Set Register GPIO<63:32> */
#define GPSR2		__REG(0x40E00020)  /* GPIO Pin Output Set Register GPIO<80:64> */

#define GPCR0		__REG(0x40E00024)  /* GPIO Pin Output Clear Register GPIO<31:0> */
#define GPCR1		__REG(0x40E00028)  /* GPIO Pin Output Clear Register GPIO <63:32> */
#define GPCR2		__REG(0x40E0002C)  /* GPIO Pin Output Clear Register GPIO <80:64> */

#define GRER0		__REG(0x40E00030)  /* GPIO Rising-Edge Detect Register GPIO<31:0> */
#define GRER1		__REG(0x40E00034)  /* GPIO Rising-Edge Detect Register GPIO<63:32> */
#define GRER2		__REG(0x40E00038)  /* GPIO Rising-Edge Detect Register GPIO<80:64> */

#define GFER0		__REG(0x40E0003C)  /* GPIO Falling-Edge Detect Register GPIO<31:0> */
#define GFER1		__REG(0x40E00040)  /* GPIO Falling-Edge Detect Register GPIO<63:32> */
#define GFER2		__REG(0x40E00044)  /* GPIO Falling-Edge Detect Register GPIO<80:64> */

#define GEDR0		__REG(0x40E00048)  /* GPIO Edge Detect Status Register GPIO<31:0> */
#define GEDR1		__REG(0x40E0004C)  /* GPIO Edge Detect Status Register GPIO<63:32> */
#define GEDR2		__REG(0x40E00050)  /* GPIO Edge Detect Status Register GPIO<80:64> */

#define GAFR0_L		__REG(0x40E00054)  /* GPIO Alternate Function Select Register GPIO<15:0> */
#define GAFR0_U		__REG(0x40E00058)  /* GPIO Alternate Function Select Register GPIO<31:16> */
#define GAFR1_L		__REG(0x40E0005C)  /* GPIO Alternate Function Select Register GPIO<47:32> */
#define GAFR1_U		__REG(0x40E00060)  /* GPIO Alternate Function Select Register GPIO<63:48> */
#define GAFR2_L		__REG(0x40E00064)  /* GPIO Alternate Function Select Register GPIO<79:64> */
#define GAFR2_U		__REG(0x40E00068)  /* GPIO Alternate Function Select Register GPIO 80 */

/* GPIO pin */
#define GPIO11_3P6MHz	(1 << 11)
#define GPIO13_MBGNT	(1 << 13)
#define GPIO15_nCS1	(1 << 15)
#define GPIO23_SCLK	(1 << 23)
#define GPIO24_SFRM	(1 << 24)

#define GPIO33_nCS5	(1 << 1)
#define GPIO34_FFRXD	(1 << 2)
#define GPIO35_FFCTS	(1 << 3)
#define GPIO36_FFDCD	(1 << 4)
#define GPIO37_FFDSR	(1 << 5)
#define GPIO38_FFRI	(1 << 6)
#define GPIO39_FFTXD	(1 << 7)
#define GPIO40_FFDTR	(1 << 8)
#define GPIO41_FFRTS	(1 << 9)
#define GPIO43_BTTXD	(1 << 11)
#define GPIO47_STTXD	(1 << 15)
#define GPIO48_nPOE	(1 << 16)
#define GPIO49_nPWE	(1 << 17)
#define GPIO50_nPIOR	(1 << 18)
#define GPIO51_nPIOW	(1 << 19)
#define GPIO52_nPCE1	(1 << 20)
#define GPIO53_nPCE2	(1 << 21)
#define GPIO54_nPSKTSEL	(1 << 22)
#define GPIO55_nPREG	(1 << 23)
#define GPIO56_nPWAIT	(1 << 24)
#define GPIO57_nIOIS16	(1 << 25)

#define GPIO78_nCS2	(1 << 14)
#define GPIO79_nCS3	(1 << 15)
#define GPIO80_nCS4	(1 << 16)

/* GPIO alternative function */
#define GP11_3P6MHz	(0x1 << 22)	/* 3.6 MHz osc. out */
#define GP13_MBGNT	(0x2 << 26)	/* Memory Controller grant */
#define GP14_MBREQ	(0x1 << 28)	/* Mem.ctrl. alt. bus master request */
#define GP15_nCS1	(0x2 << 30)

#define GP16_PWM0	(0x2 << 0)	/* PWM0 output */
#define GP18_RDY	(0x1 << 4)	/* Ext. bus Ready */
#define GP19_DREQ1	(0x1 << 6)	/* External DMA request */
#define GP20_DREQ0	(0x1 << 8)	/* External DMA request */
#define GP21_DVAL0	(0x2 << 10)	/* ? */
#define GP22_DVAL1	(0x2 << 12)	/* ? */
#define GP23_SCLK	(0x2 << 14)	/* SSP clock */
#define GP24_SRFM	(0x2 << 16)	/* SSP Frame */
#define GP25_TXD	(0x2 << 18)	/* SSP Transmit */
#define GP26_RXD	(0x1 << 20)	/* SSP Recevice */
#define GP27_EXTCLK	(0x1 << 22)	/* SSP Ext_clk */
#define GP28_BITCLK	(0x1 << 24)	/* AC97 bit_clk(IN) */
#define GP29_SDATA_IN	(0x1 << 26)	/* AC97 Sdata_in0 */
#define GP30_SDATA_OUT	(0x2 << 28)	/* AC97 Sdata_out */
#define GP31_SYNC	(0x2 << 30)	/* AC97 Sync */

#define GP33_nCS5	(0x2 << 2)
#define GP34_FFRXD	(0x1 << 4)	/* FFUART receive */
#define GP39_FFTXD	(0x2 << 14)	/* FFUART transmit */
#define GP42_BTRXD	(0x1 << 20)	/* BTUART receive */
#define GP43_BTTXD	(0x2 << 22)	/* BTUART transmit */
#define GP46_ICP_RXD	(0x1 << 28)	/* ICP receive */
#define GP46_STRXD	(0x2 << 28)	/* STUART receive */
#define GP47_STTXD	(0x1 << 30)	/* STUART transmit */
#define GP47_ICP_TXD	(0x2 << 30)	/* ICP transmit */

#define GP48_nPOE	(0x2 << 0)	/* Output Enable for Card Space */
#define GP49_nPWE	(0x2 << 2)	/* Write Enable for Card Space */
#define GP50_nPIOR	(0x2 << 4)	/* I/O Read for Card Space */
#define GP51_nPIOW	(0x2 << 6)	/* I/O Write for Card Space */
#define GP52_nPCE1	(0x2 << 8)	/* Card Enable for Card Space */
#define GP53_nPCE2	(0x2 << 10)	/* Card Enable for Card Space */
#define GP54_nPSKTSEL	(0x2 << 12)	/* Socket Select for Card Space */
#define GP55_nPREG	(0x2 << 14)	/* Card Address Bit 26 */
#define GP56_nPWAIT	(0x1 << 16)	/* Wait signal for Card Space */
#define GP57_nIOIS16	(0x1 << 18)	/* Bus Width select for I/O Card Space */
#define GP58_LDD0	(0x2 << 20)	/* LCD data */
#define GP59_LDD1	(0x2 << 22)
#define GP60_LDD2	(0x2 << 24)
#define GP61_LDD3	(0x2 << 26)
#define GP62_LDD4	(0x2 << 28)
#define GP63_LDD5	(0x2 << 30)

#define GP64_LDD6	(0x2 << 0)
#define GP65_LDD7	(0x2 << 2)
#define GP66_LDD8	(0x2 << 4)
#define GP67_LDD9	(0x2 << 6)
#define GP68_LDD10	(0x2 << 8)
#define GP69_LDD11	(0x2 << 10)
#define GP70_LDD12	(0x2 << 12)
#define GP71_LDD13	(0x2 << 14)
#define GP72_LDD14	(0x2 << 16)
#define GP73_LDD15	(0x2 << 18)
#define GP74_FCLK	(0x2 << 20)	/* LCD Frame clock */
#define GP75_LCLK	(0x2 << 22)	/* LCD line clock */
#define GP76_PCLK	(0x2 << 24)	/* LCD pixel clock */
#define GP77_ACBIAS	(0x2 << 26)	/* LCD AC bias */
#define GP78_nCS2	(0x2 << 28)
#define GP79_nCS3	(0x2 << 30)

#define GP80_nCS4	(0x2 << 0)

/*
 * Power Manager
 */

#define PXA250_PWR_BASE	0x40F00000
#define PMCR_OFFSET	0x00	/* PM Control Reg. */
#define PWER_OFFSET	0x0C	/* PM Wake-up Enable Reg. */
#define PCFR_OFFSET	0x1C	/* PM General Configuration Reg. */
#define PCFR_OPDE	(1 << 0)
#define PCFR_FP		(1 << 1)
#define PCFR_FS		(1 << 2)
#define PCFR_DS		(1 << 3)
#define PWER_WE0	(1 << 0)
#define PWER_WE1	(1 << 1)
#define PWER_WERTC	(1 << 31)

#define PMCR		__REG(0x40F00000)  /* Power Manager Control Register */
#define PSSR		__REG(0x40F00004)  /* Power Manager Sleep Status Register */
#define PSPR		__REG(0x40F00008)  /* Power Manager Scratch Pad Register */
#define PWER		__REG(0x40F0000C)  /* Power Manager Wake-up Enable Register */
#define PRER		__REG(0x40F00010)  /* Power Manager GPIO Rising-Edge Detect Enable Register */
#define PFER		__REG(0x40F00014)  /* Power Manager GPIO Falling-Edge Detect Enable Register */
#define PEDR		__REG(0x40F00018)  /* Power Manager GPIO Edge Detect Status Register */
#define PCFR		__REG(0x40F0001C)  /* Power Manager General Configuration Register */
#define PGSR0		__REG(0x40F00020)  /* Power Manager GPIO Sleep State Register for GP[31-0] */
#define PGSR1		__REG(0x40F00024)  /* Power Manager GPIO Sleep State Register for GP[63-32] */
#define PGSR2		__REG(0x40F00028)  /* Power Manager GPIO Sleep State Register for GP[84-64] */
#define RCSR		__REG(0x40F00030)  /* Reset Controller Status Register */
#define RCSR_ALL	0x0f
#define RCSR_GPR	(1 << 3)	/* GPIO reset */
#define RCSR_SMR	(1 << 2)	/* Sleep Mode */
#define RCSR_WDR	(1 << 1)	/* Watchdog reset */
#define RCSR_HWR	(1 << 0)	/* Hardware reset */

#define PSSR_MASK	0x37
#define PSSR_RDH	(1 << 5)	/* Read Disable Hold */
#define PSSR_PH		(1 << 4)	/* Peripherial Control Hold */
#define PSSR_VFS	(1 << 2)	/* VDD Fault Status */
#define PSSR_BFS	(1 << 1)	/* Battery Fault Status */
#define PSSR_SSS	(1 << 0)	/* Software Sleep Status */

/*
 * Core Clock
 */

#define PXA250_CCLK_BASE	0x41300000
#define CCCR_OFFSET		0x00	/* Core Clock Configuration Register */
#define CKEN_OFFSET		0x04	/* Clock Enable Register */
#define OSCC_OFFSET		0x08	/* Oscillator Configuration Register */

#define CCCR		__REG(0x41300000)  /* Core Clock Configuration Register */
#define CKEN		__REG(0x41300004)  /* Clock Enable Register */
#define OSCC		__REG(0x41300008)  /* Oscillator Configuration Register */

#define CCCR_N_MASK	0x0380		/* Run Mode Frequency to Turbo Mode Frequency Multiplier */
#define CCCR_M_MASK	0x0060		/* Memory Frequency to Run Mode Frequency Multiplier */
#define CCCR_L_MASK	0x001f		/* Crystal Frequency to Memory Frequency Multiplier */

#define CKEN16_LCD	(1 << 16)	/* LCD Unit Clock Enable */
#define CKEN14_I2C	(1 << 14)	/* I2C Unit Clock Enable */
#define CKEN13_FICP	(1 << 13)	/* FICP Unit Clock Enable */
#define CKEN12_MMC	(1 << 12)	/* MMC Unit Clock Enable */
#define CKEN11_USB	(1 << 11)	/* USB Unit Clock Enable */
#define CKEN8_I2S	(1 << 8)	/* I2S Unit Clock Enable */
#define CKEN7_BTUART	(1 << 7)	/* BTUART Unit Clock Enable */
#define CKEN6_FFUART	(1 << 6)	/* FFUART Unit Clock Enable */
#define CKEN5_STUART	(1 << 5)	/* STUART Unit Clock Enable */
#define CKEN3_SSP	(1 << 3)	/* SSP Unit Clock Enable */
#define CKEN2_AC97	(1 << 2)	/* AC97 Unit Clock Enable */
#define CKEN1_PWM1	(1 << 1)	/* PWM1 Clock Enable */
#define CKEN0_PWM0	(1 << 0)	/* PWM0 Clock Enable */

#define OSCC_OON	(1 << 1)	/* 32.768kHz OON (write-once only bit) */
#define OSCC_OOK	(1 << 0)	/* 32.768kHz OOK (read-only bit) */


/*
 * Memory controller
 */

#define PXA250_MEMC_BASE	0x48000000
#define MDCNFG_OFFSET	0x00	/* SDRAM Configuration Register 0 */
#define MDREFR_OFFSET	0x04	/* SDRAM Refresh Control Register */
#define MSC0_OFFSET	0x08	/* Static Memory Control Register 0 */
#define MSC1_OFFSET	0x0C	/* Static Memory Control Register 1 */
#define MSC2_OFFSET	0x10	/* Static Memory Control Register 2 */
#define MECR_OFFSET	0x14	/* Expansion Memory (PCMCIA/Compact Flash) Bus Configuration */
#define SXCNFG_OFFSET	0x1C	/* Synchronous Static Memory Control Register */
#define SXMRS_OFFSET	0x24	/* MRS value to be written to Synchronous Flash or SMROM */
#define MCMEM0_OFFSET	0x28	/* Card interface Common Memory Space Socket 0 Timing */
#define MCMEM1_OFFSET	0x2C	/* Card interface Common Memory Space Socket 1 Timing */
#define MCATT0_OFFSET	0x30	/* Card interface Attribute Space Socket 0 Timing Configuration */
#define MCATT1_OFFSET	0x34	/* Card interface Attribute Space Socket 1 Timing Configuration */
#define MCIO0_OFFSET	0x38	/* Card interface I/O Space Socket 0 Timing Configuration */
#define MCIO1_OFFSET	0x3C	/* Card interface I/O Space Socket 1 Timing Configuration */
#define MDMRS_OFFSET	0x40	/* MRS value to be written to SDRAM */
#define BOOT_DEF_OFFSET	0x44	/* Read-Only Boot-Time Register. Contains BOOT_SEL and PKG_SEL */
#define MDCNFG_DE0	(1 << 0)
#define MDCNFG_DE1	(1 << 1)
#define MDCNFG_DE2	(1 << 16)
#define MDCNFG_DE3	(1 << 17)
#define MDCNFG_DWID0	(1 << 2)

#define MDREFR_K2FREE	(1 << 25)
#define MDREFR_K1FREE	(1 << 24)
#define MDREFR_K0FREE	(1 << 23)
#define MDREFR_SLFRSH	(1 << 22)
#define MDREFR_APD	(1 << 20)
#define MDREFR_E0PIN	(1 << 12)
#define MDREFR_E1PIN	(1 << 15)
#define MDREFR_K0DB2	(1 << 14)
#define MDREFR_K1DB2	(1 << 17)
#define MDREFR_K2DB2	(1 << 19)
#define MDREFR_K0RUN	(1 << 13)
#define MDREFR_K1RUN	(1 << 16)
#define MDREFR_K2RUN	(1 << 18)

#define MDCNFG		__REG(0x48000000)  /* SDRAM Configuration Register 0 */
#define MDREFR		__REG(0x48000004)  /* SDRAM Refresh Control Register */
#define MSC0		__REG(0x48000008)  /* Static Memory Control Register 0 */
#define MSC1		__REG(0x4800000C)  /* Static Memory Control Register 1 */
#define MSC2		__REG(0x48000010)  /* Static Memory Control Register 2 */
#define MECR		__REG(0x48000014)  /* Expansion Memory (PCMCIA/Compact Flash) Bus Configuration */
#define SXLCR		__REG(0x48000018)  /* LCR value to be written to SDRAM-Timing Synchronous Flash */
#define SXCNFG		__REG(0x4800001C)  /* Synchronous Static Memory Control Register */
#define SXMRS		__REG(0x48000024)  /* MRS value to be written to Synchronous Flash or SMROM */
#define MCMEM0		__REG(0x48000028)  /* Card interface Common Memory Space Socket 0 Timing */
#define MCMEM1		__REG(0x4800002C)  /* Card interface Common Memory Space Socket 1 Timing */
#define MCATT0		__REG(0x48000030)  /* Card interface Attribute Space Socket 0 Timing Configuration */
#define MCATT1		__REG(0x48000034)  /* Card interface Attribute Space Socket 1 Timing Configuration */
#define MCIO0		__REG(0x48000038)  /* Card interface I/O Space Socket 0 Timing Configuration */
#define MCIO1		__REG(0x4800003C)  /* Card interface I/O Space Socket 1 Timing Configuration */
#define MDMRS		__REG(0x48000040)  /* MRS value to be written to SDRAM */
#define BOOT_DEF	__REG(0x48000044)  /* Read-Only Boot-Time Register. Contains BOOT_SEL and PKG_SEL */

#endif /* _PXA250_H_ */
/*
 | $Id: pxa250.h,v 1.6 2002/09/03 07:42:49 tolkien Exp $
 |
 | Local Variables:
 | mode: c
 | mode: font-lock
 | version-control: t
 | delete-old-versions: t
 | End:
 |
 | -*- End-Of-File -*-
 */
