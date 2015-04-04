#define __ASSEMBLY__
#include "arm32.h"
#include "common_defs.h"
#undef __ASSEMBLY__

.section .vectors
.align 12
.global monitor_vectors
monitor_vectors:
	b	.		/* Reset			*/
	b	monitor_handle_exception /* Undefined instruction	*/
	b	monitor_smc_vect /* Secure monitor call		*/
	b	monitor_handle_exception    /* Prefetch abort		*/
	b	monitor_handle_exception    /* Data abort			*/
	b	.		/* Reserved			*/
	b	.		/* IRQ				*/
	b	.       /* FIQ				*/

monitor_smc_vect:
	srsdb sp!, #CPSR_MODE_MON
    bl el3_handle_smc
    cmp r0, #0
    beq monitor_smc_vect_done
    b monitor_switch
monitor_smc_vect_done:
    rfefd sp!

monitor_handle_exception:
    push {r0-r3}
    bl el3_handle_exception
    pop {r0-r3}
    rfefd sp!
