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

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#define EC_UNKNOWN 0x00
#define EC_WFI_WFE 0x01
#define EC_SIMD 0x07
#define EC_ILLEGAL_STATE 0x0E
#define EC_SVC32 0x11
#define EC_SMC32 0x13
#define EC_SVC64 0x15
#define EC_SMC64 0x17
#define EC_SYSINSN 0x18
#define EC_IABORT_LOWER 0x20
#define EC_IABORT 0x21
#define EC_DABORT_LOWER 0x24
#define EC_DABORT 0x25

#endif
