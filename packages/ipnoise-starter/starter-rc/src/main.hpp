#ifndef MAIN_HPP
#define MAIN_HPP

#ifdef _LIN32
#include "ipnoise-common/system.h"
#endif

#ifdef _ARM32
#include "ipnoise-common/system.h"
#endif

#ifndef my_system
#define my_system(argv, std_out, std_err)   \
    do {                                    \
    } while (0);
#endif

#endif

