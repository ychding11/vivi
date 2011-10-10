/*
 * vivi/arch/s3c2410/head.S:
 *   Initialise hardware
 *
 * Copyright (C) 2001 MIZI Research, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * Author: Janghoon Lyu <nandy@mizi.com>
 * Date  : $Date: 2003/02/26 10:38:11 $
 *
 * $Revision: 1.18 $
 *
 *
 * History:
 *
 * 2002-05-14: Janghoon Lyu <nandy@mizi.com>
 *   - Initial code
 *
 */

#include "config.h"
#include "linkage.h"
#include "machine.h"

@ Start of executable code 

ENTRY(_start)
ENTRY(ResetEntryPoint)

@
@ Exception vector table (physical address = 0x00000000)
@

@ 0x00: Reset
	b	Reset

@ 0x04: Undefined instruction exception
UndefEntryPoint:
	b	HandleUndef

@ 0x08: Software interrupt exception
SWIEntryPoint:
	b	HandleSWI

@ 0x0c: Prefetch Abort (Instruction Fetch Memory Abort)
PrefetchAbortEnteryPoint:
	b	HandlePrefetchAbort

@ 0x10: Data Access Memory Abort
DataAbortEntryPoint:
	b	HandleDataAbort

@ 0x14: Not used
NotUsedEntryPoint:
	b	HandleNotUsed

@ 0x18: IRQ(Interrupt Request) exception
IRQEntryPoint:
	b	HandleIRQ

@ 0x1c: FIQ(Fast Interrupt Request) exception
FIQEntryPoint:
	b	HandleFIQ

@
@ VIVI magics
@

@ 0x20: magic number so we can verify that we only put 
	.long   0
@ 0x24:
	.long   0
@ 0x28: where this vivi was linked, so we can put it in memory in the right place
	.long   _start
@ 0x2C: this contains the platform, cpu and machine id
	.long   ARCHITECTURE_MAGIC
@ 0x30: vivi capabilities 
	.long   0
#ifdef CONFIG_PM
@ 0x34:
	b	SleepRamProc
#endif
#ifdef CONFIG_TEST
@ 0x38:
	b	hmi
#endif


@
@ Start VIVI head
@
Reset:
	@ disable watch dog timer
	mov	r1, #0x53000000
	mov	r2, #0x0
	str	r2, [r1]

