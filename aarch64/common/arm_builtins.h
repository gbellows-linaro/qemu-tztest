#ifndef _ARM_BUILTINS_H
#define _ARM_BUILTINS_H

#define __smc(_imm) asm volatile ("smc # %[imm] \n"::[imm] "I" (_imm))

#endif
