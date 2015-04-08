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
    asm volatile("mov r0, r0\n")

#define __get_exception_address(_addr) \
    asm volatile("mov r0, r0\n")

extern uintptr_t read_scr();
extern void write_scr(uintptr_t);
extern uintptr_t read_sder();
extern void write_sder(uintptr_t);
extern uintptr_t read_cptr();
extern void write_cptr(uintptr_t);
extern uintptr_t read_cpacr();
extern void write_cpacr(uintptr_t);
extern uintptr_t read_cpacr();
extern void write_cpacr(uintptr_t);
extern uintptr_t read_sctlr();
extern void write_sctlr(uintptr_t);
extern uintptr_t read_mvbar();
extern void write_mvbar(uintptr_t);
extern uintptr_t read_nsacr();
extern void write_nsacr(uintptr_t);
extern uintptr_t read_dfar();
extern void write_dfsr(uintptr_t);
extern uintptr_t read_dfsr();
extern void write_dfar(uintptr_t);
extern uintptr_t read_ifar();
extern void write_ifar(uintptr_t);
extern uintptr_t read_ifsr();
extern void write_ifsr(uintptr_t);
extern void __set_exception_return(uintptr_t);
extern void __exception_return(uintptr_t, uint32_t);

#define READ_SCR() read_scr()
#define WRITE_SCR(_val) write_scr(_val)
#define READ_SDER() read_sder()
#define WRITE_SDER(_val) write_sder(_val)
#define READ_CPACR() read_cpacr()
#define WRITE_CPACR(_val) write_cpacr(_val)
#define READ_MVBAR() read_mvbar()
#define WRITE_MVBAR(_val) write_mvbar(_val)
#define READ_NSACR() read_nsacr()
#define WRITE_NSACR(_val) write_nsacr(_val)

#endif
