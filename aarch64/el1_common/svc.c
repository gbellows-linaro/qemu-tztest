#include "svc.h"

const char *svc_op_name[] = {
    [SVC_OP_EXIT] = "SVC_OP_EXIT",
    [SVC_OP_ALLOC] = "SVC_OP_ALLOC",
    [SVC_OP_MAP] = "SVC_OP_MAP",
    [SVC_OP_YIELD] = "SVC_OP_YIELD",
    [SVC_OP_GET_SYSCNTL] = "SVC_OP_GET_SYSCNTL",
    [SVC_OP_GET_REG] = "SVC_OP_GET_REG",
    [SVC_OP_SET_REG] = "SVC_OP_SET_REG",
    [SVC_OP_TEST] = "SVC_OP_TEST"
};

