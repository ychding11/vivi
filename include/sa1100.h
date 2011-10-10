/****************************************************************************/
/* Copyright 2000 Compaq Computer Corporation.                              */
/*                                           .                              */
/* Copying or modifying this code for any purpose is permitted,             */
/* provided that this copyright notice is preserved in its entirety         */
/* in all copies or modifications.  COMPAQ COMPUTER CORPORATION             */
/* MAKES NO WARRANTIES, EXPRESSED OR IMPLIED, AS TO THE USEFULNESS          */
/* OR CORRECTNESS OF THIS CODE OR ITS FITNESS FOR ANY PARTICULAR            */
/* PURPOSE.                                                                 */
/****************************************************************************/

/*
 * Copyright (C) 2001 Mizi Research, Inc.
 *
 * Copying or modifying this code for any purpose is permitted,
 * provided that this copyright notice is preserved in its entirety
 * in all copies or modifications. Mizi Research makes no warranties, 
 * expressed or implied, as to the usefullness or correctness of this 
 * code or its fitness for any particular purpose.
 */

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

#define SA1100_UTCR0     0x00
#define SA1100_UTCR1     0x04
#define SA1100_UTCR2     0x08
#define SA1100_UTCR3     0x0C
#define SA1100_UTDR      0x14
#define SA1100_UTSR0     0x1c
#define SA1100_UTSR1     0x20

#define SA1100_UTCR0_PE		(1 << 0)   /* parity enable */
#define SA1100_UTCR0_OES	(1 << 1)   /* 1 for even parity */
#define SA1100_UTCR0_2STOP	(1 << 2)   /* 1 for 2 stop bits */
#define SA1100_UTCR0_8BIT	(1 << 3)   /* 1 for 8 bit data */
#define SA1100_UTCR0_SCE	(1 << 4)   /* sample clock enable */
#define SA1100_UTCR0_RCE	(1 << 5)   /* receive clock edge select */
#define SA1100_UTCR0_TCE	(1 << 6)   /* transmit clock edge select */

#define SA1100_UTCR1_BRDHIMASK  0xF
#define SA1100_UTCR2_BRDLoMASK  0xFF

#define SA1100_UTCR3_RXE	(1 << 0)	/* receiver enable */
#define SA1100_UTCR3_TXE	(1 << 1)	/* transmit enable */
#define SA1100_UTCR3_BRK	(1 << 2)	/* send a BRK */
#define SA1100_UTCR3_RIE	(1 << 3)	/* receive FIFO interrupt enable */
#define SA1100_UTCR3_TIE	(1 << 4)	/* transmit FIFO interrupt enable */
#define SA1100_UTCR3_LBM	(1 << 5)	/* loopback mode */

/* [1] 11.11.6 */
#define SA1100_UTDR_PRE		(1 << 8)	/* parity error */
#define SA1100_UTDR_FRE		(1 << 9)	/* framing error */
#define SA1100_UTDR_ROR		(1 << 10)	/* receiver overrun */

/* [1] 11.11.7 */
#define SA1100_UTSR0_TFS	(1 << 0)	/* transmit FIFO service request */

/* [1] 11.11.8 */
#define SA1100_UTSR1_TBY	(1 << 0)	/* transmit FIFO busy */
#define SA1100_UTSR1_RNE	(1 << 1)        /* receive FIFO not empty */
#define SA1100_UTSR1_TNF	(1 << 2)        /* transmit FIFO not full */
#define SA1100_UTSR1_PRE	(1 << 3)        /* parity error */          
#define SA1100_UTSR1_FRE	(1 << 4)        /* framing error */         
#define SA1100_UTSR1_ROR	(1 << 5)        /* receiver overrun */      

#define SA1100_UTSR1_ERROR_MASK 0x38

#define SA1100_SDLCBASE		0x80020060
#define SA1100_UART1BASE	0x80010000 
#define SA1100_UART2BASE	0x80030000 
#define SA1100_UART3BASE	0x80050000 

