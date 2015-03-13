#ifndef _SYSCNTL_H
#define _SYSCNTL_H

typedef struct {
     void *buf_pa;
     void *buf_va;
} smc_interop_t;

typedef struct {
    uint32_t ec;
    uint32_t iss;
    uint32_t far;
    bool ignore;
} sys_exception_t;

typedef struct {
    smc_interop_t smc_interop;
    sys_exception_t el3_excp;
    sys_exception_t el1_excp[2];
} sys_control_t;

#endif
