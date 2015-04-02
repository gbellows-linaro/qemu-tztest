#include "arm32.h"
#include "platform.h"
#include "memory.h"
#include "common_defs.h"

.arch_extension sec

#define TEMP_STACK 0x40002000
#define PA_POOL_BASE 0x4C041000

.section .init
secure_init:
init_uart:
	/* UART initialisation (38400 8N1) */
	ldr	r0,	=UART0_BASE     // UART base (Versatile Express)
	mov	r1, #0x10			// ibrd
	str	r1, [r0, #0x24]
	mov	r1, #0xc300
	orr	r1, #0x0001			// cr
	str	r1, [r0, #0x30]

    /* Disable interrupts for now */
    mrs r10, cpsr
    orr r10, r10, #0xc0     /* Mask IRQ and FIQ */
    msr cpsr, r10

    /* Set-up the secure vbar
     * During startup, before the MMU is enabled we'll set-up the vecs in low
     * mem.
     */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           /* SCTLR.V = 0 */
    mcr p15, 0, r10, c1, c0, 0
    ldr r10, =secure_vectors
    mcr p15, 0, r10, c12, c0, 0     /* VBAR = secure_vectors */

    /* The stack still needs to be allocated and mapped so we set up a
     * temporary stack for the time being.
     */
    ldr r10, =TEMP_STACK
	mov sp, r10

    /* Use the top of the stack to track our PA pool pointer */
    ldr r10, =PA_POOL_BASE
    str r10, [sp]

secure_mmu_init:
    /* Disable data and instruction caches */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x0004
    bic r10, r10, #0x1000
    mcr p15, 0, r10, c1, c0, 0

    /* Set domain 0 & 1 for client access */
    mov r10, #0x5
    mcr p15, 0, r10, c3, c0, 0

    /* Set TTBR0 to the initialized address plus enable shareable write-back
     * write-allocate.
     */
    ldr r10, =EL1_S_PGTBL_BASE
    orr r10, r10, #0xB
    mcr p15, 0, r10, c2, c0, 0

    /* Set-up the table base control to split between TTBR0/1 (N = 0) */
    mov r10, #0x0
    mcr p15, 0, r10, c2, c0, 2

secure_map_init:
    /* Direct map the init code */
    ldr r0, =_EL1_S_INIT_BASE
    movw r1, #0xDDA   //#PAGE_P1_RO
    ldr r2, =_EL1_S_INIT_SIZE
    bl map_pa_range

secure_map_flash:
    /* Direct map the EL1_S flash sections so we can copy from them once
     * the MMU has been enabled.
     */
    ldr r0, =_EL1_S_FLASH_TEXT
    movw r1, #0xDDA   //(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RO)
    ldr r2, =_EL1_S_TEXT_SIZE
    bl map_pa_range

    ldr r0, =_EL1_S_FLASH_DATA
    movw r1, #0xDDA   //(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RO)
    ldr r2, =_EL1_S_DATA_SIZE
    bl map_pa_range

secure_map_text:
    /* Map the EL1_S text address range.  Initially this needs to be RW so we can
     * copy in the text from flash.
     */
    ldr r0, =_EL1_S_TEXT_BASE
    movw r1, #0xDDA    //(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    ldr r2, =_EL1_S_TEXT_SIZE
    bl map_va_range

secure_map_data:
    /* Map the EL1_S data address range. */
    ldr r0, =_EL1_S_DATA_BASE
    movw r1, #0xDDA   //(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    ldr r2, =_EL1_S_DATA_SIZE
    bl map_va_range

secure_map_stack:
    /* Map the first page of the stack so we can get off the ground */
    ldr r0, =0x6FFEF000   // EL1_S_STACK_BASE-0x1000
    movw r1, #0xDDA   //(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    bl map_va

secure_map_pt:
    /* Direct map the page table pool */
    ldr r0, =EL1_S_PGTBL_BASE
    movw r1, #0xDDA   //(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    ldr r2, =EL1_S_PGTBL_SIZE
    bl map_pa_range

secure_map_uart:
    ldr r0, =UART0_BASE
    movw r1, #0xDDA   //(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    bl map_pa

secure_map_sysreg:
    ldr r0, =SYSREG_BASE;
    movw r1, #0xDDA   //(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    bl map_pa

save_last_pa:
    pop {r8}

secure_mmu_enable:
    /* Enable the mmu */
    mcr p15, 0, r10, c8, c7, 0
    mrc p15, 0, r10, c1, c0, 0
    orr r10, r10, #0x1
    mcr p15, 0, r10, c1, c0, 0
    isb
    dsb

secure_init_stack:
    /* Set-up the secure SVC stack */
	ldr sp, =EL1_S_STACK_BASE     //sec_svc_stacktop

    /* Migrate the next PA to the new stack we should not use it again until
     * after we transition to start.
     */
    push {r8}

    /* Have to set user (and system) stack from SYS mode so we can get back to
     * SVC.
     */
    cps #CPSR_MODE_SYS
	ldr sp, =EL0_S_STACK_BASE     //sec_usr_stacktop

    cps #CPSR_MODE_SVC

/* The EL1_S address space is set-up and the MMU is started so it is safe to copy
 * the text and data sections in.
 */
secure_copy_text:
    ldr r0, =_EL1_S_TEXT_BASE
    ldr r1, =_EL1_S_FLASH_TEXT
    ldr r2, =_EL1_S_TEXT_SIZE
    bl memcpy

secure_copy_data:
    ldr r0, =_EL1_S_DATA_BASE
    ldr r1, =_EL1_S_FLASH_DATA
    ldr r2, =_EL1_S_DATA_SIZE
    bl memcpy

secure_init_start:
    pop {r11}
    ldr r10, =mem_next_pa
    str r11, [r10]

    ldr r0, =_EL1_S_INIT_BASE
    ldr r1, =_EL1_S_INIT_SIZE
    ldr r2, =secure_svc_loop
    bx r2

/* We should never get here */
secure_init_end:
    b secure_init_end

secure_remap_vecs:
    /* Now that we have enabled the MMU, move the mapping of the secure vector
     * table to high mem.
     * No need to set up VBAR as we'll be using hivec */
    mrc p15, 0, r10, c1, c0, 0
    orr r10, r10, #0x2000           @ SCTLR.V = 1
    mcr p15, 0, r10, c1, c0, 0

    /* Check that are entry state makes sense before initializing the monitor
     * mode.
     */
    bl secure_check_init

#include "init_util.S"