#define SA1100_UART1_UTCR0      (SA1100_UART1BASE + SA1100_UTCR0)
#define SA1100_UART1_UTCR1      (SA1100_UART1BASE + SA1100_UTCR1)
#define SA1100_UART1_UTCR2      (SA1100_UART1BASE + SA1100_UTCR2)
#define SA1100_UART1_UTCR3      (SA1100_UART1BASE + SA1100_UTCR3)
#define SA1100_UART1_UTDR       (SA1100_UART1BASE + SA1100_UTDR)
#define SA1100_UART1_UTSR0      (SA1100_UART1BASE + SA1100_UTSR0)
#define SA1100_UART1_UTSR1      (SA1100_UART1BASE + SA1100_UTSR1)

#define SA1100_UART2_UTCR0      (SA1100_UART2BASE + SA1100_UTCR0)
#define SA1100_UART2_UTCR1      (SA1100_UART2BASE + SA1100_UTCR1)
#define SA1100_UART2_UTCR2      (SA1100_UART2BASE + SA1100_UTCR2)
#define SA1100_UART2_UTCR3      (SA1100_UART2BASE + SA1100_UTCR3)
#define SA1100_UART2_UTDR       (SA1100_UART2BASE + SA1100_UTDR)
#define SA1100_UART2_UTSR0      (SA1100_UART2BASE + SA1100_UTSR0)
#define SA1100_UART2_UTSR1      (SA1100_UART2BASE + SA1100_UTSR1)

#define SA1100_UART3_UTCR0      (SA1100_UART3BASE + SA1100_UTCR0)
#define SA1100_UART3_UTCR1      (SA1100_UART3BASE + SA1100_UTCR1)
#define SA1100_UART3_UTCR2      (SA1100_UART3BASE + SA1100_UTCR2)
#define SA1100_UART3_UTCR3      (SA1100_UART3BASE + SA1100_UTCR3)
#define SA1100_UART3_UTDR       (SA1100_UART3BASE + SA1100_UTDR)
#define SA1100_UART3_UTSR0      (SA1100_UART3BASE + SA1100_UTSR0)
#define SA1100_UART3_UTSR1      (SA1100_UART3BASE + SA1100_UTSR1)



/* 
 * DRAM Configuration values 
 */

/* [1] 10.2 */
#define SA1100_DRAM_CONFIGURATION_BASE 0xA0000000

#define SA1100_MDCNFG			0x00 /* must be initialized */
#define SA1100_MDCAS00			0x04 /* must be initialized */
#define SA1100_MDCAS01			0x08 /* must be initialized */
#define SA1100_MDCAS02			0x0c /* must be initialized */

#define SA1100_MSC0			0x10 /* must be initialized */
#define SA1100_MSC1			0x14 /* must be initialized */
#define SA1100_MECR			0x18 /* should be initialized */
#define SA1100_MDREFR			0x1c /* must be initialized */

#define SA1100_MDCAS20                  0x20 /* OK not to initialize this register, because the enable bits are cleared on reset */
#define SA1100_MDCAS21                  0x24 /* OK not to initialize this register, because the enable bits are cleared on reset */
#define SA1100_MDCAS22                  0x28 /* OK not to initialize this register, because the enable bits are cleared on reset */

#define SA1100_MSC2                     0x2C /* must be initialized */

#define SA1100_SMCNFG                   0x30 /* should be initialized */


#define MDCNFG_BANK0_ENABLE (1 << 0)
#define MDCNFG_BANK1_ENABLE (1 << 1)
#define MDCNFG_DTIM0_SDRAM  (1 << 2)
#define MDCNFG_DWID0_32B    (0 << 3)
#define MDCNFG_DWID0_16B    (1 << 3)
#define MDCNFG_DRAC0(n_)    (((n_) & 7) << 4)
#define MDCNFG_TRP0(n_)     (((n_) & 0xF) << 8)
#define MDCNFG_TDL0(n_)     (((n_) & 3) << 12)
#define MDCNFG_TWR0(n_)     (((n_) & 3) << 14)
	
