#include "arm32.h"
#include "common_defs.h"

.align 5
.global monitor_vectors
monitor_vectors:
	b	.		/* Reset			*/
	b	.		/* Undefined instruction	*/
	b	monitor_smc_vect /* Secure monitor call		*/
	b	.		/* Prefetch abort		*/
	b	.		/* Data abort			*/
	b	.		/* Reserved			*/
	b	.		/* IRQ				*/
	b	.       /* FIQ				*/

monitor_smc_vect:
    cmp r0, #SMC_YIELD
    bge sm_smc_entry
	srsdb sp!, #CPSR_MODE_MON
    cmp r0, #SMC_NOOP
    beq 1f
	push {r0-r3}
    mov r0, r1
    bl monitor_dispatch
	pop	{r0-r3}
1:  rfefd sp!

.global monitor_init
/* monitor_init(nonsecure_entrypt) */
monitor_init:
    push {lr}

    /* Set-up the monitor stack */
	mrs	r10, cpsr
    cps #CPSR_MODE_MON
    ldr sp, =mon_stacktop
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

