#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <ipnoise-common/log_common.h>

void get_function_name(
    const char      *a_func_name,
    char            *a_out,
    const uint32_t  a_size
);

#define TWARN(a_format, ...)                                \
    do {                                                    \
        char func_name[512] = { 0x00 };                     \
        get_function_name(                                  \
            __FUNCTION__,                                   \
            func_name,                                      \
            sizeof(func_name)                               \
        );                                                  \
        PWARN_POS(                                          \
            __FILE__,                                       \
            __LINE__,                                       \
            func_name,                                      \
            a_format,                                       \
            ## __VA_ARGS__                                  \
        );                                                  \
    } while (0);

#define TERROR(a_format, ...)                               \
    do {                                                    \
        char func_name[512] = { 0x00 };                     \
        get_function_name(                                  \
            __FUNCTION__,                                   \
            func_name,                                      \
            sizeof(func_name)                               \
        );                                                  \
        PERROR_POS(                                         \
            __FILE__,                                       \
            __LINE__,                                       \
            func_name,                                      \
            a_format,                                       \
            ## __VA_ARGS__                                  \
        );                                                  \
    } while (0);

#define TFATAL(a_format, ...)                               \
    do {                                                    \
        char func_name[512] = { 0x00 };                     \
        get_function_name(                                  \
            __FUNCTION__,                                   \
            func_name,                                      \
            sizeof(func_name)                               \
        );                                                  \
        PFATAL_POS(                                         \
            __FILE__,                                       \
            __LINE__,                                       \
            func_name,                                      \
            a_format,                                       \
            ## __VA_ARGS__                                  \
        );                                                  \
    } while (0);


#endif

