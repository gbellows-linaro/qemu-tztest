#include "tztest.h"
#include "tztest_builtins.h"
#include "libcflat.h"

#define RETURN(_r)  __svc(0,(_r))

void tztest_secure_usr_test1()
{
    DEBUG_MSG("Entered\n");
    __svc(1,0);
    DEBUG_MSG("Exiting\n");
    RETURN(0);
}

