#include "tztest.h"
#include "tztest_builtins.h"
#include "libcflat.h"

#define CALL(_f)  __svc(0, _f)

extern int nsec_exception;
extern int nsec_fail_count;
extern int nsec_test_count;

void P0_nonsecure_tests() 
{
    /* Set things to non-secure P1 and attempt accesses */
    printf("\nValidating non-secure P0 inaccessibility:\n");

    printf("\tUnprivileged P0 smc call ... ");
    TEST_EXCP_COND(asm volatile (".arch_extension sec\n" "smc #0\n"), 
                   nsec, CPSR_MODE_UND, ==);

    printf("\tUnprivileged P0 smc SCR read ... ");
    TEST_EXCP_COND(_read_scr(), nsec, CPSR_MODE_UND, ==);

    printf("\tUnprivileged P0 smc SCR write ... ");
    TEST_EXCP_COND(_write_scr(0), nsec, CPSR_MODE_UND, ==);

    printf("\tUnprivileged P0 smc SDER read ... ");
    TEST_EXCP_COND(_read_sder(), nsec, CPSR_MODE_UND, ==);

    printf("\tUnprivileged P0 smc SDER write ... ");
    TEST_EXCP_COND(_write_sder(0), nsec, CPSR_MODE_UND, ==);

    printf("\tUnprivileged P0 smc MVBAR read ... ");
    TEST_EXCP_COND(_read_mvbar(), nsec, CPSR_MODE_UND, ==);

    printf("\tUnprivileged P0 smc MVBAR write ... ");
    TEST_EXCP_COND(_write_mvbar(0), nsec, CPSR_MODE_UND, ==);

    printf("\tUnprivileged P0 smc NSACR write ... ");
    TEST_EXCP_COND(_write_nsacr(0), nsec, CPSR_MODE_UND, ==);
}

void tztest_nonsecure_usr_main()
{
    DEBUG_MSG("Entered\n");

    __svc(1,0);

    DEBUG_MSG("Next\n");

    //CALL(tztest_secure_usr_test1);

    DEBUG_MSG("Exiting\n");
    
    P0_nonsecure_tests();
}
