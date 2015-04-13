#define __ASSEMBLY__
#include "exception.h"
#undef __ASSEMBLY__

.section .vectors
.align 12                   // Align to vector table size (0x800)
.globl el1_vectors
el1_vectors:
.word 0                     // Add padding to force the below alignment
.align 9                    // Force these vectors to 0x400 alignment
el1_sync_exception_current:
    str x30, [sp, #-8]!
    stp x2, x3, [sp, #-16]!
    stp x0, x1, [sp, #-16]!
    mov x2, x0;
    mrs x0, esr_el1
    mov x1, #0xffffff
    and x1, x1, x0
    lsr x0, x0, #26
    mrs x2, far_el3
    mrs x3, elr_el3
    bl el1_handle_exception
    ldp x0, x1, [sp], #16
    ldp x2, x3, [sp], #16
    ldr x30, [sp], #8
    eret
.align 10                   // Force these vectors to 0x400 alignment
el1_sync_exception_lower64:
    str x30, [sp, #-8]!
    stp x2, x3, [sp, #-16]!
    stp x0, x1, [sp, #-16]!
    mrs x0, esr_el1
    mov x1, #0xffffff
    and x1, x1, x0
    lsr x0, x0, #26
    cmp x0, #EC_SVC64
    b.eq el1_sync_exception_lower64_svc
    cmp x0, #EC_SVC32
    b.eq el1_sync_exception_lower64_svc
    mrs x2, far_el1
    mrs x3, elr_el1
    bl el1_handle_exception
    ldp x0, x1, [sp], #16
    ldp x2, x3, [sp], #16
    b el1_sync_exception_lower64_done
el1_sync_exception_lower64_svc:
    ldp x0, x1, [sp]        /* Fetch copies of our inputs as SVC args */
    bl el1_handle_svc
    ldp x2, x1, [sp], #16   /* We don't want to overwrite x0, so use x2 */
    ldp x2, x3, [sp], #16   /* We can throw away the old x0, and restore x2 */
el1_sync_exception_lower64_done:
    ldr x30, [sp], #8
    eret
.align 7
el1_serr_exception:
	b	el1_serr_exception
.align 7
el1_irq_exception:
	b	el1_irq_exception
.align 7
el1_fiq_exception:
	b	el1_fiq_exception

.end
