#ifndef __EL0_COMMON_H
#define __EL0_COMMON_H

#include "libcflat.h"
#include "svc.h"
#include "syscntl.h"
#include "arch.h"
#include "arm_builtins.h"
#include "el0.h"
#include "debug.h"
#include "tztest.h"

extern sys_control_t *syscntl;

extern void *alloc_mem(int type, size_t len);
extern void map_va(void *va, size_t len, int type);

#endif
