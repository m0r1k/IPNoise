#ifndef DIR_H
#define DIR_H

#include <stdint.h>

#define DEFAULT_MKDIR_MODE  0755

#ifndef min
#define min(a, b) (a < b) ? a : b
#endif

int32_t mkdirr(
    const char      *a_path,
    const uint32_t  a_mode
);

#endif

