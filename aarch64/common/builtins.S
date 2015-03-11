#define __ASSEMBLY__

.globl __smc
__smc:
    smc #0
    ret

.globl __svc
__svc:
    svc #0
    ret

#undef __ASSEMBLY__
