#include "arm32.h"
#include "common_defs.h"

.section .vectors, "ax"
.globl secure_vectors
secure_vectors:
	.word 0                 /* reset */
	b   secure_undef_vec    /* undef */
	b   secure_svc_vec      /* svc */
	b   secure_pabort_vec   /* pabt */
	b   secure_dabort_vec   /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

secure_undef_vec:
    srsdb sp!, #CPSR_MODE_UND
    push {r10}
    ldr r10, =secure_undef_handler
    blx r10
    pop {r10}
    rfefd sp!

secure_pabort_vec:
    mrc p15, 0, r0, c5, c0, 1       /* IFSR */
    mrc p15, 0, r1, c6, c0, 2       /* IFAR */
    bl secure_pabort_handler

secure_dabort_vec:
    mrc p15, 0, r0, c5, c0, 0       /* DFSR */
    mrc p15, 0, r1, c6, c0, 0       /* DFAR */
    bl secure_dabort_handler

secure_svc_vec:
    /* Check if this is a return from USR mode and pop the return address off
     * the stack.  If so, we got here through the dispatch mechanism that
     * pushed the return on the stack which should be secure svc loop.  For
     * this reason we want to return to SVC mode and not a return from
     * exception.
     * Otherwise, route handling to the secure svc_handler.  This is the case
     * where we came from secure usr mode.
     */
    cmp r0, #SVC_RETURN_FROM_SECURE_USR
    bne 1f
    mov r0, r1
    pop {pc}
1:
    srsdb sp!, #CPSR_MODE_SVC
    push {r10}
    ldr r10, =secure_svc_handler
    blx r10
    pop {r10}
    rfefd sp!

