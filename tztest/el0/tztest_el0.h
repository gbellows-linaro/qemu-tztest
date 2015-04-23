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

#ifndef _TZTEST_EL0_H
#define _TZTEST_EL0_H

extern uint32_t el0_check_smc(uint32_t);
extern uint32_t el0_check_register_access(uint32_t);
extern uint32_t el0_check_cpacr_trap(uint32_t);
extern uint32_t el0_check_fp_trap(uint32_t);
extern uint32_t el0_check_wfx_trap(uint32_t);

#endif

