#include <arm32.h>

.arch_extension sec

.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
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
    bl sec_undef_handler
    mov pc, lr

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
    cmp r0, #0x0
    bne 1f 
    pop {pc}
1:
    srsdb sp!, #CPSR_MODE_SVC
    bl sec_svc_handler
    rfefd sp!

    .globl secure_init
secure_init:
    /* Disable interrupts for now */
    mrs r10, cpsr
    orr r10, r10, #0xc0     @ Mask IRQ and FIQ
    msr cpsr, r10

    /* Make sure secure vectors are based at 0 */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           @ SCTLR.V = 0
    mcr p15, 0, r10, c1, c0, 0

    /* Set up secure VBAR */
    ldr r11, =secure_vectors
    mcr p15, 0, r11, c12, c0, 0
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

    /* This function will return the initialized base address */
    ldr r10, =sec_l1_page_table
    mov r0, r10
    ldr r1, =sec_ram_base
    bl pagetable_init

    /* Set TTBR0 to the initialized address plus enable shareable write-back 
     * write-allocate.
     */
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

secure_init_monitor:
    cps #CPSR_MODE_SVC
    bl tztest_secure_svc_init_monitor

    mov r0, #0x0
    smc #0

    bl tztest_secure_svc_loop

	/* If we get here we are on the way out */
end:	
    b      end

    .globl dispatch_secure_usr
dispatch_secure_usr:
    /* Push the LR onto the stack so we can return immediately from the svc
     * handler on return.  
     */
    push {lr}

    /* Reset the user stack for the dispatch, so we have a clean stack */
    cps #CPSR_MODE_SYS
	ldr sp, =sec_usr_stacktop
    cps #CPSR_MODE_SVC

    /* Return to USR mode to execute the dispatch function in r0
     */
    mov r1, #CPSR_MODE_USR
    push {r0, r1}
    rfefd sp!
