#ifndef _SVC_H
#define _SVC_H

#define SVC_RETURN_FROM_SECURE_USR 0
#define SVC_DISPATCH_MONITOR 1
#define SVC_DISPATCH_SECURE_USR 2
#define SVC_DISPATCH_SECURE_SVC 3
#define SVC_DISPATCH_NONSECURE_SVC 4
#define SVC_GET_SECURE_STATE 5
#define SVC_EXIT 6

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

#endif
