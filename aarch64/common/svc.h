#ifndef _SVC_H
#define _SVC_H

#define SVC_RETURN_FROM_SECURE_USR 0
#define SVC_DISPATCH_MONITOR 1
#define SVC_DISPATCH_SECURE_USR 2
#define SVC_DISPATCH_SECURE_SVC 3
#define SVC_DISPATCH_NONSECURE_SVC 4
#define SVC_GET_SECURE_STATE 5
#define SVC_EXIT 6
#define SVC_ALLOC 7
#define SVC_MAP 8
#define SVC_YIELD 9
#define SVC_GET_SYSCNTL 10

#ifndef __ASSEMBLY__
#include "interop.h"

extern void __svc(uint32_t, void *);

typedef union {
    op_alloc_mem_t alloc;
    op_map_mem_t map;
} svc_op_desc_t;

#endif

#endif
