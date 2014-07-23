#include <arm32.h>

.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
nonsecure_vectors:
	.word 0	/* reset */
	b   nonsecure_undef_vec /* undef */
	b	nonsecure_svc_vec /* svc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

nonsecure_undef_vec:
    srsdb sp!, #CPSR_MODE_UND
    bl nsec_undef_handler
    rfefd sp!

nonsecure_svc_vec:
    srsdb sp!, #CPSR_MODE_SVC
    bl nsec_svc_handler
    rfefd sp!

    .globl init_nonsecure
init_nonsecure:
    /* Disable interrupts for now */
    mrs r10, cpsr
    orr r10, r10, #0xc0     @ Mask IRQ and FIQ
    msr cpsr_all, r10

    /* Make sure vectors are based at 0 */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           @ SCTLR.V = 0
    mcr p15, 0, r10, c1, c0, 0

    /* Set up non-secure VBAR */
    mrc p15, 0, r11, c12, c0, 0
    ldr r11, =nonsecure_vectors
    mcr p15, 0, r11, c12, c0, 0
    isb

init_nonsecure_mmu:
    /* Disable data and instruction caches */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x0004
    bic r10, r10, #0x1000
    mcr p15, 0, r10, c1, c0, 0

    /* Set domain 0 & 1 for client access */
    mov r10, #0x5
    mcr p15, 0, r10, c3, c0, 0

    /* This function will return the initialized base address */
    ldr r0, =nsec_l1_page_table
    ldr r1, =nsec_ram_base

    /* Set TTBR0 to the initialized address plus enable shareable write-back 
     * write-allocate.
     */
    orr r0, r0, #0xB
    mcr p15, 0, r0, c2, c0, 0

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

init_nonsecure_stacks:
    /* Reset the SVC stack*/
	ldr sp, =nsec_svc_stacktop

    cps #CPSR_MODE_UND
	ldr sp, =nsec_und_stacktop

    /* Have to set user (and system) stack from SYS mode so we can get back to
     * SVC.
     */
    cps #CPSR_MODE_USR
	ldr sp, =nsec_usr_stacktop

	bl      tztest_nonsecure_usr_main
	
end:	
    b      end
