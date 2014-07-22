#include <arm32.h>

.arch_extension sec

.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
secure_vectors:
	.word 0	/* reset */
	b   secure_undef_vec /* undef */
	b   secure_svc_vec /* svc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

secure_undef_vec:
    bl sec_undef_handler
    mov pc, lr

secure_svc_vec:
    /* Check if this is a return from USR mode and pop the return address off
     * the stack.  Otherwise, route handling to the secure svc_handler.
     */
    cmp r0, #0x0
    bne 1f 
    pop {lr}
	movs pc, lr
1:
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
    mrc p15, 0, r11, c12, c0, 0
    ldr r11, =secure_vectors
    mcr p15, 0, r11, c12, c0, 0
    isb
    
init_secure_stacks:
    /* Set-up the secure SVC stack */
	ldr sp, =sec_svc_stacktop

    cps #CPSR_MODE_UND
	ldr sp, =sec_und_stacktop

    /* Have to set user (and system) stack from SYS mode so we can get back to
     * SVC.
     */
    cps #CPSR_MODE_SYS
	ldr sp, =sec_usr_stacktop

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
