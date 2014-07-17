
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

.bss
.align 5
    secure_context_base:  
        .word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    secure_context_end:

    nonsecure_context_base:
        .word 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    nonsecure_context_end:

    secure_context:       .word 0
    nonsecure_context:    .word 0 

.text
.align 5
mon_vectors:
	.word 0	/* reset */
	.word 0	/* undef */
	b  	smc_vec /* smc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

smc_vec:
    /* Save the potential smc args on the monitor stack */
    push {r0-r3}

    /* Check what secure state we are coming from */
    mrc p15, 0, r0, c1, c1, 0
    tst r0, #0x1
    eor r0, r0, #0x1
    mcr p15, 0, r0, c1, c1, 0
    beq smc_from_secure

smc_from_nonsecure:
    /* Coming from nonsecure, restoring secure */
    ldr r2, =nonsecure_context
    ldr r3, =secure_context
    b smc_switch

smc_from_secure:
    /* Coming from secure, restoring nonsecure */
    ldr r2, =secure_context
    ldr r3, =nonsecure_context

smc_switch:
    ldr r0, [r2]    // Departing stack
    stmfd r0!, {r4-r12}
    mrs r1, sp_svc
    stmfd r0!, {r1}
    mrs r1, spsr
    stmfd r0!, {r1, lr}
    str r0, [r2]
    
    ldr r0, [r3]    // Target stack
    ldmfd r0!, {r1, lr}
    msr spsr, r1
    ldmfd r0, {r1}
    msr sp_svc, r1
    ldmfd r0!, {r4-r12}
    str r0, [r3]

    /* Restore incoming smc args for switched context to use */
    pop {r0-r3}

    movs pc, lr

/************************************************************/

    .globl init_monitor
init_monitor:
    /* Set up the monitor vector table */
    ldr r1, =mon_vectors
    mcr p15, 0, r1, c12, c0, 1
    
    /* Set up secure state stack and save current context */
    ldr r1, =secure_context_end
    stmfd r1!, {r4-r12}
    stmfd r1!, {sp}
    mrs r2, cpsr
    stmfd r1!, {r2, lr}

    /* Switch to monitor mode */
    cps #MON

    /* Set up the monitor mode stack */
    ldr sp, =mon_stacktop

    /* Save secure context state position */
    ldr r2, =secure_context
    str r1, [r2]

    /* Set up the nonsecure state stack in preparation for switch */
    ldr r1, =nonsecure_context_end
    ldr r2, =nonsecure_context
    str r1, [r2]

    /* Switch to nonsecure state */
    mrc p15, 0, r1, c1, c1, 0
    orr r1, #NONSECURE
    mcr p15, 0, r1, c1, c1, 0

    /* Setup to return to the specified SVC starting point (r0) */
    mov lr, r0
    msr spsr, #SVC

    /* Clear the registers */
    mov r0, #0
    mov r1, #0
    mov r2, #0
    mov r3, #0
    mov r4, #0
    mov r5, #0
    mov r6, #0
    mov r7, #0
    mov r8, #0
    mov r9, #0
    mov r10, #0
    mov r11, #0
    mov r12, #0

    movs pc, lr

