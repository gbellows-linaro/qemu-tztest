#include "arm32.h"
#include "common_defs.h"

.section .vectors, "ax"
.globl el1_vectors
el1_vectors:
	.word 0                 /* reset */
	b   el1_undef_vec    /* undef */
	b   el1_svc_vec      /* svc */
	b   el1_pabort_vec   /* pabt */
	b   el1_dabort_vec   /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

el1_undef_vec:
    srsdb sp!, #CPSR_MODE_UND
    push {r10}
    ldr r10, =el1_handle_exception
    blx r10
    pop {r10}
    rfefd sp!

el1_pabort_vec:
    mrc p15, 0, r0, c5, c0, 1       /* IFSR */
    mrc p15, 0, r1, c6, c0, 2       /* IFAR */
    bl el1_handle_exception

el1_dabort_vec:
    mrc p15, 0, r0, c5, c0, 0       /* DFSR */
    mrc p15, 0, r1, c6, c0, 0       /* DFAR */
    bl el1_handle_exception

el1_svc_vec:
    /* Check if this is a return from USR mode and pop the return address off
     * the stack.  If so, we got here through the dispatch mechanism that
     * pushed the return on the stack which should be el1 svc loop.  For
     * this reason we want to return to SVC mode and not a return from
     * exception.
     * Otherwise, route handling to the el1 svc_handler.  This is the case
     * where we came from el1 usr mode.
     */
    cmp r0, #SVC_RETURN_FROM_SECURE_USR
    bne 1f
    mov r0, r1
    pop {pc}
1:
    srsdb sp!, #CPSR_MODE_SVC
    push {r10}
    ldr r10, =el1_handle_svc
    blx r10
    pop {r10}
    rfefd sp!

