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

#ifndef __EL1_COMMON_H
#define __EL1_COMMON_H

#include "libcflat.h"
#include "memory.h"
#include "svc.h"
#include "smc.h"
#include "string.h"
#include "el1.h"
#include "builtins.h"
#include "state.h"
#include "debug.h"
#include "syscntl.h"

extern void el1_init_el0();
extern bool el1_load_el0(uintptr_t base, uintptr_t *entry);

#endif
