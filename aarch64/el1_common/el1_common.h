#ifndef __EL1_COMMON_H
#define __EL1_COMMON_H

#include "libcflat.h"
#include "memory.h"
#include "svc.h"
#include "smc.h"
#include "string.h"
#include "el1.h"
#include "armv8_exception.h"
#include "armv8_vmsa.h"
#include "arm_builtins.h"
#include "debug.h"
#include "syscntl.h"

extern void el1_init_el0();
extern void *el1_load_el0(char *base, char *start_va);

#endif
