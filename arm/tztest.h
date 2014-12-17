#ifndef _TZTEST_H
#define _TZTEST_H

#include "common_defs.h"
#include "common_svc.h"

extern volatile int _tztest_fail_count;
extern volatile int _tztest_test_count;

#define CALL(_f)  __svc(0, _f)
#define RETURN(_r)  __svc(0,(_r))

#define DISPATCH(_op, _func, _arg, _ret)    \
    do {                                    \
        tztest_svc_desc_t _desc;            \
        _desc.dispatch.func = (_func);      \
        _desc.dispatch.arg = (_arg);        \
        __svc((_op), &_desc);               \
        (_ret) = _desc.dispatch.ret;        \
    } while(0)

#define SECURE_USR_FUNC(_func)  \
    uint32_t _func##_wrapper(uint32_t arg) { RETURN(_func(arg)); return 0; }

#define DISPATCH_SECURE_USR(_func, _arg, _ret)            \
        DISPATCH(SVC_DISPATCH_SECURE_USR, (_func##_wrapper), (_arg), (_ret))
#define DISPATCH_SECURE_SVC(_func, _arg, _ret)            \
        DISPATCH(SVC_DISPATCH_SECURE_SVC, (_func), (_arg), (_ret))
#define DISPATCH_MONITOR(_func, _arg, _ret)            \
        DISPATCH(SVC_DISPATCH_MONITOR, (_func), (_arg), (_ret))
#define DISPATCH_NONSECURE_SVC(_func, _arg, _ret)            \
        DISPATCH(SVC_DISPATCH_NONSECURE_SVC, (_func), (_arg), (_ret))

#define INC_TEST_COUNT()    (*tztest_test_count += 1)
#define INC_FAIL_COUNT()    (*tztest_fail_count += 1)

#define TEST_CONDITION(_cond)                           \
    do {                                                \
        if (!(_cond)) {                                 \
            printf("FAILED\n");                         \
            INC_FAIL_COUNT();                           \
        } else {                                        \
            printf("PASSED\n");                         \
        }                                               \
        INC_TEST_COUNT();                               \
    } while(0)

#define TEST_FUNCTION(_fn, _cond)                       \
    do {                                                \
        _fn;                                            \
        TEST_CONDITION(_cond);                          \
    } while(0)

#define TEST_EXCEPTION(_fn, _excp)                      \
    do {                                                \
        TEST_FUNCTION(_fn, *tztest_exception == (_excp));   \
        *tztest_exception = 0;                          \
    } while (0)

#define TZTEST_SVAL 0xaaaaaaaa
#define TZTEST_NSVAL ~TZTEST_SVAL
#define TZTEST_GET_REG_SECURE_BANK(_reg, _val)      \
    do {                                            \
        _write_scr(scr & ~SCR_NS);                  \
        (_val) = _read_##_reg();                    \
    } while(0)

#define TZTEST_GET_REG_NONSECURE_BANK(_reg, _val)   \
    do {                                            \
        _write_scr(scr | SCR_NS);                   \
        (_val) = _read_##_reg();                    \
    } while(0)

#define TZTEST_SET_REG_SECURE_BANK(_reg, _val)      \
    do {                                            \
        _write_scr(scr & ~SCR_NS);                  \
        _write_##_reg(_val);                        \
    } while(0)

#define TZTEST_SET_REG_NONSECURE_BANK(_reg, _val)   \
    do {                                            \
        _write_scr(scr | SCR_NS);                   \
        _write_##_reg(_val);                        \
    } while(0)

#define TZTEST_GET_REG_BANKS(_reg, _sval, _nsval)   \
    do {                                            \
        TZTEST_GET_REG_SECURE_BANK(_reg, _sval);    \
        TZTEST_GET_REG_NONSECURE_BANK(_reg, _nsval);\
    } while(0)

#define TZTEST_SET_REG_BANKS(_reg, _sval, _nsval)   \
    do {                                            \
        TZTEST_SET_REG_SECURE_BANK(_reg, _sval);    \
        TZTEST_SET_REG_NONSECURE_BANK(_reg, _nsval);\
    } while(0)

#define VERIFY_REGISTER_CUSTOM(_reg, _mask, _sval, _nsval)              \
    do {                                                                \
        uint32_t sval = 0, nsval = 0;                                   \
        uint32_t _reg[2] = {0,0};                                       \
        printf("\tChecking %s banks... ", #_reg);                       \
        TZTEST_GET_REG_BANKS(_reg, _reg[!SCR_NS], _reg[SCR_NS]);        \
        TZTEST_SET_REG_BANKS(_reg, (_sval), (_nsval));                  \
        TZTEST_GET_REG_SECURE_BANK(_reg, sval);                         \
        TZTEST_GET_REG_NONSECURE_BANK(_reg, nsval);                     \
        TEST_CONDITION(((sval & (_mask)) != (nsval & (_mask))) &&       \
                       (((_sval) & (_mask)) == (sval & (_mask))) &&     \
                       (((_nsval) & (_mask)) == (nsval & (_mask))));    \
        TZTEST_SET_REG_BANKS(_reg, _reg[!SCR_NS], _reg[SCR_NS]);        \
    } while(0)

#define VERIFY_REGISTER(_reg)    \
    VERIFY_REGISTER_CUSTOM(_reg, 0xFFFFFFFF, TZTEST_SVAL, TZTEST_NSVAL)

extern volatile int *tztest_exception;
extern volatile int *tztest_exception_addr;
extern volatile int *tztest_exception_status;
extern volatile int *tztest_fail_count;
extern volatile int *tztest_test_count;
extern void validate_state(uint32_t, uint32_t);
extern uint32_t _shared_memory_heap_base;
extern uint32_t *nsec_l1_page_table;
extern uint32_t *nsec_l2_page_table;

#endif
