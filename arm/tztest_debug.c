#include "tztest.h"

#ifdef DEBUG
uint32_t tztest_div_by_2(uint32_t arg) {return arg/2;}
SECURE_USR_FUNC(tztest_div_by_2);

#define TZTEST_HANDSHAKE_FUNC(_name, _remote)       \
    uint32_t _name(uint32_t arg) {                  \
        uint32_t ret = 0;                           \
        _remote(tztest_div_by_2, arg, ret);         \
        assert(arg/2 == ret);                       \
        return ret;                                 \
    }

TZTEST_HANDSHAKE_FUNC(hs_with_ns_svc, DISPATCH_NONSECURE_SVC)
TZTEST_HANDSHAKE_FUNC(hs_with_s_svc, DISPATCH_SECURE_SVC)
TZTEST_HANDSHAKE_FUNC(hs_with_s_usr, DISPATCH_SECURE_USR)
TZTEST_HANDSHAKE_FUNC(hs_with_mon, DISPATCH_MONITOR)

#define CHECK_HANDSHAKE(_func)                                      \
            _func;                                            \
            if (val/2 != ret) {                                     \
                DEBUG_MSG("Handshake %s FAILED (0x%x != 0x%x)\n",   \
                          #_func, val/2, ret);                      \
                assert(val/2 == ret);                               \
            }                                                       \

void test_handshake()
{
    uint32_t ret = 0;
    uint32_t val = TZTEST_SVAL;

    /* NS_USR -> NS_SVC */
    CHECK_HANDSHAKE(ret = hs_with_ns_svc(val));

    /* NS_USR -> NS_SVC -> S_SVC */
    CHECK_HANDSHAKE(ret = hs_with_s_svc(val));

    /* NS_USR -> NS_SVC -> S_SVC -> S_USR */
    CHECK_HANDSHAKE(ret = hs_with_s_usr(val));

    /* NS_USR -> NS_SVC -> MON */
    CHECK_HANDSHAKE(ret = hs_with_mon(val));

    /* NS_SVC -> S_SVC */
    CHECK_HANDSHAKE(DISPATCH_NONSECURE_SVC(hs_with_s_svc, val, ret));

    /* NS_SVC -> S_USR */
    CHECK_HANDSHAKE(DISPATCH_NONSECURE_SVC(hs_with_s_usr, val, ret));

    /* NS_SVC -> MON */
    CHECK_HANDSHAKE(DISPATCH_NONSECURE_SVC(hs_with_mon, val, ret));

    /* S_SVC -> MON */
    CHECK_HANDSHAKE(DISPATCH_SECURE_SVC(hs_with_mon, val, ret));
}
#endif

