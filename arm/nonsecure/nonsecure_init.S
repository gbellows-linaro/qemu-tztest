#include "common_defs.h"
#include "arm32.h"

/* The nonsecure image load code must be placed at the beginning of the image
 * as this is the starting point for the boot code to load.  The loader code
 * will pass the initial starting point back to the boot code for later
 * execution.
 */
nonsecure_load:
    ldr r0, =_flash_nsectext_start
    ldr r1, =_ram_nsectext_start
    ldr r2, =_nsectext_size
copy_nsectext:
    ldrb r10, [r0], #1
    strb r10, [r1], #1
    subs r2, r2, #1
    bne copy_nsectext

    ldr r0, =_flash_nsecdata_start
    ldr r1, =_ram_nsecdata_start
    ldr r2, =_nsecdata_size
copy_nsecdata:
    ldrb r10, [r0], #1
    strb r10, [r1], #1
    subs r2, r2, #1
    bne copy_nsecdata

    ldr r0, =nonsecure_init
    mov pc, lr

.align 5
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
nonsecure_vectors:
	b   nonsecure_init          /* reset */
	b   nonsecure_undef_vec     /* undef */
	b	nonsecure_svc_vec       /* svc */
	b   nonsecure_pabort_vec    /* pabt */
	b   nonsecure_dabort_vec    /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

nonsecure_undef_vec:
    srsdb sp!, #CPSR_MODE_UND
    bl nonsecure_undef_handler
    rfefd sp!

nonsecure_pabort_vec:
    mrc p15, 0, r0, c5, c0, 1       /* IFSR */
    mrc p15, 0, r1, c6, c0, 2       /* IFAR */
    bl nonsecure_pabort_handler
    b end

nonsecure_dabort_vec:
    mrc p15, 0, r0, c5, c0, 0       /* DFSR */
    mrc p15, 0, r1, c6, c0, 0       /* DFAR */
    bl nonsecure_dabort_handler
    b end

nonsecure_svc_vec:
    srsdb sp!, #CPSR_MODE_SVC
    bl nonsecure_svc_handler
    rfefd sp!

nonsecure_init:
    /* Disable interrupts for now */
    mrs r10, cpsr
    orr r10, r10, #0xc0     @ Mask IRQ and FIQ
    msr cpsr_all, r10

    /* Make sure vectors are based at 0 */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           @ SCTLR.V = 0
    mcr p15, 0, r10, c1, c0, 0

    /* Set up non-secure VBAR */
    ldr r11, =nonsecure_vectors
    mcr p15, 0, r11, c12, c0, 0
    isb

nonsecure_stack_init:
    /* Reset the SVC stack*/
	ldr sp, =nsec_svc_stacktop

    cps #CPSR_MODE_UND
	ldr sp, =nsec_und_stacktop

    cps #CPSR_MODE_ABT
	ldr sp, =nsec_abt_stacktop

    /* Have to set user (and system) stack from SYS mode so we can get back to
     * SVC.
     */
    cps #CPSR_MODE_SYS
	ldr sp, =nsec_usr_stacktop

    cps #CPSR_MODE_SVC

nonsecure_mmu_init:
    /* Disable data and instruction caches */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x0004
    bic r10, r10, #0x1000
    mcr p15, 0, r10, c1, c0, 0

    /* Set domain 0 & 1 for client access */
    mov r10, #0x5
    mcr p15, 0, r10, c3, c0, 0

    /* This function will return the initialized base address */
    bl nonsecure_pagetable_init

    /* Set TTBR0 to the initialized address plus enable shareable write-back
     * write-allocate.
     */
    ldr r10, =_nsec_l1_page_table
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

    cps #CPSR_MODE_USR

	bl      tztest_nonsecure_usr_main

end:
    mov r0, #SVC_EXIT
    svc #0
    b      end
