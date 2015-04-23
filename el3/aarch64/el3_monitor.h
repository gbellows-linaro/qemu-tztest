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

#ifndef _EL3_MONITOR_H
#define _EL3_MONITOR_H

typedef struct state_buf {
    uintptr_t elr_el3;
    uintptr_t spsr_el3;
    uintptr_t elr_el1;
    uintptr_t spsr_el1;
    uintptr_t esr_el1;
    uintptr_t sp_el1;
    uintptr_t spsel;
    uintptr_t sp_el0;
    uintptr_t spsr_abt;
    uintptr_t spsr_und;
    uintptr_t spsr_irq;
    uintptr_t spsr_fiq;
    uintptr_t sctlr_el1;
    uintptr_t actlr_el1;
    uintptr_t cpacr_el1;
    uintptr_t csselr_el1;
    uintptr_t ttbr0_el1;
    uintptr_t ttbr1_el1;
    uintptr_t tcr_el1;
    uintptr_t vbar_el1;
    uintptr_t mair_el1;
    uintptr_t amair_el1;
    uintptr_t tpidr_el0;
    uintptr_t tpidr_el1;
    uintptr_t tpidrro_el0;
    uintptr_t contextidr_el1;
    uintptr_t par_el1;
    uintptr_t far_el1;
    uintptr_t afsr0_el1;
    uintptr_t afsr1_el1;
    uintptr_t reg[27];        /* X4:X30 */
} state_buf;

extern void monitor_switch(int op, smc_op_desc_t *desc);
extern void monitor_restore_state(state_buf *);
extern void monitor_save_state(state_buf *);

#endif
