#include "arm32.h"
#include "common_defs.h"

.arch_extension sec

.section .vectors, "ax"
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
    push {r10}
    ldr r10, =secure_undef_handler
    blx r10
    pop {r10}
    rfefd sp!

secure_pabort_vec:
    mrc p15, 0, r0, c5, c0, 1       /* IFSR */
    mrc p15, 0, r1, c6, c0, 2       /* IFAR */
    bl secure_pabort_handler
    b end

secure_dabort_vec:
    mrc p15, 0, r0, c5, c0, 0       /* DFSR */
    mrc p15, 0, r1, c6, c0, 0       /* DFAR */
    bl secure_dabort_handler
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
    push {r10}
    ldr r10, =secure_svc_handler
    blx r10
    pop {r10}
    rfefd sp!

.section .text
secure_init:
    /* We enter with R0 pointing to the nonsecure entry point.  Put it in R11
     * for now to avoid overwriting it on calls. */
    mov r11, r0

    /* Disable interrupts for now */
    mrs r10, cpsr
    orr r10, r10, #0xc0     /* Mask IRQ and FIQ */
    msr cpsr, r10

    /* During startup, before the MMU is enabled we'll set-up the vecs in low
     * mem.
     */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           /* SCTLR.V = 0 */
    mcr p15, 0, r10, c1, c0, 0
    ldr r10, =secure_vectors
    mcr p15, 0, r10, c12, c0, 0     /* VBAR = secure_vectors */

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
    bl secure_pagetable_init

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

secure_remap_vecs:
    /* Now that we have enabled the MMU, move the mapping of the secure vector
     * table to high mem.
     * No need to set up VBAR as we'll be using hivec */
    mrc p15, 0, r10, c1, c0, 0
    orr r10, r10, #0x2000           @ SCTLR.V = 1
    mcr p15, 0, r10, c1, c0, 0

    /* Check that are entry state makes sense before initializing the monitor
     * mode.
     */
    bl secure_check_init

secure_init_monitor:
    /* Call monitor mode initialization and pass to it the non-secure execution
     * entry point.
     */
    mov r0, r11
    bl monitor_init

    /* Once monitor mode is set-up, we yield to non-secure execution.  The
     * non-secure entrypoint was established in monitor init.
     */
    mov r0, #SMC_YIELD
    smc #0

    /* The first return to the secure world will set us off into our
     * secure-side monitor loop.  The only way out is to issue an EXIT SMC call
     * to the secure world.
     */
    bl secure_svc_loop

	/* If we get here we are on the way out, poweroff the device */
end:
    b secure_shutdown

