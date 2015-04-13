#ifndef _ARM_BUILTINS_H
#define _ARM_BUILTINS_H

#include "stdint.h"

#define CPTR_TFP    (1 << 10)
#define CPTR_TCPAC  (1 << 31)
#define SCTLR_nTWI  (1 << 16)
#define SCTLR_nTWE  (1 << 18)
#define SCR_WFI     (1 << 12)
#define SCR_WFE     (1 << 13)

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
extern uint64_t read_cpacr_el1();
extern void write_cpacr_el1(uint64_t);
extern uint64_t read_sctlr_el1();
extern void write_sctlr_el1(uint64_t);
extern void __set_exception_return(uint64_t);
extern void __exception_return(uintptr_t, uint32_t);

#define READ_SCR() read_scr_el3()
#define WRITE_SCR(_val) write_scr_el3(_val)
#define READ_SDER() read_sder32_el3()
#define WRITE_SDER(_val) write_sder32_el3(_val)
#define READ_CPTR_EL3() read_cptr_el3()
#define WRITE_CPTR_EL3(_val) write_cptr_el3(_val)
#define READ_CPACR() read_cpacr_el1()
#define WRITE_CPACR(_val) write_cpacr_el1(_val)
#define READ_CURRENTEL() read_currentel()
#define WRITE_CURRENTEL(_val) read_currentel(_val)

#endif