/* DRAM Refresh Control Register (MDREFR) [1] 10.2.2 */	
#define MDREFR_TRASR(n_)    (((n_) & 0xF) << 0)	
#define MDREFR_DRI(n_)    (((n_) & 0xFFF) << 4)
#define MDREFR_E0PIN		(1 << 16)
#define MDREFR_K0RUN		(1 << 17)
#define MDREFR_K0DB2		(1 << 18)
#define MDREFR_E1PIN		(1 << 20)	/* SDRAM clock enable pin 1 (banks 0-1) */
#define MDREFR_K1RUN		(1 << 21)	/* SDRAM clock pin 1 run (banks 0-1) */
#define MDREFR_K1DB2		(1 << 22)	/* SDRAM clock pin 1 divide-by-two (banks 0-1) */
#define MDREFR_K2RUN		(1 << 25)       /* SDRAM clock enable pin 2 (banks 2-3) */       
#define MDREFR_K2DB2		(1 << 26)       /* SDRAM clock pin 2 run (banks 2-3) */          
#define MDREFR_EAPD		(1 << 28)       /* SDRAM clock pin 2 divide-by-two (banks 2-3) */
#define MDREFR_KAPD		(1 << 29)
#define MDREFR_SLFRSH		(1 << 31)	/* SDRAM self refresh */


#define MSC_RT_ROMFLASH         0
#define MSC_RT_SRAM_012         1
#define MSC_RT_VARLAT_345       1
#define MSC_RT_BURST4           2
#define MSC_RT_BURST8           3

#define MSC_RBW32               (0 << 2)
#define MSC_RBW16               (1 << 2)

#define MSC_RDF(n_)             (((n_)&0x1f)<<3)
#define MSC_RDN(n_)             (((n_)&0x1f)<<8)
#define MSC_RRR(n_)             (((n_)&0x7)<<13)

#define SA1100_RCSR_REG		0x90030004
#define RCSR_HWR		(1 << 0)	
#define RCSR_SWR		(1 << 1)
#define RCSR_WDR		(1 << 2)
#define RCSR_SMR		(1 << 3)

#define SA1100_PSSR_REG		0x90020004
#define PSSR_SSS		(1 << 0) 
#define PSSR_BFS		(1 << 1)
#define PSSR_VFS                (1 << 2)
#define PSSR_DH			(1 << 3)
#define PSSR_PH			(1 << 4)

#define SA1100_PSPR_REG		0x90020008

#define SA1100_ICMR_REG         0x90050004

#define SA1100_PPCR_REG         0x90020014

#define SA1100_PPCR_59MHZ	0
#define SA1100_PPCR_73MHZ	1
#define SA1100_PPCR_88MHZ	2
#define SA1100_PPCR_103MHZ	3
#define SA1100_PPCR_118MHZ	4
#define SA1100_PPCR_132MHZ	5
#define SA1100_PPCR_147MHZ	6
#define SA1100_PPCR_162MHZ	7
#define SA1100_PPCR_176MHZ	8
#define SA1100_PPCR_191MHZ	9
#define SA1100_PPCR_206MHZ	10
#define SA1100_PPCR_221MHZ	11

#define SA1100_PPDR_REG   	0x90060000	/* PPC Pin Direction Reg.          */
#define SA1100_PPDR_LFCLK 	0x400

#define SA1100_PPSR_REG   	0x90060004	/* PPC Pin State Reg.              */

#define	SA1100_GPIO_BASE	(0x90040000)
#define	SA1100_GPIO_GPLR_OFF	(0)
#define	SA1100_GPIO_GPDR_OFF	(4)
#define	SA1100_GPIO_GPSR_OFF	(8)
#define	SA1100_GPIO_GPCR_OFF	(0xc)
#define	SA1100_GPIO_GRER_OFF	(0x10)
#define	SA1100_GPIO_GFER_OFF	(0x14)
#define	SA1100_GPIO_GEDR_OFF	(0x18)
#define	SA1100_GPIO_GAFR_OFF	(0x1c)

#define GPLR			__REG(SA1100_GPIO_BASE+SA1100_GPIO_GPLR_OFF)


