#ifndef _TZTEST_BUILTINS_H
#define _TZTEST_BUILTINS_H

/* SMC intrinsic
 * Need to enable the security extension on v7 or the assembler will complain
 */
#define __smc(_r0, _r1)                     \
    asm volatile (                          \
        ".arch_extension sec\n"             \
        "mov r0, %[r0]\n"                   \
        "mov r1, %[r1]\n"                   \
        "smc 0\n"                           \
        : : [r0] "r" (_r0) ,[r1] "r" (_r1)  \
        )

#define __cps(_r0) asm volatile ("cps %[r0]\n":: [r0] "X" (_r0))

#define __mrs(_r0, _reg) asm volatile ("mrs %[r0], "#_reg"\n" : [r0] "=r" (_r0))
#define __msr(_r0, _reg) asm volatile ("mrs "#_reg", %[r0]\n" :: [r0] "r" (_r0))

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

#define _read_cp(_reg, _cp, _opc1, _crm, _crn, _opc2)   \
    static inline int _read_##_reg() {                  \
        int _r0 = -1;                                   \
        __mrc(_cp, _opc1, _r0, _crm, _crn, _opc2);      \
        return _r0;                                     \
    }

#define _write_cp(_reg, _cp, _opc1, _crm, _crn, _opc2)  \
    static inline void  _write_##_reg(int _r0) {        \
        __mcr(_cp, _opc1, _r0, _crm, _crn, _opc2);      \
    }

_read_cp(scr, 15, 0, 1, 1, 0)       /* _read_scr */
_read_cp(sder, 15, 0, 1, 1, 1)      /* _read_sder */
_read_cp(nsacr, 15, 0, 1, 1, 2)     /* _read_nsacr */
_read_cp(mvbar, 15, 0, 12, 0, 1)    /* _read_mvbar */
_write_cp(scr, 15, 0, 1, 1, 0)       /* _write_scr */
_write_cp(sder, 15, 0, 1, 1, 1)      /* _write_sder */
_write_cp(nsacr, 15, 0, 1, 1, 2)     /* _write_nsacr */
_write_cp(mvbar, 15, 0, 12, 0, 1)    /* _write_mvbar */

static inline int _read_cpsr() {
    int _r0 = -1;
    __mrs(_r0, cpsr);
    return _r0;
}

#endif