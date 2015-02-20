#include "common_defs.h"

.align 11                   // Align to vector table size (0x800)
.section .vectors
el3_vectors:
.word 0                     // Add padding to force the below alignment
.align 10                   // Force these vectors to 0x400 alignment
el3_sync_exception:
    mrs x0, esr_el3
    mov x1, #0xffffff
    and x1, x1, x0
    lsr x0, x0, #26
    bl el3_handle_exception
    eret
.align 7
el3_serr_exception:
	b	el3_serr_exception
.align 7
el3_irq_exception:
	b	el3_irq_exception
.align 7
el3_fiq_exception:
	b	el3_fiq_exception

monitor_smc_vect:
//    cmp x0, #SMC_YIELD
//    bge sm_smc_entry
    cmp x0, #SMC_NOOP
    beq 1f
	stp x2, x3, [sp, #-16]!
	stp x1, x0, [sp, #-16]!
    mov x0, x1
    bl monitor_dispatch
	ldp x0, x1, [sp], #16
	ldp x2, x3, [sp], #16
1:  eret

.align 12
.section .text
.global el3_init
/* el3_init(sec_elr, nsec_elr) */
el3_init:
    /* Set-up the EL3 vbar */
    ldr x10, =el3_vectors
    msr vbar_el3, x10

    /* Set-up the monitor stack */
    ldr x10, =EL3_STACK_BASE
	mov sp, x10

    /* Save the incoming LR so we can return to the right place */
    str x30, [sp, #-8]!
    stp x0, x1, [sp, #-16]!

    /* Enable floating point register usage as printf uses it */
    mrs x10, cptr_el3
    and x10, x10, #~0x400       /* CPTR.TFP */
    msr cptr_el3, x10

el3_init_mmu:
    /* Disable data and instruction caches */
    mrs x10, sctlr_el3
    bic x10, x10, #0x0004
    bic x10, x10, #0x1000
    msr sctlr_el3, x10

    bl el3_pagetable_init

    /* Set TTBR0 to the initialized address plus enable shareable write-back
     * write-allocate.
     */
    ldr x10, =EL3_PGTBL_BASE
    msr ttbr0_el3, x10

    mrs x10, sctlr_el3
    orr x10, x10, #0x1      // Enable MMU
    msr sctlr_el3, x10
    isb
    dsb sy

el3_init_monitor:
    ldp x0, x1, [sp], #16
    mov x4, #5
    adr x2, sec_state
    stp x0, x4, [x2], #16        /* S.ELR_EL3 = sec_base */
    adr x3, nsec_state
    stp x1, x4, [x3], #16        /* NS.ELR_EL3 = nsec_base */
    mov x0, #0
    mov x1, #31
zero_loop:
    sub x1, x1, #1
    str x0, [x2], #8
    str x0, [x3], #8
    cbnz x1, zero_loop
    adr x0, sec_state
    bl monitor_restore_state
    eret

.end
