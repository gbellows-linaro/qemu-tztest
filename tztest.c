#include "semihosting.h"
#include <string.h>
#include <stdio.h>

enum {
    USR = 0x10,
    FIQ,
    IRQ,
    SVC,
    MON = 0x16,
    ABT,
    HYP = 0x1a,
    UND,
    SYS = 0x1f
} mode;

/* Make the below globals volatile as  found that the compiler uses the
 * register value ratherh than the memory value making it look like the writes
 * actually happened.
 */
volatile int smc_count = 0; 
volatile int svc_count = 0;
volatile int undef_count = 0;
volatile int unused_count = 0;
volatile int exception = 0;

void write_message(char const *msg)
{
    semi_write(1, msg, strlen(msg));
}

int smc_handler() __attribute__ ((interrupt ("SWI")));
int svc_handler() __attribute__ ((interrupt ("SWI")));
int undef_handler() __attribute__ ((interrupt ("UNDEF")));

int smc_handler() {
    int temp = smc_count;
    smc_count++;
    exception = MON;
    return temp;
}

int svc_handler() {
    svc_count++;
    exception = SVC;
}

int undef_handler() {
    undef_count++;
    exception = UND;
}

int unused_handler() {
    unused_count++;
    exception = -1;
}

/* SMC intrinsic
 * Need to enable the security extension on v7 or the assembler will complain
 */
#define _smc(imm)                   \
    asm (                           \
        ".arch_extension sec\n"     \
        "mov r0, #"#imm"\n"         \
        "smc 0"                     \
    )

int c_start(int x, int y)
{
    int res = 0;
    int mode = get_mode();

    write_message("Starting QEMU TZ test...\n");

    set_mode(SYS);
    res = __smc(2);
    if (exception == MON) {
        write_message("**** Took expected SMC exception\n");
        exception = 0;
    }

    if (smc_count > 0 ) write_message("**** Took SMC exception \n");
    if (svc_count > 0 ) write_message("**** Took SVC exception \n");

    set_mode(mode);

    return 0;
}
