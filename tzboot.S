/*
 * boot.S - simple register setup code for stand-alone Linux booting
 *
 * Copyright (C) 2011 ARM Limited. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE.txt file.
 */
	.syntax	unified
	.arch_extension sec

    .globl reset
reset:
	b	init   	@ Must be first instruction for power on reset vector

.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
mon_vectors:
	.word 0	/* reset */
	b   unused_vec /* undef */
	b 	smc_handler /* smc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
vectors:
	.word 0	/* reset */
	b   undef_handler	/* undef */
	b	svc_handler /* svc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

unused_vec:
    b  unused_handler
    b end

	.globl	init
init:
init_set_mode:
    /* Set mode and disable interrupts */
    mrs r10, cpsr
    bic r10, r10, #0x1f     @ Mask mode
    ldr r12, =0x16          @ Monitor mode
    orr r12, r10, r12
    orr r12, r12, #0xc0     @ Mask IRQ and FIQ
    msr cpsr, r12
    isb

init_scr:
    /* Set up SCR
     * It is expected that on reset SCR is 0.  For now we only only make SMC
     * undefined in non-secure state by setting SCD (bit 7) to 1.
     */
    mrc p15, 0, r10, c1, c1, 0      @ SCR
    bic r10, r10, #0x07f
    ldr r11, =0x80                  @ SCR.SCD = 1
    orr r10, r10, r11
    mcr p15, 0, r11, c1, c1, 0
    isb

init_vbar:
    /* Make sure vectors are based at 0 */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           @ SCTLR.V = 0
    mcr p15, 0, r10, c1, c0, 0

    /* Set up MVBAR */
    mrc p15, 0, r11, c12, c0, 1
    ldr r11, =mon_vectors
    mcr p15, 0, r11, c12, c0, 1
    isb

    /* Set up secure VBAR */
    mrc p15, 0, r11, c12, c0, 0
    ldr r11, =vectors
    mcr p15, 0, r11, c12, c0, 0
    isb
    
    /* Switch to non-secure state */
    mov r0, #1
    bl set_ns

    /* Make sure vectors are based at 0 */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           @ SCTLR.V = 0
    mcr p15, 0, r10, c1, c0, 0

    /* Set up non-secure VBAR */
    mrc p15, 0, r11, c12, c0, 0
    ldr r11, =vectors
    mcr p15, 0, r11, c12, c0, 0
    isb

    /* Switch back to secure state */
    mov r0, #0
    bl set_ns

init_uart:
	@
	@ UART initialisation (38400 8N1)
	@
	ldr	r0, =0x1c090000			@ UART base (Versatile Express)
	mov	r1, #0x10			@ ibrd
	str	r1, [r0, #0x24]
	mov	r1, #0xc300
	orr	r1, #0x0001			@ cr
	str	r1, [r0, #0x30]

init_stacks:
    /* Switch to system mode */
    mov r0, #0x1f           @ SYS
    bl set_mode

	@ Set up a stack for CPU 0 so we can write most of this in C.
	ldr     sp, =stacktop-0x80000

    /* Switch to supervisor mode from here on */
    mov r0, #0x13           @ SVC
    bl set_mode

    /* Switch to nonsecure state */
    mov r0, #0
    bl set_ns

	@ Set up a stack for CPU 0 so we can write most of this in C.
	ldr     sp, =stacktop

	@ And call the C entrypoint
	bl      c_start
	
end:	
    bl      end

	@
	@ Function for C code to make semihosting calls:
	@
	.globl __semi_call
__semi_call:
	svc     0x123456
	mov pc, lr

    .globl set_mode
set_mode:
    /* Set the execution mode directly */
    mov r9, lr                     @ Save LR as we lose it on the CPSR change
    mrs r10, cpsr
    bic r10, r10, #0x1f
    mov r12, r0
    orr r12, r10, r12
    msr cpsr, r12
    isb
	mov pc, r9                     @ Jump to the inst following call
    
    .globl get_mode
 get_mode:
    mrs r10, cpsr
    and r0, r10, #0x1f
	mov pc, lr                     @ Jump to the inst following call

    .globl set_ns
 set_ns:
    /* Set the security state */
    mrc p15, 0, r10, c1, c1, 0      @ SCR
    bic r10, r10, #1                @ Mask off NS 
    orr r10, r10, r0
    mcr p15, 0, r11, c1, c1, 0
    isb
	mov pc, lr

    .globl __smc
__smc:
    smc 0                           @ SMC calling convention mandates imm of 0
	mov pc, lr
