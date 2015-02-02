#include "platform.h"

.align 5
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
init_uart:
	/*
	 * UART initialisation (38400 8N1)
	 */
	ldr	x0,	=UART0_BASE     // UART base (Versatile Express)
	mov	x1, #0x10			// ibrd
	str	x1, [x0, #0x24]
	mov	x1, #0xc300
	orr	x1, x1, #0x0001			// cr
	str	x1, [x0, #0x30]

init_cpu:
    /* Disable interrupts for now */
    mrs x10, daif
    orr x10, x10, #0xc0     // Mask IRQ and FIQ
    msr daif, x10

    /* Set up boot VBAR */
    adr x11, boot_vectors
    msr vbar_el3, x11
    isb

    /* Start by setting up the boot stack*/
    ldr x11, =boot_stacktop
	mov sp, x11

load_secure:
copy_secvecs:
    /* First copy the vector section text into RAM */
    ldr x0, =_flash_secvecs_start
    ldr x1, =_ram_secvecs_start
    ldr x2, =_secvecs_size
    bl memcpy

copy_sectext:
    /* Next copy the text section into RAM */
    ldr x0, =_flash_sectext_start
    ldr x1, =_ram_sectext_start
    ldr x2, =_sectext_size
    bl memcpy

copy_secdata:
    /* Last copy the text section into RAM */
    ldr x0, =_flash_secdata_start
    ldr x1, =_ram_secdata_start
    ldr x2, =_secdata_size
    bl memcpy

load_nonsecure:
    /* Branch to the nonsecure image base.  It should be set-up so that this is
     * the image loader which returns the starting execution point.
     */
    mov x0, #FLASH_NSEC_BASE
    blr x0

    /* Branch to the starting point of the secure image in RAM which should
     * kick off initialization.
     */
    mov x1, #RAM_SEC_BASE
    br x1

    /* We shouldn't get here as the secure image will shutdown */
end:
    b      end

memcpy:
    cbz x2, memcpy_done
    ldrb w10, [x0], #1
    strb w10, [x1], #1
    subs x2, x2, #1
    b memcpy
memcpy_done:
    ret
