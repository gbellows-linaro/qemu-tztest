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

#ifndef _MEMORY_H
#define _MEMORY_H

#include "platform.h"

#define VA_SIZE 48
#define TnSZ (64-VA_SIZE)
#define STACK_SIZE 0x40000

#define EL3_FLASH_BASE FLASH_BASE
#define EL3_RAM_BASE RAM_BASE
#define EL3_RAM_SIZE (512*1024)
#define EL3_BASE_VA 0xF000000000
#define EL3_PGTBL_SIZE 0x40000
#define EL3_PGTBL_BASE EL3_RAM_BASE + EL3_RAM_SIZE - EL3_PGTBL_SIZE
#define EL3_STACK_SIZE STACK_SIZE
#define EL3_STACK_BASE 0xFF00000000
#define EL3_PA_POOL_BASE (EL3_PTE_POOL_BASE+(0x40000-0x4000))
#define EL3_PTE_POOL_BASE EL3_RAM_BASE
#define EL3_INIT_STACK EL3_RAM_BASE+EL3_RAM_SIZE-0x1000
#define EL3_VA_HEAP_BASE EL3_BASE_VA + 0x1000000

#define EL1_S_FLASH_BASE (FLASH_BASE+0x10000)
#define EL1_S_RAM_BASE (RAM_BASE+0x1000000)
#define EL1_S_RAM_SIZE (512*1024)
#define EL1_S_BASE_VA 0xC000000000
#define EL1_S_PGTBL_SIZE 0x40000
#define EL1_S_PGTBL_BASE EL1_S_RAM_BASE+EL1_S_RAM_SIZE-EL1_S_PGTBL_SIZE
#define EL1_S_STACK_SIZE STACK_SIZE
#define EL1_S_STACK_BASE 0xCF00000000
#define EL1_S_PA_POOL_BASE (EL1_S_PGTBL_BASE+EL1_S_PGTBL_SIZE)
#define EL1_S_PTE_POOL_BASE (EL1_S_PGTBL_BASE + 0x4000)
#define EL1_S_INIT_STACK EL1_S_RAM_BASE+EL1_S_RAM_SIZE-0x1000
#define EL1_S_VA_HEAP_BASE EL1_S_BASE_VA + 0x1000000

#define EL1_NS_FLASH_BASE (FLASH_BASE+0x20000)
#define EL1_NS_RAM_BASE (RAM_BASE+0x2000000)
#define EL1_NS_RAM_SIZE (512*1024)
#define EL1_NS_BASE_VA 0x80000000
#define EL1_NS_PGTBL_SIZE 0x40000
#define EL1_NS_PGTBL_BASE EL1_NS_RAM_BASE+EL1_NS_RAM_SIZE-EL1_NS_PGTBL_SIZE
#define EL1_NS_STACK_SIZE STACK_SIZE
#define EL1_NS_STACK_BASE 0xFFFFF000
#define EL1_NS_PA_POOL_BASE (EL1_NS_PGTBL_BASE+EL1_NS_PGTBL_SIZE)
#define EL1_NS_PTE_POOL_BASE (EL1_NS_PGTBL_BASE + 0x4000)
#define EL1_NS_INIT_STACK EL1_NS_RAM_BASE+EL1_NS_RAM_SIZE-0x1000
#define EL1_NS_VA_HEAP_BASE EL1_NS_BASE_VA + 0x1000000

#define EL0_S_FLASH_BASE (FLASH_BASE+0x50000)
#define EL0_S_RAM_BASE (RAM_BASE+0x4000000)
#define EL0_S_RAM_SIZE (512*1024)
#define EL0_S_BASE_VA 0x000C0000
#define EL0_S_STACK_SIZE STACK_SIZE
#define EL0_S_STACK_BASE 0x80000000-0x1000

#define EL0_NS_FLASH_BASE (FLASH_BASE+0x30000)
#define EL0_NS_RAM_BASE (RAM_BASE+0x4000000)
#define EL0_NS_RAM_SIZE (512*1024)
#define EL0_NS_BASE_VA 0x00080000
#define EL0_NS_STACK_SIZE STACK_SIZE
#define EL0_NS_STACK_BASE 0x70000000-0x1000

#endif
