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

#ifndef _DEBUG_H
#define _DEBUG_H

#include "state.h"

#ifdef DEBUG
#define DEBUG_MSG(_str, ...) \
    printf("\n[DEBUG] %s (%s): "\
           _str, __FUNCTION__, sec_state_str[secure_state], ##__VA_ARGS__)
#define DEBUG_ARG
#else
#define DEBUG_MSG(_str, ...)
#define DEBUG_ARG __attribute__ ((unused))
#endif

#endif
