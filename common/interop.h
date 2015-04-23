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

#ifndef _INTEROP_H
#define _INTEROP_H
#include <stdint.h>
#include <stddef.h>

#define OP_MAP_NSEC_EL0 0
#define OP_MAP_SEC_EL0  1
#define OP_MAP_NSEC_EL1 2
#define OP_MAP_SEC_EL1  4
#define OP_MAP_EL3      8
#define OP_MAP_ALL \
            (OP_MAP_SEC_EL0 | OP_MAP_NSEC_EL1 | OP_MAP_SEC_EL1 | OP_MAP_EL3)

typedef struct {
    uint32_t type;
    size_t len;
    void *addr;
} op_alloc_mem_t;

typedef struct {
    uint32_t type;
    size_t len;
    void *va;
    void *pa;
} op_map_mem_t;

typedef struct {
    uint32_t fid;
    uintptr_t arg;
    uintptr_t state;
    uintptr_t el;
    uintptr_t ret;
} op_dispatch_t;

typedef enum {
    CURRENTEL = 1,
    CPTR_EL3,
    CPACR,
    SCR,
    SCTLR,
    CPSR,
} op_reg_key_t;

typedef struct {
    uintptr_t key;
    uintptr_t data;
    uint32_t el;
} op_data_t;

typedef struct {
    uintptr_t orig;
    uintptr_t val;
    uint32_t count;
    uint32_t fail;
} op_test_t;
#endif
