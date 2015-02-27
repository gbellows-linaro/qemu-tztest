#include "common_defs.h"

.section .text
.globl monitor_init
monitor_init:
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

.globl monitor_save_state
monitor_save_state:
    stp x0, x1, [sp, #-16]!     /* Save the input regs x0:x3 */
    stp x2, x3, [sp, #-16]!
    mrs x1, elr_el3
    mrs x2, spsr_el3
    stp x1, x2, [x0], #16
    mrs x1, sp_el0
    mrs x2, sp_el1
    stp x1, x2, [x0], #16
    mrs x1, elr_el1
    mrs x2, esr_el1
    stp x1, x2, [x0], #16
    stp x4, x5, [x0], #16
    stp x6, x7, [x0], #16
    stp x8, x9, [x0], #16
    stp x10, x11, [x0], #16
    stp x12, x13, [x0], #16
    stp x14, x15, [x0], #16
    stp x16, x17, [x0], #16
    stp x18, x19, [x0], #16
    stp x20, x21, [x0], #16
    stp x22, x23, [x0], #16
    stp x24, x25, [x0], #16
    stp x26, x27, [x0], #16
    stp x28, x29, [x0], #16
    str x30, [x0], #8
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16
    ret

.globl monitor_restore_state
monitor_restore_state:
    str x30, [sp, #-8]!         /* Save the LR onto the stack for later */
    stp x0, x1, [sp, #-16]!
    ldp x1, x2, [x0], #16
    msr elr_el3, x1
    msr spsr_el3, x2
    ldp x1, x2, [x0], #16
    msr sp_el0, x1
    msr sp_el1, x2
    ldp x1, x2, [x0], #16
    msr elr_el1, x1
    msr esr_el1, x2
    ldp x4, x5, [x0], #16
    ldp x6, x7, [x0], #16
    ldp x8, x9, [x0], #16
    ldp x10, x11, [x0], #16
    ldp x12, x13, [x0], #16
    ldp x14, x15, [x0], #16
    ldp x16, x17, [x0], #16
    ldp x18, x19, [x0], #16
    ldp x20, x21, [x0], #16
    ldp x22, x23, [x0], #16
    ldp x24, x25, [x0], #16
    ldp x26, x27, [x0], #16
    ldp x28, x29, [x0], #16
    ldr x30, [x0], #8
    ldp x0, x1, [sp], #16
    ldr x3, [sp], #8            /* Restore the LR so we can jump back */
    br x3

.globl monitor_switch
monitor_switch:
    stp x0, x1, [sp, #-16]!     /* Save the input regs x0:x3 */
    mrs x1, scr_el3
    and x1, x1, #0x1
    cbnz x1, switch_to_nsec
switch_to_sec:
    adr x0, sec_state
    bl monitor_save_state
    adr x0, nsec_state
    bl monitor_restore_state
    b switch_state
switch_to_nsec:
    adr x0, nsec_state
    bl monitor_save_state
    adr x0, sec_state
    bl monitor_restore_state
switch_state:
    eor x1, x1, #0x1
    msr scr_el3, x1
    ldp x0, x1, [sp], #16
    eret

.end
