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

#ifndef _MEM_UTIL_H
#define _MEM_UTIL_H

extern uintptr_t mem_allocate_pa();
extern void mem_map_pa(uintptr_t, uintptr_t, uintptr_t, uintptr_t);
extern void mem_map_va(uintptr_t, uintptr_t, uintptr_t);
extern int mem_unmap_va(uintptr_t addr);
extern void *mem_heap_allocate(size_t len);
extern void *mem_lookup_pa(void *va);

#endif
