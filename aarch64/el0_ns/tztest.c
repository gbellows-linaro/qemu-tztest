
#include "libcflat.h"
#include "arm_builtins.h"
#include "svc.h"

int y = 0;

int foo()
{
    printf("Hello from foo\n");

    return 4;
}

int main()
{
    int x = foo();

    y = x;

    __svc(SVC_EXIT);
    return y;
}
