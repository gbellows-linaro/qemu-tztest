#define __ASSEMBLY__
#include "arm32.h"
#include "common_defs.h"
#undef __ASSEMBLY__

.section .vectors
.align 12
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

