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
	b	init_secure   	@ Must be first instruction for power on reset vector

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

undef_vec:
    b undef_handler
    b end

secure_svc_vec:
    b svc_handler
	mov pc, lr

/* Test boot starting point */
init_secure:
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
	ldr sp, =svc_stacktop

    /* Disable interrupts for now */
    mrs r10, cpsr
    orr r10, r10, #0xc0     @ Mask IRQ and FIQ
    msr cpsr, r10

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

init_secure_stacks:
    /* Switch to system mode */
    cps #SYS

    /* Set up the SYS stack */
	ldr sp, =sys_stacktop

    /* Switch to system mode */
    cps #SVC

    ldr lr, =tztest_start
    ldr r0, =init_nonsecure
    b init_monitor

end:
    b end
