#define __ASSEMBLY__

.section .text
.globl __smc
__smc:
    smc #0
    ret

.globl __svc
__svc:
    svc #0
    ret

.macro READ_REG reg
.globl read_\reg
read_\reg:
    mrs x0, \reg
    ret
.endm

.macro WRITE_REG reg
.globl write_\reg
write_\reg:
    msr \reg, x0
    ret
.endm

READ_REG currentel
WRITE_REG currentel

READ_REG scr_el3
WRITE_REG scr_el3

READ_REG sder32_el3
WRITE_REG sder32_el3

READ_REG cptr_el3
WRITE_REG cptr_el3

READ_REG cpacr_el1
WRITE_REG cpacr_el1

#undef __ASSEMBLY__
