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

#ifndef _SYSCNTL_H
#define _SYSCNTL_H

#include <stdint.h>
#include "libcflat.h"

typedef struct {
     void *buf_pa;
     void *buf_va;
} smc_interop_t;

typedef struct {
    uint32_t ec;
    uint32_t iss;
    uint32_t far;
    bool log;
    uint32_t action;
    bool taken;
    uint32_t el;
    uint32_t state;
} sys_exception_t;

#define EXCP_ACTION_SKIP 1

#define SEC     0
#define NSEC    1

typedef struct {
    volatile int fail_count;
    volatile int test_count;
} test_control_t;

typedef struct {
    smc_interop_t smc_interop;
    sys_exception_t excp;
    test_control_t *test_cntl;
} sys_control_t;

extern sys_control_t *syscntl;

#endif
