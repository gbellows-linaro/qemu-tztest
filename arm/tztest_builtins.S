    .globl __read_cpsr
__read_cpsr:
    mrs r0, cpsr
	mov pc, lr

    .globl __read_scr
__read_scr:
    mrc p15, 0, r0, c1, c1, 0
	mov pc, lr

    .globl __write_scr
__write_scr:
    mcr p15, 0, r0, c1, c1, 0
	mov pc, lr

    .globl __read_sder
__read_sder:
    mrc p15, 0, r0, c1, c1, 1
	mov pc, lr

    .globl __write_sder
__write_sder:
    mcr p15, 0, r0, c1, c1, 1
	mov pc, lr

    .globl __read_nsacr
__read_nsacr:
    mrc p15, 0, r0, c1, c1, 2
	mov pc, lr

    .globl __write_nsacr
__write_nsacr:
    mcr p15, 0, r0, c1, c1, 2
	mov pc, lr

    .globl __read_mvbar
__read_mvbar:
    mrc p15, 0, r0, c12, c0, 1
	mov pc, lr

    .globl __write_mvbar
__write_mvbar:
    mcr p15, 0, r0, c12, c0, 1
	mov pc, lr

    .globl __read_sctlr
__read_sctlr:
    mrc p15, 0, r0, c1, c0, 0
	mov pc, lr

/* Function for C code to make semihosting calls: */
	.globl __semi_call
__semi_call:
	svc     0x123456
	mov pc, lr

.globl halt
halt:
1:	wfi
	b	1b
