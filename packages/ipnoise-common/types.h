#ifndef TYPES_H
#define TYPES_H

#ifndef __aligned_u64
#  define __aligned_u64 __u64 __attribute__((aligned(8)))
#endif

#if defined(WIN32) || defined(WIN64)
#include <ipnoise-common/types_win.h>
#endif

#endif

