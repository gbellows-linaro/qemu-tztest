
#include "common_defs.h"

.global secure_dispatch_usr
secure_dispatch_usr:
    /* Push the LR onto the stack so we can return immediately from the svc
     * handler on return.
     */
    push {lr}

    /* Reset the user stack for the dispatch, so we have a clean stack */
    cps #CPSR_MODE_SYS
	ldr sp, =sec_usr_stacktop
    push {r0, r1}
    cps #CPSR_MODE_SVC

    /* Return to USR mode to execute the dispatch function in r0
     */
    ldr r0, =dispatch_usr_wrapper
    mov r1, #CPSR_MODE_USR
    push {r0, r1}
    rfefd sp!

/* This is the user-side dispatch wrapper that accepts the function pointer and
 * function arg in r0 & r1 respectively.  The wrapper is simply an anstraction
 * layer for translating the input parameters into the user function call.
 */
dispatch_usr_wrapper:
    pop {r0, r1}
    mov r2, r0
    mov r0, r1
    mov pc, r2

.global secure_shutdown
secure_shutdown:
    ldr r0, =SYSREG_BASE
    add r0, r0, #SYSREG_CFGCTRL
    ldr r1, =SYS_SHUTDOWN
    str r1, [r0]
    b secure_shutdown
