#define __ASSEMBLY__
#include "smc.h"
#include "arm32.h"

.section .text

.globl monitor_restore_state
monitor_restore_state:
    push {lr}
    cps #CPSR_MODE_SYS
    ldmia r0!, {sp, lr}

    cps #CPSR_MODE_IRQ
    ldmia r0!, {r3, sp, lr}
	msr	spsr, r3

    cps #CPSR_MODE_SVC
    ldmia r0!, {r3, sp, lr}
	msr	spsr, r3

    cps #CPSR_MODE_ABT
    ldmia r0!, {r3, sp, lr}
	msr	spsr, r3

    cps #CPSR_MODE_UND
    ldmia r0!, {r3, sp, lr}
	msr	spsr, r3

    cps #CPSR_MODE_MON
    ldmia r0!, {r1, r2}
    ldmia r0!, {r4-r12}

    pop {r3}
    mov r0, r2
    push {r0, r1}
    bx r3

.globl monitor_save_state
monitor_save_state:
    push {lr}
    cps #CPSR_MODE_SYS
    stmia r0, {sp, lr}

    cps #CPSR_MODE_IRQ
    mrs r3, spsr
    stmia r0, {r3, sp, lr}

    cps #CPSR_MODE_SVC
    mrs r3, spsr
    stmia r0, {r3, sp, lr}

    cps #CPSR_MODE_ABT
    mrs r3, spsr
    stmia r0, {r3, sp, lr}

    cps #CPSR_MODE_UND
    mrs r3, spsr
    stmia r0, {r3, sp, lr}

    cps #CPSR_MODE_MON
    stmia r0, {sp, lr}
    stmia r0, {r4-r12}

    pop {lr}
    bx lr

.globl monitor_switch
monitor_switch:
    push {r0, r1}
    mrc p15, 0, r3, c1, c1, 0       /* Read SCR */
    and r3, r3, #0x1                /* Mask off NS bit */
    cmp r3, #0
    beq switch_to_nsec
switch_to_sec:
    ldr r0, =nsec_state
    bl monitor_save_state
    ldr r0, =sec_state
    bl monitor_restore_state
    b switch_state
switch_to_nsec:
    ldr r0, =sec_state
    bl monitor_save_state
    ldr r0, =nsec_state
    bl monitor_restore_state
switch_state:
    mrc p15, 0, r3, c1, c1, 0       /* Read SCR */
    eor r3, r3, #0x1
    mcr p15, 0, r3, c1, c1, 0       /* Write opposite SCR */
    pop {r2, r3}
    pop {r0, r1}
    push {r2, r3}
    rfefd sp


