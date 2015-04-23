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

#ifndef _TZTEST_H
#define _TZTEST_H

#include <stdint.h>

typedef uint32_t (*tztest_t)(uint32_t);
typedef struct {
    uint32_t fid;
    uint32_t el;
    uint32_t state;
    uint32_t arg;
} tztest_case_t;
extern tztest_case_t tztest[];
extern tztest_t test_func[];
extern void tztest_start();
extern void run_test(uint32_t, uint32_t, uint32_t, uint32_t);

#endif
