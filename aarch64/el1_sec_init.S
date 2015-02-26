#include "common_defs.h"
#include "platform.h"

.section .vectors
secure_vectors:
	.word 0        /* reset */
	b   secure_undef_vec    /* undef */
	b   secure_svc_vec      /* svc */
	b   secure_pabort_vec   /* pabt */
	b   secure_dabort_vec   /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

secure_undef_vec:
//    ldr x10, =secure_undef_handler
//    blr x10
    eret

secure_pabort_vec:
//    bl secure_pabort_handler
//    b end

secure_dabort_vec:
//    bl secure_dabort_handler
//    b end

secure_svc_vec:
    /* Check if this is a return from USR mode and pop the return address off
     * the stack.  If so, we got here through the dispatch mechanism that
     * pushed the return on the stack which should be secure svc loop.  For
     * this reason we want to return to SVC mode and not a return from
     * exception.
     * Otherwise, route handling to the secure svc_handler.  This is the case
     * where we came from secure usr mode.
     */
    cmp x0, #SVC_RETURN_FROM_SECURE_USR
    bne 1f
    mov x0, x1
    ret
1:
//    ldr x10, =secure_svc_handler
//    blr x10
    eret

.section .init
secure_init:
    /* We enter with R0 pointing to the nonsecure entry point.  Put it in R11
     * for now to avoid overwriting it on calls. */
    mov x11, x0

    /* Disable interrupts for now */
    mrs x10, daif
    orr x10, x10, #0xc0     /* Mask IRQ and FIQ */
    msr daif, x10

    /* Setup the secure EL1 vectors
     */
    ldr x10, =secure_vectors
    msr vbar_el1, x10

    isb

secure_stack_init:
    /* Set-up the secure SVC stack */
	ldr x10, =SEC_STACK_BASE
	mov sp, x10

    smc #SMC_YIELD

secure_mmu_init:
    /* Disable data and instruction caches */
    mrs x10, sctlr_el3
    bic x10, x10, #0x0004
    bic x10, x10, #0x1000
    msr sctlr_el3, x10

    /* Set-up the initial secure page tables */
//    bl secure_pagetable_init

    /* Set TTBR0 to the initialized address plus enable shareable write-back
     * write-allocate.
     */
    ldr x10, =SEC_PGTBL_BASE
    orr x10, x10, #0x8
    orr x10, x10, #0x2
    orr x10, x10, #0x1
    msr ttbr0_el3, x10

    /* Enable the mmu */
//    tlbi alle3
    mrs x10, sctlr_el3
    orr x10, x10, #0x1
    msr sctlr_el3, x10
    isb
    dsb sy

check_secure:
    /* Check that are entry state makes sense before initializing the monitor
     * mode.
     */
//    bl secure_check_init

secure_init_monitor:
    /* Call monitor mode initialization and pass to it the non-secure execution
     * entry point.
     */
    mov x0, x11
//    bl monitor_init

    /* Once monitor mode is set-up, we yield to non-secure execution.  The
     * non-secure entrypoint was established in monitor init.
     */
    mov x0, #SMC_YIELD
    smc #1

    /* The first return to the secure world will set us off into our
     * secure-side monitor loop.  The only way out is to issue an EXIT SMC call
     * to the secure world.
     */
//    bl secure_svc_loop

	/* If we get here we are on the way out, poweroff the device */
end:
//    b secure_shutdown
b end

