#include "platform.h"

.align 5
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

    /* Start by setting up a temporary stack */
    ldr x11, =(RAM_BASE+RAM_SIZE)
	mov sp, x11

    /* Copy the text section to RAM */
    ldr x0, =_EL3_RAM_TEXT
    ldr x1, =_EL3_FLASH_TEXT
    ldr x2, =_EL3_TEXT_SIZE
    bl memcpy

    /* Copy the data section to RAM */
    ldr x0, =_EL3_RAM_DATA
    ldr x1, =_EL3_FLASH_DATA
    ldr x2, =_EL3_DATA_SIZE
    bl memcpy

    /* Branch to initialize EL3 mode. We pass the secure and non-secure EL1
     * entry points as it sets up the initial exception return to these modes
     * We don't come back from the branch to EL3 as it initiates the transition
     * to secure EL1 and sets up non-secure EL1 for the first SMC context
     * switch.
     */
    mov x0, SEC_FLASH_BASE
    mov x1, NSEC_FLASH_BASE
    ldr x11, =_EL3_RAM_TEXT
    blr x11

    /* We shouldn't get here as the secure image will shutdown */
end:
    b      end

memcpy:
    cbz x2, memcpy_done
    ldrb w10, [x1], #1
    strb w10, [x0], #1
    subs x2, x2, #1
    b memcpy
memcpy_done:
    ret
