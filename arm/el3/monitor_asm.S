#include "arm32.h"
#include "memory.h"
#include "common_defs.h"

.global monitor_init
/* monitor_init(nonsecure_entrypt) */
monitor_init:
    push {lr}

    /* Set-up the monitor stack */
	mrs	r10, cpsr
    cps #CPSR_MODE_MON
    ldr sp, =EL3_STACK_BASE
	msr	cpsr, r10

    /* Set-up mvbar */
    ldr r10, =monitor_vectors
    mcr p15, 0, r10, c12, c0, 1

    /* Call to set the nonsecure entry point to be the save r0 value passed in
     * by the boot code.  This will be the initial entry point on the first
     * monitor context switch.
     */
    bl monitor_init_ns_entry

    pop {pc}

