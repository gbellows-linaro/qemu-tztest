#ifndef _TZTEST_H
#define _TZTEST_H

#include <stdint.h>

typedef uint32_t (*tztest_t)(uint32_t);
extern tztest_t tztest[];
extern void tztest_start();

#endif
