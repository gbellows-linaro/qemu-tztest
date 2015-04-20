#ifndef _ARM_BUILTINS_H
#define _ARM_BUILTINS_H

#include <stdint.h>

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
extern uintptr_t read_cpsr();
extern void write_cpsr(uintptr_t);
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
#define READ_CPSR() read_cpsr()
#define WRITE_CPSR(_val) write_cpsr(_val)
#define READ_SCTLR() read_sctlr()
#define WRITE_SCTLR(_val) write_sctlr(_val)

#if REMOVE_OR_INTEGRATE
#define __cps(_r0) asm volatile ("cps %[r0]\n":: [r0] "X" (_r0))

#define __srsdb_svc(_mode) asm volatile ("srsdb sp!, #0x13\n")

#define __pop(_r0) \
    asm volatile ("pop {%[r0]}\n" : [r0] "=r" (_r0))

#define __mrc(_cp, _opc1, _r0, _crm, _crn, _opc2)                           \
    asm volatile (                                                          \
        "mrc p"#_cp", "#_opc1", %[r0], c"#_crm", c"#_crn", "#_opc2"\n"      \
        : [r0] "=r" (_r0)                                                   \
    )

#define __mcr(_cp, _opc1, _r0, _crm, _crn, _opc2)                           \
    asm volatile (                                                          \
        "mcr p"#_cp", "#_opc1", %[r0], c"#_crm", c"#_crn", "#_opc2"\n"      \
        : : [r0] "r" (_r0)                                                  \
    )

#define _READCP(_reg, _cp, _opc1, _crm, _crn, _opc2)    \
    static inline uint32_t _read_##_reg() {             \
        volatile int _r0 = -1;                          \
        __mrc(_cp, _opc1, _r0, _crm, _crn, _opc2);      \
        return _r0;                                     \
    }

#define _WRITECP(_reg, _cp, _opc1, _crm, _crn, _opc2)       \
    static inline uint32_t  _write_##_reg(uint32_t _r0) {   \
        __mcr(_cp, _opc1, _r0, _crm, _crn, _opc2);          \
        return 0;                                           \
    }

#define _RWCP(_reg, _cp, _opc1, _crm, _crn, _opc2)  \
    _READCP(_reg, _cp, _opc1, _crm, _crn, _opc2)    \
    _WRITECP(_reg, _cp, _opc1, _crm, _crn, _opc2)

_RWCP(scr, 15, 0, 1, 1, 0)           /* _read/write_scr */
_RWCP(sder, 15, 0, 1, 1, 1)          /* _read/write_sder */
_RWCP(nsacr, 15, 0, 1, 1, 2)         /* _read/write_nsacr */
_RWCP(mvbar, 15, 0, 12, 0, 1)        /* _read/write_mvbar */

/* Banked read/write CP register definitions */
_RWCP(csselr, 15, 2, 0, 0, 0)        /* _read/write_csselr */
_RWCP(sctlr, 15, 0, 1, 0, 0)         /* _read/write_sctlr */
_RWCP(actlr, 15, 0, 1, 0, 1)         /* _read/write_actlr */
_RWCP(ttbr0, 15, 0, 2, 0, 0)         /* _read/write_ttbr0 */
_RWCP(ttbr1, 15, 0, 2, 0, 1)         /* _read/write_ttbr1 */
_RWCP(ttbcr, 15, 0, 2, 0, 2)         /* _read/write_ttbcr */
_RWCP(dacr, 15, 0, 3, 0, 0)          /* _read/write_dacr */
_RWCP(dfsr, 15, 0, 5, 0, 0)          /* _read/write_dfsr */
_RWCP(ifsr, 15, 0, 5, 0, 1)          /* _read/write_ifsr */
_RWCP(dfar, 15, 0, 6, 0, 0)          /* _read/write_dfar */
_RWCP(ifar, 15, 0, 6, 0, 2)          /* _read/write_ifar */
_RWCP(par, 15, 0, 7, 4, 0)           /* _read/write_par */
_RWCP(prrr, 15, 0, 10, 2, 0)         /* _read/write_prrr */
_RWCP(nmrr, 15, 0, 10, 2, 1)         /* _read/write_nmrr */
_RWCP(vbar, 15, 0, 12, 0, 0)         /* _read/write_vbar */
_RWCP(fcseidr, 15, 0, 13, 0, 0)      /* _read/write_fcseidr */
_RWCP(contextidr, 15, 0, 13, 0, 1)   /* _read/write_contextidr*/
_RWCP(tpidrurw, 15, 0, 13, 0, 2)     /* _read/write_tpidrurw */
_RWCP(tpidruro, 15, 0, 13, 0, 3)     /* _read/write_tpidruro */
_RWCP(tpidrprw, 15, 0, 13, 0, 4)     /* _read/write_tpidrprw */

#endif
#endif
