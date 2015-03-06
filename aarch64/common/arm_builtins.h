#ifndef _ARM_BUILTINS_H
#define _ARM_BUILTINS_H

#define __smc(_imm) asm volatile ("smc # %[imm] \n"::[imm] "I" (_imm))
#define __svc(_imm, _desc) \
    asm volatile ("mov x0, %1\n" \
                  "svc #%0\n" \
                  ::[imm] "I" (_imm), "r" (_desc))
#define __exception_return() asm volatile ("eret \n")
#define __set_exception_return(_elr) \
    asm volatile("msr elr_el1, %[elr]\n"::[elr] "r" (_elr))
#define __get_exception_return(_addr) \
    asm volatile("mrs %0, elr_el1\n": "=r" (_addr))
#define __get_exception_address(_addr) \
    asm volatile("mrs %0, far_el1\n": "=r" (_addr))

#endif
