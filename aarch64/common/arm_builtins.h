#ifndef _ARM_BUILTINS_H
#define _ARM_BUILTINS_H

#define __smc(_imm) asm volatile ("smc # %[imm] \n"::[imm] "I" (_imm))
#define __svc(_imm) asm volatile ("svc # %[imm] \n"::[imm] "I" (_imm))
#define __exception_return() asm volatile ("eret \n")
#define __set_exception_return(_elr) \
    asm volatile("msr elr_el1, %[elr]\n"::[elr] "r" (_elr))

#endif