#define	SA1100_GPIO_SET(off, bits)  \
    ((*((volatile unsigned long *)(((char*)SA1100_GPIO_BASE)+(off))))|=(bits))

#define	SA1100_GPIO_CLR(off, bits)  \
    ((*((volatile unsigned long *)(((char*)SA1100_GPIO_BASE)+(off))))&=~(bits))

#define	SA1100_GPIO_READ(off)  \
    (*((volatile unsigned long *)(((char*)SA1100_GPIO_BASE)+(off))))

#define	SA1100_GPIO_WRITE(off, v)  \
    ((*((volatile unsigned long *)(((char*)SA1100_GPIO_BASE)+(off)))) = (v))

#define	SA1100_GPIO_GAFR_LCD_BITS   (0xff << 2)
#define	SA1100_GPIO_GPDR_LCD_BITS   (0xff << 2)

#define	SA1100_GPIO_GPLR_READ()	    SA1100_GPIO_READ(SA1100_GPIO_GPLR_OFF)
#define	SA1100_GPIO_GPDR_READ()	    SA1100_GPIO_READ(SA1100_GPIO_GPDR_OFF)
#define	SA1100_GPIO_GPSR_READ()	    SA1100_GPIO_READ(SA1100_GPIO_GPSR_OFF)
#define	SA1100_GPIO_GPCR_READ()	    SA1100_GPIO_READ(SA1100_GPIO_GPCR_OFF)
#define	SA1100_GPIO_GRER_READ()	    SA1100_GPIO_READ(SA1100_GPIO_GRER_OFF)
#define	SA1100_GPIO_GFER_READ()	    SA1100_GPIO_READ(SA1100_GPIO_GFER_OFF)
#define	SA1100_GPIO_GEDR_READ()	    SA1100_GPIO_READ(SA1100_GPIO_GEDR_OFF)
#define	SA1100_GPIO_GAFR_READ()	    SA1100_GPIO_READ(SA1100_GPIO_GAFR_OFF)

#define	SA1100_GPIO_GPLR_WRITE(v)   SA1100_GPIO_WRITE(SA1100_GPIO_GPLR_OFF,v)
#define	SA1100_GPIO_GPDR_WRITE(v)   SA1100_GPIO_WRITE(SA1100_GPIO_GPDR_OFF,v)
#define	SA1100_GPIO_GPSR_WRITE(v)   SA1100_GPIO_WRITE(SA1100_GPIO_GPSR_OFF,v)
#define	SA1100_GPIO_GPCR_WRITE(v)   SA1100_GPIO_WRITE(SA1100_GPIO_GPCR_OFF,v)
#define	SA1100_GPIO_GRER_WRITE(v)   SA1100_GPIO_WRITE(SA1100_GPIO_GRER_OFF,v)
#define	SA1100_GPIO_GFER_WRITE(v)   SA1100_GPIO_WRITE(SA1100_GPIO_GFER_OFF,v)
#define	SA1100_GPIO_GEDR_WRITE(v)   SA1100_GPIO_WRITE(SA1100_GPIO_GEDR_OFF,v)
#define	SA1100_GPIO_GAFR_WRITE(v)   SA1100_GPIO_WRITE(SA1100_GPIO_GAFR_OFF,v)

#define	SA1100_GPIO_GPLR_SET(v)	    SA1100_GPIO_SET(SA1100_GPIO_GPLR_OFF,v)
#define	SA1100_GPIO_GPDR_SET(v)	    SA1100_GPIO_SET(SA1100_GPIO_GPDR_OFF,v)
#define	SA1100_GPIO_GPSR_SET(v)	    SA1100_GPIO_SET(SA1100_GPIO_GPSR_OFF,v)
#define	SA1100_GPIO_GPCR_SET(v)	    SA1100_GPIO_SET(SA1100_GPIO_GPCR_OFF,v)
#define	SA1100_GPIO_GRER_SET(v)	    SA1100_GPIO_SET(SA1100_GPIO_GRER_OFF,v)
#define	SA1100_GPIO_GFER_SET(v)	    SA1100_GPIO_SET(SA1100_GPIO_GFER_OFF,v)
#define	SA1100_GPIO_GEDR_SET(v)	    SA1100_GPIO_SET(SA1100_GPIO_GEDR_OFF,v)
#define	SA1100_GPIO_GAFR_SET(v)	    SA1100_GPIO_SET(SA1100_GPIO_GAFR_OFF,v)

