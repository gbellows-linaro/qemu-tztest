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

#ifndef _ARMV8_VMSA_H
#define _ARMV8_VMSA_H

#define PAGE_SIZE 0x1000
#define ARMV7_PAGE_XN_SHIFT 0
#define ARMV7_PAGE_B_SHIFT 2
#define ARMV7_PAGE_C_SHIFT 3
#define ARMV7_PAGE_AP0_SHIFT 4
#define ARMV7_PAGE_AP1_SHIFT 5
#define ARMV7_PAGE_TEX_SHIFT 6
#define ARMV7_PAGE_AP2_SHIFT 9
#define ARMV7_PAGE_S_SHIFT 10
#define ARMV7_PAGE_NG_SHIFT 11
#define ARMV7_PAGE_ADDR_SHIFT 12

#define ARMV7_PTE_TYPE_LARGE_PAGE 0x1
#define ARMV7_PTE_TYPE_SMALL_PAGE 0x2
#define ARMV7_PTE_TYPE_TABLE_PAGE 0x1

/* Using the simplified access permissions we can define the following */
#define ARMV7_PAGE_RW       (0<<ARMV7_PAGE_AP2_SHIFT)
#define ARMV7_PAGE_RO       (1<<ARMV7_PAGE_AP2_SHIFT)
#define ARMV7_PAGE_PRIV     (0<<ARMV7_PAGE_AP1_SHIFT)
#define ARMV7_PAGE_USER     (1<<ARMV7_PAGE_AP1_SHIFT)
#define ARMV7_PAGE_ACCESS   (1<<ARMV7_PAGE_AP0_SHIFT)

#define PTE_ACCESS          ARMV7_PAGE_ACCESS
#define PTE_PRIV_RW         (ARMV7_PAGE_ACCESS|ARMV7_PAGE_PRIV|ARMV7_PAGE_RW)
#define PTE_PRIV_RO         (ARMV7_PAGE_ACCESS|ARMV7_PAGE_PRIV|ARMV7_PAGE_RO)
#define PTE_USER_RW         (ARMV7_PAGE_ACCESS|ARMV7_PAGE_USER|ARMV7_PAGE_RW)
#define PTE_USER_RO         (ARMV7_PAGE_ACCESS|ARMV7_PAGE_USER|ARMV7_PAGE_RO)
#define PTE_PAGE            ARMV7_PTE_TYPE_SMALL_PAGE
#define PTE_TABLE           ARMV7_PTE_TYPE_TABLE_PAGE
#endif
