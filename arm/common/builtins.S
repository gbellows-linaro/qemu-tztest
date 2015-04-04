#define __ASSEMBLY__

.section .text
.globl __smc
__smc:
    smc #0
    bx lr

.globl __svc
__svc:
    svc #0
    bx lr

.macro READ_CPREG reg, opc1, crm, crn, opc2
.globl read_\reg
read_\reg:
    mrc p15, \opc1, r0, c\crm, c\crn, \opc2
    bx lr
.endm

.macro WRITE_CPREG reg, opc1, crm, crn, opc2
.globl write_\reg
write_\reg:
    mcr p15, \opc1, r0, c\crm, c\crn, \opc2
    bx lr
.endm

.macro RWCP reg, opc1, crm, crn, opc2
    READ_CPREG \reg, \opc1, \crm, \crn, \opc2
    WRITE_CPREG \reg, \opc1, \crm, \crn, \opc2
.endm

RWCP scr_el3, 0, 1, 1, 0
RWCP sder32_el3, 0, 1, 1, 1
RWCP cpacr_el1, 0, 1, 0, 2
RWCP sctlr_el1, 0, 1, 0, 0

.globl __set_exception_return
__set_exception_return:
    bx lr

#undef __ASSEMBLY__
