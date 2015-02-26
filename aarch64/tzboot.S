#include "platform.h"

.align 12
.section .init
/* We use the same vector table for Hyp and Monitor mode, since
 * we will only use each once and they don't overlap.
 */
boot_vectors:
	b reset	/* reset */
	.word 0 /* undef */
	.word 0 /* svc */
	.word 0 /* pabt */
	.word 0 /* dabt */
	.word 0 /* hmc */
	.word 0 /* irq */
	.word 0 /* fiq */

reset:
    /* Disable interrupts for now */
    mrs x10, daif
    orr x10, x10, #0xc0     // Mask IRQ and FIQ
    msr daif, x10

    /* Set up VBAR */
    adr x11, boot_vectors
    msr vbar_el3, x11
    isb

init_uart:
	/* UART initialisation (38400 8N1) */
	ldr	x0,	=UART0_BASE     // UART base
	mov	x1, #0x10			// ibrd
	str	x1, [x0, #0x24]
	mov	x1, #0xc300
	orr	x1, x1, #0x0001	    // cr
	str	x1, [x0, #0x30]

    /* Branch to initialize EL3 mode. We pass the secure and non-secure EL1
     * entry points as it sets up the initial exception return to these modes
     * We don't come back from the branch to EL3 as it initiates the transition
     * to secure EL1 and sets up non-secure EL1 for the first SMC context
     * switch.
     */
    mov x0, EL1_S_FLASH_BASE
    mov x1, EL1_NS_FLASH_BASE
    ldr x11, =el3_init
    br x11

    /* We shouldn't get here as the secure image will shutdown */
end:
    b      end

.end
