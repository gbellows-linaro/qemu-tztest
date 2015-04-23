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

#include "tztest_internal.h"
#include "tztest_el0.h"

tztest_t test_func[] = {
    [TZTEST_SMC] = el0_check_smc,
    [TZTEST_REG_ACCESS] = el0_check_register_access,
#ifdef AARCH64
    [TZTEST_CPACR_TRAP] = el0_check_cpacr_trap,
    [TZTEST_WFX_TRAP] = el0_check_wfx_trap,
    [TZTEST_FP_TRAP] = el0_check_fp_trap,
#endif
};

