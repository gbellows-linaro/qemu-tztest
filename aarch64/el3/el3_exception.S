.section .vectors
.align 12                   // Align to vector table size (0x800)
.globl el3_vectors
el3_vectors:
.word 0                     // Add padding to force the below alignment
.align 9                    // Force these vectors to 0x400 alignment
el3_sync_exception_current:
    stp x30, x2, [sp, #-16]!
    stp x0, x1, [sp, #-16]!
    mov x2, x0
    mrs x0, esr_el3
    mov x1, #0xffffff
    and x1, x1, x0
    lsr x0, x0, #26
    bl el3_handle_exception
    ldp x0, x1, [sp], #16
    ldp x30, x2, [sp], #16
    eret
.align 10                   // Force these vectors to 0x400 alignment
el3_sync_exception_lower64:
    stp x30, x2, [sp, #-16]!
    stp x0, x1, [sp, #-16]!
    mov x2, x0
    mrs x0, esr_el3
    mov x1, #0xffffff
    and x1, x1, x0
    lsr x0, x0, #26
    bl el3_handle_exception
    mov x3, x0
    ldp x0, x1, [sp], #16
    ldp x30, x2, [sp], #16
    cbz x3, el3_sync_exception_lower64_done
    b monitor_switch
el3_sync_exception_lower64_done:
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

.end