#define	SA1100_GPIO_GPLR_CLR(v)	    SA1100_GPIO_CLR(SA1100_GPIO_GPLR_OFF,v)
#define	SA1100_GPIO_GPDR_CLR(v)	    SA1100_GPIO_CLR(SA1100_GPIO_GPDR_OFF,v)
#define	SA1100_GPIO_GPSR_CLR(v)	    SA1100_GPIO_CLR(SA1100_GPIO_GPSR_OFF,v)
#define	SA1100_GPIO_GPCR_CLR(v)	    SA1100_GPIO_CLR(SA1100_GPIO_GPCR_OFF,v)
#define	SA1100_GPIO_GRER_CLR(v)	    SA1100_GPIO_CLR(SA1100_GPIO_GRER_OFF,v)
#define	SA1100_GPIO_GFER_CLR(v)	    SA1100_GPIO_CLR(SA1100_GPIO_GFER_OFF,v)
#define	SA1100_GPIO_GEDR_CLR(v)	    SA1100_GPIO_CLR(SA1100_GPIO_GEDR_OFF,v)
#define	SA1100_GPIO_GAFR_CLR(v)	    SA1100_GPIO_CLR(SA1100_GPIO_GAFR_OFF,v)

/*
 * power management reggies
 */

#define	SA1100_PMCR 0x90020000	/* Power manager control register */
#define	SA1100_PSSR 0x90020004	/* Power manager sleep status register */
#define	SA1100_PSPR 0x90020008	/* Power manager scratch pad register */
#define	SA1100_PWER 0x9002000C	/* Power manager wake-up enable register */
#define	SA1100_PCFR 0x90020010	/* Power manager general configuration reg */
#define	PCFR_OPDE   (1<<0)	/* power down 3.6MHz osc */
#define	PCFR_FP	    (1<<1)	/* float PCMCIA controls during sleep */
#define	PCFR_FS	    (1<<2)	/* float static chip selects during sleep */
#define	PCFR_FO	    (1<<3)	/* force 32KHz osc enable on */

#define	SA1100_PPCR 0x90020014 /* Power manager PLL configuration register */
#define	SA1100_PGSR 0x90020018 /* Power manager GPIO sleep state register */
#define	SA1100_POSR 0x9002001C /* Power manager oscillator status register */


#define RSRR		__REG(0x90030000)
#define RCSR		__REG(0x90030004)
#define	RSRR_SWR    0x00000001	/* software reset bit */
#define SA1100_RSRR 0x90030000 /* Reset controller software reset register */
#define SA1100_RCSR 0x90030004 /* Reset controller status register */


#define	SA1100_ICIP 0x90050000	/* interrupt controller IRQ pend reg */
#define	SA1100_ICFP 0x90050010	/* interrupt controller FIQ pend reg */
#define	SA1100_ICMR 0x90050004	/* interrupt controller mask reg */
#define	SA1100_ICLR 0x90050008	/* interrupt controller level reg */
#define	SA1100_ICCR 0x9005000C	/* interrupt controller control reg */

#define	SA1100_RTAR 0x90010000	/* RTC alarm reg */
#define	SA1100_RCNR 0x90010004	/* RTC count reg */
#define	SA1100_RTTR 0x90010008	/* RTC timer trim reg */
#define	SA1100_RTSR 0x90010010	/* RTC status reg */

