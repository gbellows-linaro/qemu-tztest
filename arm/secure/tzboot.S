#include "platform.h"
#include "memory.h"

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
	@
	@ UART initialisation (38400 8N1)
	@
	ldr	r0,	=UART0_BASE     @ UART base (Versatile Express)
	mov	r1, #0x10			@ ibrd
	str	r1, [r0, #0x24]
	mov	r1, #0xc300
	orr	r1, #0x0001			@ cr
	str	r1, [r0, #0x30]

init_cpu:
    /* Disable interrupts for now */
    mrs r10, cpsr
    orr r10, r10, #0xc0     @ Mask IRQ and FIQ
    msr cpsr, r10

    /* Make sure boot vectors are based at 0 */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           @ SCTLR.V = 0
    mcr p15, 0, r10, c1, c0, 0

    /* Set up boot VBAR */
    mrc p15, 0, r11, c12, c0, 0
    ldr r11, =boot_vectors
    mcr p15, 0, r11, c12, c0, 0
    isb

    /* Start by setting up the boot stack*/
	ldr sp, =boot_stacktop

load_images:
    /* First copy the vector section text into RAM */
    ldr r0, =_flash_secvecs_start
    ldr r1, =_ram_secvecs_start
    ldr r2, =_secvecs_size
copy_secvecs:
    ldrb r10, [r0], #1
    strb r10, [r1], #1
    subs r2, r2, #1
    bne copy_secvecs

    /* Next copy the text section into RAM */
    ldr r0, =_flash_sectext_start
    ldr r1, =_ram_sectext_start
    ldr r2, =_sectext_size
copy_sectext:
    ldrb r10, [r0], #1
    strb r10, [r1], #1
    subs r2, r2, #1
    bne copy_sectext

    /* Last copy the text section into RAM */
    ldr r0, =_flash_secdata_start
    ldr r1, =_ram_secdata_start
    ldr r2, =_secdata_size
copy_secdata:
    ldrb r10, [r0], #1
    strb r10, [r1], #1
    subs r2, r2, #1
    bne copy_secdata

    /* Branch to the nonsecure image base.  It should be set-up so that this is
     * the image loader which returns the starting execution point.
     */
    mov r0, #EL1_NS_FLASH_BASE
    blx r0

    /* Branch to the starting point of the secure image in RAM which should
     * kick off initialization.
     */
    mov r1, #EL3_RAM_BASE
    bx r1

    /* We shouldn't get here as the secure image will shutdown */
end:
    b      end
