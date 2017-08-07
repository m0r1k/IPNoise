#ifndef TEST_H
#define TEST_H

#include <stdint.h>

int32_t test_speed(
    int32_t     (*a_func)(
        int32_t a_argc,
        char    **a_argv
    ),
    uint32_t    a_count,
    int32_t     a_argc,
    char        **a_argv
);

#endif

