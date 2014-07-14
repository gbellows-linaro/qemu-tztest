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

    .equ USR, 0x10
    .equ FIQ, 0x11
    .equ IRQ, 0x12
    .equ SVC, 0x13
    .equ MON, 0x16
    .equ ABT, 0x17
    .equ HYP, 0x1a
    .equ UND, 0x1b
    .equ SYS, 0x1f

    .equ SECURE, 0x0
    .equ NONSECURE, 0x1

reset:
	b	init   	@ Must be first instruction for power on reset vector

.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
mon_vectors:
	.word 0	/* reset */
	b   unused_handler /* undef */
	b  	smc_vec /* smc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
secure_vectors:
	.word 0	/* reset */
	b   undef_handler /* undef */
	b	secure_svc_vec /* svc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
nonsecure_vectors:
	.word 0	/* reset */
	b   undef_handler /* undef */
	b	nonsecure_svc_vec /* svc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

undef_vec:
    b undef_handler
    b end

unused_vec:
    b  unused_handler
    b end

nonsecure_svc_vec:
    b svc_handler
	mov pc, lr

secure_svc_vec:
    b svc_handler
	mov pc, lr

smc_vec:
    srsdb sp!, #MON
    b smc_handler
    rfeia sp!       /* TODO: Should this be a SUBS or MOVS instead? pg A4-179 */

/* Test boot starting point */
init:
init_uart:
	@
	@ UART initialisation (38400 8N1)
	@
	ldr	r0, =0x1c090000			@ UART base (Versatile Express)
	@ldr	r0, =0x09000000			@ UART base (QEMU VIRT)
	mov	r1, #0x10			@ ibrd
	str	r1, [r0, #0x24]
	mov	r1, #0xc300
	orr	r1, #0x0001			@ cr
	str	r1, [r0, #0x30]

init_cpu:
    /* Start by setting up the SVC stack*/
    mrs r10, cpsr
    cps #SVC
	ldr sp, =svc_stacktop
    msr cpsr, r10

    /* Disable interrupts for now */
    mrs r10, cpsr
    orr r10, r10, #0xc0     @ Mask IRQ and FIQ
    msr cpsr, r10
    isb

    /* Make sure secure vectors are based at 0 */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           @ SCTLR.V = 0
    mcr p15, 0, r10, c1, c0, 0

    /* Set up secure VBAR */
    mrc p15, 0, r11, c12, c0, 0
    ldr r11, =secure_vectors
    mcr p15, 0, r11, c12, c0, 0
    isb
    
init_check:
	/* Initial test to validate that the QEMU code initializes in the correct
     * state for Secure use.  
     */
    bl check_init_mode

init_monitor:
    /* Set up the monitor mode stack */
    mrs r10, cpsr
    cps #MON
    ldr sp, =mon_stacktop
    msr cpsr, r10

    /* Make SMC calls undefined from non-secure state */
    /*
    mrc p15, 0, r10, c1, c1, 0      @ SCR
    ldr r11, =0x80                  @ SCR.SCD = 1
    orr r10, r10, r11
    mcr p15, 0, r11, c1, c1, 0
    isb
    */

    /* Set up the monitor vector table */
    mrc p15, 0, r11, c12, c0, 1
    ldr r11, =mon_vectors
    mcr p15, 0, r11, c12, c0, 1

init_nonsecure:
    /* Switch to non-secure state */
    mov r0, 0
    mov r1, NONSECURE
    smc 0

    /* Make sure vectors are based at 0 */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           @ SCTLR.V = 0
    mcr p15, 0, r10, c1, c0, 0

    /* Set up non-secure VBAR */
    mrc p15, 0, r11, c12, c0, 0
    ldr r11, =nonsecure_vectors
    mcr p15, 0, r11, c12, c0, 0
    isb

init_stacks:
    /* Switch back to secure state */
    mov r0, 0
    mov r1, SECURE
    smc 0

    /* Switch to system mode */
    cps #SYS

    /* Set up the SYS stack */
	ldr sp, =sys_stacktop

    /* Switch to nonsecure state */
    mov r0, 0
    mov r1, NONSECURE
    smc 0

    /* Set up the USR stack */
    mrs r10, cpsr
    cps #USR
	ldr sp, =usr_stacktop
    msr cpsr, r10

	/* And call the test starting C entrypoint */
	bl      tztest_start
	
end:	
    b      end
