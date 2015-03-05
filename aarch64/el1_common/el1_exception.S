.section .vectors
.align 12                   // Align to vector table size (0x800)
.globl el1_vectors
el1_vectors:
.word 0                     // Add padding to force the below alignment
.align 9                    // Force these vectors to 0x400 alignment
el1_sync_exception_current:
    mrs x0, esr_el1
    mov x1, #0xffffff
    and x1, x1, x0
    lsr x0, x0, #26
    mrs x2, far_el1
    mrs x3, elr_el1
    bl el1_handle_exception
    eret
.align 10                   // Force these vectors to 0x400 alignment
el1_sync_exception_lower64:
    mrs x0, esr_el1
    mov x1, #0xffffff
    and x1, x1, x0
    lsr x0, x0, #26
    mrs x2, far_el1
    mrs x3, elr_el1
    bl el1_handle_exception
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