/*
 * Operating System (OS) timer control registers
 *
 * Registers
 *    OSMR0             Operating System (OS) timer Match Register 0
 *                      (read/write).
 *    OSMR1             Operating System (OS) timer Match Register 1
 *                      (read/write).
 *    OSMR2             Operating System (OS) timer Match Register 2
 *                      (read/write).
 *    OSMR3             Operating System (OS) timer Match Register 3
 *                      (read/write).
 *    OSCR              Operating System (OS) timer Counter Register
 *                      (read/write).
 *    OSSR              Operating System (OS) timer Status Register
 *                      (read/write).
 *    OWER              Operating System (OS) timer Watch-dog Enable Register
 *                      (read/write).
 *    OIER              Operating System (OS) timer Interrupt Enable Register
 *                      (read/write).
 */

#define OSMR0           __REG(0x90000000)  /* OS timer Match Reg. 0 */
#define OSMR1           __REG(0x90000004)  /* OS timer Match Reg. 1 */
#define OSMR2           __REG(0x90000008)  /* OS timer Match Reg. 2 */
#define OSMR3           __REG(0x9000000c)  /* OS timer Match Reg. 3 */
#define OSCR            __REG(0x90000010)  /* OS timer Counter Reg. */
#define OSSR            __REG(0x90000014)  /* OS timer Status Reg. */
#define OWER            __REG(0x90000018)  /* OS timer Watch-dog Enable Reg. */
#define OIER            __REG(0x9000001C)  /* OS timer Interrupt Enable Reg. */

#define OSSR_M(Nb)                      /* Match detected [0..3]           */ \
                        (0x00000001 << (Nb))
#define OSSR_M0         OSSR_M (0)      /* Match detected 0                */
#define OSSR_M1         OSSR_M (1)      /* Match detected 1                */
#define OSSR_M2         OSSR_M (2)      /* Match detected 2                */
#define OSSR_M3         OSSR_M (3)      /* Match detected 3                */

#define OWER_WME        0x00000001      /* Watch-dog Match Enable          */
                                        /* (set only)                      */

#define OIER_E(Nb)                      /* match interrupt Enable [0..3]   */ \
                        (0x00000001 << (Nb))
#define OIER_E0         OIER_E (0)      /* match interrupt Enable 0        */
#define OIER_E1         OIER_E (1)      /* match interrupt Enable 1        */
#define OIER_E2         OIER_E (2)      /* match interrupt Enable 2        */
#define OIER_E3         OIER_E (3)      /* match interrupt Enable 3        */

/*
 * Real-Time Clock (RTC) control registers
 *
 * Registers
 *    RTAR              Real-Time Clock (RTC) Alarm Register (read/write).
 *    RCNR              Real-Time Clock (RTC) CouNt Register (read/write).
 *    RTTR              Real-Time Clock (RTC) Trim Register (read/write).
 *    RTSR              Real-Time Clock (RTC) Status Register (read/write).
 *
 * Clocks
 *    frtx, Trtx        Frequency, period of the real-time clock crystal
 *                      (32.768 kHz nominal).
 *    frtc, Trtc        Frequency, period of the real-time clock counter
 *                      (1 Hz nominal).
 */

#define RTAR            __REG(0x90010000)  /* RTC Alarm Reg. */
#define RCNR            __REG(0x90010004)  /* RTC CouNt Reg. */
#define RTTR            __REG(0x90010008)  /* RTC Trim Reg. */
#define RTSR            __REG(0x90010010)  /* RTC Status Reg. */

#define RTTR_C          Fld (16, 0)     /* clock divider Count - 1         */
#define RTTR_D          Fld (10, 16)    /* trim Delete count               */
                                        /* frtc = (1023*(C + 1) - D)*frtx/ */
                                        /*        (1023*(C + 1)^2)         */
                                        /* Trtc = (1023*(C + 1)^2)*Trtx/   */
                                        /*        (1023*(C + 1) - D)       */

#define RTSR_AL         0x00000001      /* ALarm detected                  */
#define RTSR_HZ         0x00000002      /* 1 Hz clock detected             */
#define RTSR_ALE        0x00000004      /* ALarm interrupt Enable          */
#define RTSR_HZE        0x00000008      /* 1 Hz clock interrupt Enable     */


