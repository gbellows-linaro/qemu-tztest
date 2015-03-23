#ifndef _ARM_BUILTINS_H
#define _ARM_BUILTINS_H

#include "stdint.h"

#define __exception_return(_x0) asm volatile ("eret\n")
#define __set_exception_return(_elr) \
    asm volatile("mrs x7, currentel\n"  \
                 "cmp x7, #0x4\n"   \
                 "b.eq setelrel1\n"   \
                 "cmp x7, #0x8\n"   \
                 "b.eq setelrel2\n"   \
                 "setelrel3: msr elr_el3, %[elr]\n"   \
                 "b setelrdone\n"   \
                 "setelrel2: msr elr_el2, %[elr]\n"   \
                 "b setelrdone\n"   \
                 "setelrel1: msr elr_el1, %[elr]\n"   \
                 "setelrdone:\n":: [elr] "r" (_elr) : "r7")

#define __get_exception_return(_addr) \
    asm volatile("mrs x0, currentel\n"  \
                 "cmp x0, #0x4\n"   \
                 "b.eq elrel1\n"   \
                 "cmp x0, #0x8\n"   \
                 "b.eq elrel2\n"   \
                 "elrel3: mrs %0, elr_el3\n"   \
                 "b elrdone\n"   \
                 "elrel2: mrs %0, elr_el2\n"   \
                 "b elrdone\n"   \
                 "elrel1: mrs %0, elr_el1\n"   \
                 "elrdone:\n" : "=r" (_addr))

#define __get_exception_address(_addr) \
    asm volatile("mrs x0, currentel\n"  \
                 "cmp x0, #0x4\n"   \
                 "b.eq farel1\n"   \
                 "cmp x0, #0x8\n"   \
                 "b.eq farel2\n"   \
                 "farel3: mrs %0, far_el3\n"   \
                 "b fardone\n"   \
                 "farel2: mrs %0, far_el2\n"   \
                 "b fardone\n"   \
                 "farel1: mrs %0, far_el1\n"   \
                 "fardone:\n" : "=r" (_addr))

extern uint64_t read_currentel();
extern void write_currentel(uint64_t);
extern uint64_t read_scr_el3();
extern void write_scr_el3(uint64_t);
extern uint64_t read_sder32_el3();
extern void write_sder32_el3(uint64_t);
extern uint64_t read_cptr_el3();
extern void write_cptr_el3(uint64_t);
extern uint64_t read_cpacr_el1();
extern void write_cpacr_el1(uint64_t);
#endif
