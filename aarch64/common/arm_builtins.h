#ifndef _ARM_BUILTINS_H
#define _ARM_BUILTINS_H

#define __smc(_imm, _desc) \
    asm volatile ("mov x0, %1\n" \
                  "smc #%0\n" \
                  ::[imm] "I" (_imm), "r" (_desc))
#define __svc(_imm, _desc) \
    asm volatile ("mov x0, %1\n" \
                  "svc #%0\n" \
                  ::[imm] "I" (_imm), "r" (_desc))
#define __exception_return(_x0) asm volatile ("eret\n")
#define __set_exception_return(_elr) \
    asm volatile("msr elr_el1, %[elr]\n"::[elr] "r" (_elr))
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

#endif
