/*
 * Copyright (c) 2015 Linaro Limited
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2 or later, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

RWCP scr, 0, 1, 1, 0
RWCP sder, 0, 1, 1, 1
RWCP cpacr, 0, 1, 0, 2
RWCP sctlr, 0, 1, 0, 0
RWCP mvbar, 0, 12, 0, 1
RWCP nsacr, 0, 1, 1, 2
RWCP dfsr, 0, 5, 0, 0
RWCP dfar, 0, 6, 0, 0
RWCP ifsr, 0, 5, 0, 1
RWCP ifar, 0, 6, 0, 2

.globl __set_exception_return
__set_exception_return:
    bx lr

.globl __exception_return
__exception_return:
    push {r0, r1}
    rfefd sp!

.globl read_cpsr
read_cpsr:
    mrs r0, cpsr
    bx lr

.globl write_cpsr
write_cpsr:
    msr cpsr_cxsf, r0
    bx lr

#undef __ASSEMBLY__
