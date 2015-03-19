#ifndef _DEBUG_H
#define _DEBUG_H

#ifdef DEBUG
#define DEBUG_MSG(_str, ...) \
    printf("\n[DEBUG] %s (%s): " _str, __FUNCTION__, SEC_STATE_STR, ##__VA_ARGS__)
#define DEBUG_ARG
#else
#define DEBUG_MSG(_str, ...)
#define DEBUG_ARG __attribute__ ((unused))
#endif

#endif
