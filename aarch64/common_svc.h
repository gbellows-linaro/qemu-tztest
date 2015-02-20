#ifndef _COMMON_SVC_H
#define _COMMON_SVC_H

//#include "tztest_builtins.h"
#include "sm.h"
#include "libcflat.h"

typedef struct {
    uint32_t (*func)(uint32_t);
    uint32_t arg;
    uint32_t ret;
} tztest_dispatch_t;

typedef struct {
    union {
        tztest_dispatch_t dispatch;
        struct {
            uint32_t reg;
            uint32_t val;
        } reg_read;
        struct {
            uint32_t state;
        } secure_state;
    };
} tztest_svc_desc_t;

#define CLEAR_SVC_DESC(_desc) memset(&(_desc), sizeof(tztest_svc_desc_t), 0)

typedef struct {
    union {
        tztest_dispatch_t dispatch;
    };
} tztest_smc_desc_t;

#ifdef DEBUG
#define DEBUG_MSG(_str, ...) \
    printf("\n[DEBUG] %s: " _str, __FUNCTION__, ##__VA_ARGS__)
#define DEBUG_ARG
#else
#define DEBUG_MSG(_str, ...)
#define DEBUG_ARG __attribute__ ((unused))
#endif

#define MODE_STR(_mode)             \
    ((_mode == MON) ? "MON" :        \
     (_mode == SVC) ? "SVC" :        \
     (_mode == SYS) ? "SYS" :        \
     (_mode == USR) ? "USR" : "Unknown")


#define FAULT_STR(_s) \
    ((0x01 == (_s)) ? "Alignment fault" :                   \
     (0x02 == (_s)) ? "Debug event" :                       \
     (0x03 == (_s)) ? "Access flag fault" :                 \
     (0x04 == (_s)) ? "Instruction cache maintenance" :     \
     (0x05 == (_s)) ? "Translation fault" :                 \
     (0x06 == (_s)) ? "Access flag fault" :                 \
     (0x07 == (_s)) ? "Translation fault" :                 \
     (0x08 == (_s)) ? "Synchronous external abort" :        \
     (0x09 == (_s)) ? "Domain fault" :                      \
     (0x0b == (_s)) ? "Domain fault" :                      \
     (0x0c == (_s)) ? "External abort on table walk" :      \
     (0x0d == (_s)) ? "Permisison fault" :                  \
     (0x0e == (_s)) ? "Synchronous external abort" :        \
     (0x0f == (_s)) ? "Permisison fault" :                  \
     (0x10 == (_s)) ? "TLB conflict abort" :                \
     (0x1c == (_s)) ? "External parity err on table walk" : \
     (0x1e == (_s)) ? "External parity err on table walk" : \
     "Unknown")

#endif
