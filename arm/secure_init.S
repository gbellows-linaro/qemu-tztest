#include "tztest_asm.h"

.arch_extension sec

.align 5
secure_vectors:
	b   secure_init         /* reset */
	b   secure_undef_vec    /* undef */
	b   secure_svc_vec      /* svc */
	b   secure_pabort_vec   /* pabt */
	b   secure_dabort_vec   /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

secure_undef_vec:
    srsdb sp!, #CPSR_MODE_UND
    bl sec_undef_handler
    rfefd sp!

secure_pabort_vec:
    mrc p15, 0, r0, c5, c0, 1       /* IFSR */
    mrc p15, 0, r1, c6, c0, 2       /* IFAR */
    bl sec_pabort_handler
    b end

secure_dabort_vec:
    mrc p15, 0, r0, c5, c0, 0       /* DFSR */
    mrc p15, 0, r1, c6, c0, 0       /* DFAR */
    bl sec_dabort_handler
    b end

secure_svc_vec:
    /* Check if this is a return from USR mode and pop the return address off
     * the stack.  If so, we got here through the dispatch mechanism that
     * pushed the return on the stack which should be secure svc loop.  For
     * this reason we want to return to SVC mode and not a return from
     * exception.
     * Otherwise, route handling to the secure svc_handler.  This is the case
     * where we came from secure usr mode.
     */
    cmp r0, #SVC_RETURN_FROM_SECURE_USR
    bne 1f
    mov r0, r1
    pop {pc}
1:
    srsdb sp!, #CPSR_MODE_SVC
    bl sec_svc_handler
    rfefd sp!

.align 5
monitor_vectors:
	b	.		/* Reset			*/
	b	.		/* Undefined instruction	*/
	b	monitor_smc_vect /* Secure monitor call		*/
	b	.		/* Prefetch abort		*/
	b	.		/* Data abort			*/
	b	.		/* Reserved			*/
	b	.		/* IRQ				*/
	b	.       /* FIQ				*/

monitor_smc_vect:
    cmp r0, #SMC_YIELD
    bge sm_smc_entry
	srsdb sp!, #CPSR_MODE_MON
    cmp r0, #SMC_NOOP
    beq 1f
	push {r0-r3}
    mov r0, r1
    bl tztest_dispatch_monitor
	pop	{r0-r3}
1:  rfefd sp!

    .globl secure_init
secure_init:
    /* We enter with R0 pointing to the nonsecure entry point.  Put it in R11
     * for now to avoid overwriting it on calls. */
    mov r11, r0

    /* Disable interrupts for now */
    mrs r10, cpsr
    orr r10, r10, #0xc0     @ Mask IRQ and FIQ
    msr cpsr, r10

    /* Make sure secure vectors are based at 0 */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           @ SCTLR.V = 0
    mcr p15, 0, r10, c1, c0, 0

    /* Set up secure VBAR */
    ldr r10, =secure_vectors
    mcr p15, 0, r10, c12, c0, 0
    isb

secure_stack_init:
    /* Set-up the secure SVC stack */
	ldr sp, =sec_svc_stacktop

    cps #CPSR_MODE_UND
	ldr sp, =sec_und_stacktop

    cps #CPSR_MODE_ABT
	ldr sp, =sec_abt_stacktop

    /* Have to set user (and system) stack from SYS mode so we can get back to
     * SVC.
     */
    cps #CPSR_MODE_SYS
	ldr sp, =sec_usr_stacktop

    cps #CPSR_MODE_SVC

secure_mmu_init:
    /* Disable data and instruction caches */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x0004
    bic r10, r10, #0x1000
    mcr p15, 0, r10, c1, c0, 0

    /* Set domain 0 & 1 for client access */
    mov r10, #0x5
    mcr p15, 0, r10, c3, c0, 0

    /* Set-up the initial secure page tables */
    bl tztest_secure_pagetable_init

    /* Set TTBR0 to the initialized address plus enable shareable write-back
     * write-allocate.
     */
    ldr r10, =_sec_l1_page_table
    orr r10, r10, #0xB
    mcr p15, 0, r10, c2, c0, 0

    /* Set-up the table base control to split between TTBR0/1 (N = 0) */
    mov r10, #0x0
    mcr p15, 0, r10, c2, c0, 2

    /* Enable the mmu */
    mcr p15, 0, r10, c8, c7, 0
    mrc p15, 0, r10, c1, c0, 0
    orr r10, r10, #0x1
    mcr p15, 0, r10, c1, c0, 0
    isb
    dsb

#ifdef DEBUG
    /* Check that are entry state makes sense before initializing the monitor
     * mode.
     */
    bl check_init_mode
#endif

secure_init_monitor:
    /* Set-up the monitor stack */
	mrs	r10, cpsr
    cps #CPSR_MODE_MON
    ldr sp, =mon_stacktop
	msr	cpsr, r10

    /* Set-up mvbar */
    ldr r10, =monitor_vectors
    mcr p15, 0, r10, c12, c0, 1

    /* Call to set the nonsecure entry point to be the save r0 value passed in
     * by the boot code.  This will be the initial entry point on the first
     * monitor context switch.
     */
    mov r0, r11
    bl tztest_secure_svc_init_monitor

    mov r0, #SMC_YIELD
    smc #0

    bl tztest_secure_svc_loop

	/* If we get here we are on the way out, poweroff the device */
end:
    ldr r0, =SYSREG_BASE
    add r0, r0, #SYSREG_CFGCTRL
    ldr r1, =SYS_SHUTDOWN
    str r1, [r0]
    b end

    .globl dispatch_secure_usr
dispatch_secure_usr:
    /* Push the LR onto the stack so we can return immediately from the svc
     * handler on return.
     */
    push {lr}

    /* Reset the user stack for the dispatch, so we have a clean stack */
    cps #CPSR_MODE_SYS
	ldr sp, =sec_usr_stacktop
    push {r0, r1}
    cps #CPSR_MODE_SVC

    /* Return to USR mode to execute the dispatch function in r0
     */
    ldr r0, =dispatch_usr_wrapper
    mov r1, #CPSR_MODE_USR
    push {r0, r1}
    rfefd sp!

/* This is the user-side dispatch wrapper that accepts the function pointer and
 * function arg in r0 & r1 respectively.  The wrapper is simply an anstraction
 * layer for translating the input parameters into the user function call.
 */
dispatch_usr_wrapper:
    pop {r0, r1}
    mov r2, r0
    mov r0, r1
    mov pc, r2