#ifdef CONFIG_S3C2410_MPORT3
	mov	r1, #0x56000000
	mov	r2, #0x00000005
	str	r2, [r1, #0x70]
	mov r2, #0x00000001
	str	r2, [r1, #0x78]
	mov	r2, #0x00000001
	str r2, [r1, #0x74]
#endif

	@ disable all interrupts
	mov	r1, #INT_CTL_BASE
	mov	r2, #0xffffffff
	str	r2, [r1, #oINTMSK]
	ldr	r2, =0x7ff
	str	r2, [r1, #oINTSUBMSK]	

	@ initialise system clocks
	mov	r1, #CLK_CTL_BASE
	mvn	r2, #0xff000000
	str	r2, [r1, #oLOCKTIME]

	@ldr	r2, mpll_50mhz
	@str	r2, [r1, #oMPLLCON]
#ifndef CONFIG_S3C2410_MPORT1
	@ 1:2:4
	mov	r1, #CLK_CTL_BASE
	mov	r2, #0x3
	str	r2, [r1, #oCLKDIVN]

	mrc	p15, 0, r1, c1, c0, 0		@ read ctrl register 
	orr	r1, r1, #0xc0000000		@ Asynchronous  
	mcr	p15, 0, r1, c1, c0, 0		@ write ctrl register

	@ now, CPU clock is 200 Mhz
	mov	r1, #CLK_CTL_BASE
	ldr	r2, mpll_200mhz
	str	r2, [r1, #oMPLLCON]
#else
	@ 1:2:2
    mov r1, #CLK_CTL_BASE
    ldr r2, clock_clkdivn
    str r2, [r1, #oCLKDIVN]

    mrc p15, 0, r1, c1, c0, 0       @ read ctrl register
    orr r1, r1, #0xc0000000     @ Asynchronous
    mcr p15, 0, r1, c1, c0, 0       @ write ctrl register

    @ now, CPU clock is 100 Mhz
    mov r1, #CLK_CTL_BASE
    ldr r2, mpll_100mhz
    str r2, [r1, #oMPLLCON]
#endif
	bl	memsetup

#ifdef CONFIG_PM
	@ Check if this is a wake-up from sleep
	ldr	r1, PMST_ADDR
	ldr	r0, [r1]
	tst	r0, #(PMST_SMR)
	bne	WakeupStart
#endif


#ifdef CONFIG_S3C2410_SMDK
	@ All LED on
	mov	r1, #GPIO_CTL_BASE
	add	r1, r1, #oGPIO_F
	ldr	r2,=0x55aa
	str	r2, [r1, #oGPIO_CON]
	mov	r2, #0xff
	str	r2, [r1, #oGPIO_UP]
	mov	r2, #0x00
	str	r2, [r1, #oGPIO_DAT]
#endif

#if 0
	@ SVC
	mrs	r0, cpsr
	bic	r0, r0, #0xdf
	orr	r1, r0, #0xd3
	msr	cpsr_all, r1
#endif

	@ set GPIO for UART
	mov	r1, #GPIO_CTL_BASE
	add	r1, r1, #oGPIO_H
	ldr	r2, gpio_con_uart	
	str	r2, [r1, #oGPIO_CON]
	ldr	r2, gpio_up_uart
	str	r2, [r1, #oGPIO_UP]	
	bl	InitUART

#ifdef CONFIG_DEBUG_LL
	@ Print current Program Counter
	ldr	r1, SerBase
	mov	r0, #'\r'
	bl	PrintChar
	mov	r0, #'\n'
	bl	PrintChar
	mov	r0, #'@'
	bl	PrintChar
	mov	r0, pc
	bl	PrintHexWord
#endif


#ifdef CONFIG_BOOTUP_MEMTEST
	@ simple memory test to find some DRAM flaults.
	bl	memtest
#endif

#ifdef CONFIG_S3C2410_NAND_BOOT
	bl	copy_myself

	@ jump to ram
	ldr	r1, =on_the_ram
	add	pc, r1, #0
	nop
	nop
1:	b	1b		@ infinite loop

on_the_ram:
#endif

#ifdef CONFIG_DEBUG_LL
	ldr	r1, SerBase
	ldr	r0, STR_STACK
	bl	PrintWord
	ldr	r0, DW_STACK_START
	bl	PrintHexWord
#endif

	@ get read to call C functions
	ldr	sp, DW_STACK_START	@ setup stack pointer
	mov	fp, #0			@ no previous frame, so fp=0
	mov	a2, #0			@ set argv to NULL 

	bl	main			@ call main 

	mov	pc, #FLASH_BASE		@ otherwise, reboot

@
@ End VIVI head
@

/*
 * subroutines
 */

@
@ Wake-up codes
@
#ifdef CONFIG_PM
WakeupStart:
	@ Clear sleep reset bit
	ldr	r0, PMST_ADDR
	mov	r1, #PMST_SMR
	str	r1, [r0]

	@ Release the SDRAM signal protections
	ldr	r0, PMCTL1_ADDR
	ldr	r1, [r0]
	bic	r1, r1, #(SCLKE | SCLK1 | SCLK0)
	str	r1, [r0]

	@ Go...
	ldr	r0, PMSR0_ADDR	@ read a return address
	ldr	r1, [r0]
	mov	pc, r1
	nop
	nop
1:	b	1b		@ infinite loop

SleepRamProc:
	@ SDRAM is in the self-refresh mode */
	ldr	r0, REFR_ADDR
	ldr	r1, [r0]
	orr	r1, r1, #SELF_REFRESH
	str	r1, [r0]

	@ wait until SDRAM into self-refresh
	mov	r1, #16
1:	subs	r1, r1, #1	
	bne	1b

	@ Set the SDRAM singal protections
	ldr	r0, PMCTL1_ADDR
	ldr	r1, [r0]
	orr	r1, r1, #(SCLKE | SCLK1 | SCLK0)
	str	r1, [r0]

	/* Sleep... Now */
	ldr	r0, PMCTL0_ADDR
	ldr	r1, [r0]
	orr	r1, r1, #SLEEP_ON
	str	r1, [r0]	
1:	b	1b

#ifdef CONFIG_TEST
hmi:
	ldr	r0, PMCTL0_ADDR
	ldr	r1, =0x7fff0
	str	r1, [r0]
	
	@ All LED on
	mov	r1, #GPIO_CTL_BASE
	add	r1, r1, #oGPIO_F
	ldr	r2,=0x55aa
	str	r2, [r1, #oGPIO_CON]
	mov	r2, #0xff
	str	r2, [r1, #oGPIO_UP]
	mov	r2, #0xe0
	str	r2, [r1, #oGPIO_DAT]
1:	b	1b
#endif

#endif

ENTRY(memsetup)
	@ initialise the static memory 

	@ set memory control registers
	mov	r1, #MEM_CTL_BASE
	adrl	r2, mem_cfg_val
	add	r3, r1, #52
1:	ldr	r4, [r2], #4
	str	r4, [r1], #4
	cmp	r1, r3
	bne	1b

	mov	pc, lr


#ifdef CONFIG_S3C2410_NAND_BOOT
@
@ copy_myself: copy vivi to ram
@
copy_myself:
	mov	r10, lr

	@ reset NAND
	mov	r1, #NAND_CTL_BASE
	ldr	r2, =0xf830		@ initial value
	str	r2, [r1, #oNFCONF]
	ldr	r2, [r1, #oNFCONF]
	bic	r2, r2, #0x800		@ enable chip
	str	r2, [r1, #oNFCONF]
	mov	r2, #0xff		@ RESET command
	strb	r2, [r1, #oNFCMD]
	mov	r3, #0			@ wait 
1:	add	r3, r3, #0x1
	cmp	r3, #0xa
	blt	1b
2:	ldr	r2, [r1, #oNFSTAT]	@ wait ready
	tst	r2, #0x1
	beq	2b
	ldr	r2, [r1, #oNFCONF]
	orr	r2, r2, #0x800		@ disable chip
	str	r2, [r1, #oNFCONF]

	@ get read to call C functions (for nand_read())
	ldr	sp, DW_STACK_START	@ setup stack pointer
	mov	fp, #0			@ no previous frame, so fp=0

	@ copy vivi to RAM
	ldr	r0, =VIVI_RAM_BASE
	mov     r1, #0x0
	mov	r2, #0x20000
	bl	nand_read_ll

	tst	r0, #0x0
	beq	ok_nand_read
#ifdef CONFIG_DEBUG_LL
bad_nand_read: 
	ldr	r0, STR_FAIL
	ldr	r1, SerBase
	bl	PrintWord
1:	b	1b		@ infinite loop 
#endif
	
ok_nand_read:
#ifdef CONFIG_DEBUG_LL
	ldr	r0, STR_OK
	ldr	r1, SerBase
	bl	PrintWord
#endif

	@ verify
	mov	r0, #0
	ldr	r1, =0x33f00000
	mov	r2, #0x400	@ 4 bytes * 1024 = 4K-bytes
go_next:
	ldr	r3, [r0], #4
	ldr	r4, [r1], #4
	teq	r3, r4
	bne	notmatch
	subs	r2, r2, #4
	beq	done_nand_read	
	bne	go_next
notmatch:
#ifdef CONFIG_DEBUG_LL
	sub	r0, r0, #4
	ldr	r1, SerBase
	bl	PrintHexWord
	ldr	r0, STR_FAIL
	ldr	r1, SerBase
	bl	PrintWord
#endif
1:	b	1b
done_nand_read:

#ifdef CONFIG_DEBUG_LL
	ldr	r0, STR_OK
	ldr	r1, SerBase
	bl	PrintWord
#endif

	mov	pc, r10

@ clear memory
@ r0: start address
@ r1: length
mem_clear:
	mov	r2, #0
	mov	r3, r2
	mov	r4, r2
	mov	r5, r2
	mov	r6, r2
	mov	r7, r2
	mov	r8, r2
	mov	r9, r2

clear_loop:
	stmia	r0!, {r2-r9}
	subs	r1, r1, #(8 * 4)
	bne	clear_loop

	mov	pc, lr

#endif @ CONFIG_S3C2410_NAND_BOOT


#ifdef CONFIG_BOOTUP_MEMTEST
@
@ Simple memory test function
@
memtest:
	mov	r10, lr

#ifdef CONFIG_DEBUG_LL
	mov	r0, #'M'
	ldr	r1, SerBase
	bl	PrintChar
	mov	r0, #'T'
	ldr	r1, SerBase
	bl	PrintChar
	mov	r0, #'S'
	ldr	r1, SerBase
	bl	PrintChar
	mov	r0, #'T'
	ldr	r1, SerBase
	bl	PrintChar
	mov	r0, #' '
	ldr	r1, SerBase
	bl	PrintChar
#endif

        /* check the first 1MB  in increments of 4k */
        mov     r7, #0x1000 
        mov     r6, r7, lsl #8  /* 4k << 2^8 = 1MB */
        mov     r5, #DRAM_BASE

mem_test_loop:
        mov     r0, r5  
        bl      testram_nostack 
        teq     r0, #1  
        beq     badram  

        add     r5, r5, r7
        subs    r6, r6, r7
        bne     mem_test_loop


	@ the first megabyte is OK. so let us clear it.
        mov     r0, #((1024 * 1024) / (8 * 4))	@ 1MB in steps of 32 bytes
        mov     r1, #DRAM_BASE
        mov     r2, #0  
        mov     r3, #0  
        mov     r4, #0  
        mov     r5, #0  
        mov     r6, #0  
        mov     r7, #0  
        mov     r8, #0  
        mov     r9, #0  

clear_loop_memtest:
        stmia   r1!, {r2-r9} 
        subs    r0, r0, #(8 * 4)
        bne     clear_loop_memtest

#ifdef CONFIG_DEBUG_LL
	ldr	r0, STR_OK
	ldr	r1, SerBase
	bl	PrintWord
#endif

	mov	pc, r10		@ return

badram: 
#ifdef CONFIG_DEBUG_LL
	ldr	r0, STR_FAIL
	ldr	r1, SerBase
	bl	PrintWord
#endif
1:	b	1b		@ loop 


@ testmem.S: memory tester, test if there is RAM available at given location
@
@ Copyright (C) 2001 Russell King (rmk@arm.linux.org.uk)
@
@ This version clobbers registers r1-r4, so be sure to store their contents
@ in a safe position. This function is not APCS compliant, so only use it
@ from assembly code.
@
@ r0 = address to test 
@ returns r0 = 0 - ram present, r0 = 1 - no ram
@ clobbers r1 - r4 
ENTRY(testram_nostack)
        ldmia   r0, {r1, r2}    @ store current value in r1 and r2
        mov     r3, #0x55       @ write 0x55 to first word
        mov     r4, #0xaa       @ 0xaa to second
        stmia   r0, {r3, r4}
        ldmia   r0, {r3, r4}    @ read it back 
        teq     r3, #0x55       @ do the values match
        teqeq   r4, #0xaa
        bne     bad             @ oops, no
        mov     r3, #0xaa       @ write 0xaa to first word
        mov     r4, #0x55       @ 0x55 to second
        stmia   r0, {r3, r4}
        ldmia   r0, {r3, r4}    @ read it back 
        teq     r3, #0xaa       @ do the values match
        teqeq   r4, #0x55
bad:    stmia   r0, {r1, r2}    @ in any case, restore old data
        moveq   r0, #0          @ ok - all values matched 
        movne   r0, #1          @ no ram at this location
        mov     pc, lr 

#endif @ CONFIG_BOOTUP_MEMTEST



@ Initialize UART
@
@ r0 = number of UART port
InitUART:
	ldr	r1, SerBase
	mov	r2, #0x0
	str	r2, [r1, #oUFCON]
	str	r2, [r1, #oUMCON]
	mov	r2, #0x3
	str	r2, [r1, #oULCON]
	ldr	r2, =0x245
	str	r2, [r1, #oUCON]
@ #define UART_BRD ((50000000 / (UART_BAUD_RATE * 16)) - 1)
#define UART_BRD ((S3C2410MCLK /4 / (UART_BAUD_RATE * 16)) - 1)
	mov	r2, #UART_BRD
	str	r2, [r1, #oUBRDIV]

	mov	r3, #100
	mov	r2, #0x0
1:	sub	r3, r3, #0x1
	tst	r2, r3
	bne	1b

#if 0
	mov	r2, #'U'
	str	r2, [r1, #oUTXHL]

1:	ldr	r3, [r1, #oUTRSTAT]
	and	r3, r3, #UTRSTAT_TX_EMPTY
	tst	r3, #UTRSTAT_TX_EMPTY
	bne	1b	

	mov	r2, #'0'
	str	r2, [r1, #oUTXHL]

1:	ldr	r3, [r1, #oUTRSTAT]
	and	r3, r3, #UTRSTAT_TX_EMPTY
	tst	r3, #UTRSTAT_TX_EMPTY
	bne	1b	
#endif

	mov	pc, lr


@
@ Exception handling functions
@
HandleUndef:
#ifdef CONFIG_DEBUG_LL
	mov	r12, r14
	ldr	r0, STR_UNDEF
	ldr	r1, SerBase
	bl	PrintWord
	bl	PrintFaultAddr
#endif
1:	b	1b		@ infinite loop

HandleSWI:
#ifdef CONFIG_DEBUG_LL
	mov	r12, r14
	ldr	r0, STR_SWI
	ldr	r1, SerBase
	bl	PrintWord
	bl	PrintFaultAddr
#endif
1:	b	1b		@ infinite loop

HandlePrefetchAbort:
#ifdef CONFIG_DEBUG_LL
	mov	r12, r14
	ldr	r0, STR_PREFETCH_ABORT
	ldr	r1, SerBase
	bl	PrintWord
	bl	PrintFaultAddr
#endif
1:	b	1b		@ infinite loop

HandleDataAbort:
#ifdef CONFIG_DEBUG_LL
	mov	r12, r14
	ldr	r0, STR_DATA_ABORT
	ldr	r1, SerBase
	bl	PrintWord
	bl	PrintFaultAddr
#endif
1:	b	1b		@ infinite loop

HandleIRQ:
#ifdef CONFIG_DEBUG_LL
	mov	r12, r14
	ldr	r0, STR_IRQ
	ldr	r1, SerBase
	bl	PrintWord
	bl	PrintFaultAddr
#endif
1:	b	1b		@ infinite loop

HandleFIQ:
#ifdef CONFIG_DEBUG_LL
	mov	r12, r14
	ldr	r0, STR_FIQ
	ldr	r1, SerBase
	bl	PrintWord
	bl	PrintFaultAddr
#endif
1:	b	1b		@ infinite loop

HandleNotUsed:
#ifdef CONFIG_DEBUG_LL
	mov	r12, r14
	ldr	r0, STR_NOT_USED
	ldr	r1, SerBase
	bl	PrintWord
	bl	PrintFaultAddr
#endif
1:	b	1b		@ infinite loop


@
@ Low Level Debug
@
#ifdef CONFIG_DEBUG_LL

@
@ PrintFaultAddr: Print falut address
@
@ r12: contains address of instruction + 4
@
PrintFaultAddr:
	mov	r0, r12			@ Print address of instruction + 4
	ldr	r1, SerBase
	bl	PrintHexWord
	mrc	p15, 0, r0, c6, c0, 0	@ Read fault virtual address
	ldr	r1, SerBase
	bl	PrintHexWord
	mov	pc, lr

@ PrintHexNibble : prints the least-significant nibble in R0 as a
@ hex digit
@   r0 contains nibble to write as Hex
@   r1 contains base of serial port
@   writes ro with XXX, modifies r0,r1,r2
@   TODO : write ro with XXX reg to error handling
@   Falls through to PrintChar
PrintHexNibble:
	adr	r2, HEX_TO_ASCII_TABLE
	and	r0, r0, #0xF
	ldr	r0, [r2, r0]	@ convert to ascii
	b	PrintChar

@ PrintChar : prints the character in R0
@   r0 contains the character
@   r1 contains base of serial port
@   writes ro with XXX, modifies r0,r1,r2
@   TODO : write ro with XXX reg to error handling
PrintChar:
TXBusy:
	ldr	r2, [r1, #oUTRSTAT]
	and	r2, r2, #UTRSTAT_TX_EMPTY
	tst	r2, #UTRSTAT_TX_EMPTY
	beq	TXBusy	
	str	r0, [r1, #oUTXHL]
	mov	pc, lr

@ PrintWord : prints the 4 characters in R0
@   r0 contains the binary word
@   r1 contains the base of the serial port
@   writes ro with XXX, modifies r0,r1,r2
@   TODO : write ro with XXX reg to error handling
PrintWord:
	mov	r3, r0
	mov	r4, lr
	bl	PrintChar

	mov	r0, r3, LSR #8		/* shift word right 8 bits */
	bl	PrintChar

	mov	r0, r3, LSR #16		/* shift word right 16 bits */
	bl	PrintChar
	
	mov	r0, r3, LSR #24		/* shift word right 24 bits */
	bl	PrintChar

	mov	r0, #'\r'
	bl	PrintChar

	mov	r0, #'\n'
	bl	PrintChar

	mov	pc, r4

@ PrintHexWord : prints the 4 bytes in R0 as 8 hex ascii characters
@   followed by a newline
@   r0 contains the binary word
@   r1 contains the base of the serial port
@   writes ro with XXX, modifies r0,r1,r2
@   TODO : write ro with XXX reg to error handling
PrintHexWord:
	mov	r4, lr
	mov	r3, r0
	mov	r0, r3, LSR #28
	bl	PrintHexNibble
	mov	r0, r3, LSR #24
	bl	PrintHexNibble
	mov	r0, r3, LSR #20
	bl	PrintHexNibble
	mov	r0, r3, LSR #16
	bl	PrintHexNibble
	mov	r0, r3, LSR #12
	bl	PrintHexNibble
	mov	r0, r3, LSR #8
	bl	PrintHexNibble
	mov	r0, r3, LSR #4
	bl	PrintHexNibble
	mov	r0, r3
	bl	PrintHexNibble

	mov	r0, #'\r'
	bl	PrintChar

	mov	r0, #'\n'
	bl	PrintChar

	mov	pc, r4
#endif
	
@
@ Data Area
@
@ Memory configuration values
.align 4
mem_cfg_val:
	.long	vBWSCON
	.long	vBANKCON0
	.long	vBANKCON1
	.long	vBANKCON2
	.long	vBANKCON3
	.long	vBANKCON4
	.long	vBANKCON5
	.long	vBANKCON6
	.long	vBANKCON7
	.long	vREFRESH
	.long	vBANKSIZE
	.long	vMRSRB6
	.long	vMRSRB7

@ Processor clock values
.align 4
clock_locktime:
	.long	vLOCKTIME
mpll_50mhz:
	.long	vMPLLCON_50
#ifdef CONFIG_S3C2410_MPORT1
mpll_100mhz:
	.long   vMPLLCON_100
#endif
mpll_200mhz:
	.long	vMPLLCON_200
clock_clkcon:
	.long	vCLKCON
clock_clkdivn:
	.long	vCLKDIVN
@ initial values for serial
uart_ulcon:
	.long	vULCON
uart_ucon:
	.long	vUCON
uart_ufcon:
	.long	vUFCON
uart_umcon:
	.long	vUMCON
@ inital values for GPIO
gpio_con_uart:
	.long	vGPHCON
gpio_up_uart:
	.long	vGPHUP

	.align	2
DW_STACK_START:
	.word	STACK_BASE+STACK_SIZE-4

#ifdef CONFIG_DEBUG_LL
	.align	2
HEX_TO_ASCII_TABLE:
	.ascii	"0123456789ABCDEF"
STR_STACK:
	.ascii	"STKP"
STR_UNDEF:
	.ascii	"UNDF"
STR_SWI:
	.ascii	"SWI "
STR_PREFETCH_ABORT:
	.ascii	"PABT"
STR_DATA_ABORT:
	.ascii	"DABT"
STR_IRQ:
	.ascii	"IRQ "
STR_FIQ:
	.ascii	"FIQ"
STR_NOT_USED:
	.ascii	"NUSD"
	.align 2
STR_OK:
	.ascii	"OK  "
STR_FAIL:
	.ascii	"FAIL"
STR_CR:
	.ascii  "\r\n"
#endif

.align 4
SerBase:
#if defined(CONFIG_SERIAL_UART0)
	.long UART0_CTL_BASE
#elif defined(CONFIG_SERIAL_UART1)
	.long UART1_CTL_BASE
#elif defined(CONFIG_SERIAL_UART2)
	.long UART2_CTL_BASE
#else
#error not defined base address of serial
#endif

#ifdef CONFIG_PM
.align 4
PMCTL0_ADDR:
	.long 0x4c00000c
PMCTL1_ADDR:
	.long 0x56000080
PMST_ADDR:
	.long 0x560000B4
PMSR0_ADDR:
	.long 0x560000B8
REFR_ADDR:
	.long 0x48000024
#endif
