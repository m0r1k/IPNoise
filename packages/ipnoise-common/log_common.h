/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#ifndef LOG_COMMON_H
#define LOG_COMMON_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ipnoise-common/colors.h>

// Logging
extern int g_debug_level;

// FYI: __PRETTY_FUNCTION__ is as __FUNCTION__ but pretty

#if defined(WIN32) || defined(WIN64)    // if win

#define PDEBUG_POS(file, line, function, level, args...)    \
    do {                                                    \
        if (g_debug_level >= level){                        \
            fprintf(stderr,                                 \
                "[ DEBUG: %2.2d ] [%s:%d (%s)] ",           \
                level,                                      \
                file, line, function                        \
            );                                              \
            fprintf(stderr, args);                          \
        }                                                   \
    } while (0);

#define PDEBUG(level, args...)                              \
    do {                                                    \
        PDEBUG_POS(                                         \
            __FILE__,                                       \
            __LINE__,                                       \
            __FUNCTION__,                                   \
            level,                                          \
            args                                            \
        );                                                  \
    } while (0);

#define PSUCCESS(args...)                                   \
    do {                                                    \
        fprintf(stderr, "[ SUCCESS ] ");                    \
        fprintf(stdout, args);                              \
    } while (0);

#define PINFO(args...)                                      \
    do {                                                    \
        fprintf(stderr, "[ INFO ] ");                       \
        fprintf(stdout, args);                              \
    } while (0);

#define PALERT(args...)                                     \
    do {                                                    \
        fprintf(stderr, "[ ALERT ] ");                      \
        fprintf(stdout, args);                              \
    } while (0);

#define PWARN_POS(file, line, function, args...)            \
    do {                                                    \
        fprintf(stderr, "[ WARNING ] [%s:%d (%s)] ",        \
            file, line, function                            \
        );                                                  \
        fprintf(stderr, args);                              \
    } while (0);

#define PWARN(args...)                                      \
    do {                                                    \
        PWARN_POS(                                          \
            __FILE__,                                       \
            __LINE__,                                       \
            __FUNCTION__,                                   \
            args                                            \
        );                                                  \
    } while (0);

#define PRINT_TIMING(                                       \
    file, line, function,                                   \
    time_max, time, args...)                                \
    do {                                                    \
        if (time < time_max){                               \
            break;                                          \
        }                                                   \
        fprintf(stderr,                                     \
            "[ TIMING %g (max %g ms) ] [%s:%d (%s)] ",      \
            time * 1000,                                    \
            time_max * 1000,                                \
            file,                                           \
            line,                                           \
            function                                        \
        );                                                  \
        fprintf(stderr, args);                              \
    } while (0);

#define PERROR_POS(file, line, function, args...)           \
    do {                                                    \
        fprintf(stderr, "--------- ERROR --------- {\n");   \
        fprintf(stderr, "%s:%d (%s)\n",                     \
            file, line, function                            \
        );                                                  \
        fprintf(stderr, args);                              \
        perror(stderr, "last error no: ");                  \
        fprintf(stderr, "--------- ERROR --------- }\n");   \
    } while (0);

#define PERROR(args...)                                     \
    do {                                                    \
        PERROR_POS(                                         \
            __FILE__,                                       \
            __LINE__,                                       \
            __FUNCTION__,                                   \
            args                                            \
        );                                                  \
    } while (0)

#define PFATAL(args...)                                     \
    do {                                                    \
        PFATAL_POS(                                         \
            __FILE__,                                       \
            __LINE__,                                       \
            __FUNCTION__,                                   \
            args                                            \
        );                                                  \
    } while (0);

#define PFATAL_POS(file, line, function, args...)           \
    do {                                                    \
        fprintf(stderr, "--------- FATAL --------- {\n");   \
        fprintf(stderr, "%s:%d (%s)\n",                     \
            file, line, function                            \
        );                                                  \
        fprintf(stderr, args);                              \
        perror(stderr, "last error no: ");                  \
        fprintf(stderr, "--------- FATAL --------- }\n");   \
        assert(0);                                          \
    } while (0);

#else

// if not win

#define PDEBUG_POS(file, line, function,                    \
    level, args...)                                         \
    do {                                                    \
        if (g_debug_level >= level){                        \
            fprintf(stderr,                                 \
                "[ DEBUG: %2.2d ] [%s:%d (%s)] ",           \
                level,                                      \
                file, line, function                        \
            );                                              \
            fprintf(stderr, args);                          \
        }                                                   \
    } while (0);

#define PDEBUG(level, args...)                              \
    do {                                                    \
        PDEBUG_POS(                                         \
            __FILE__,                                       \
            __LINE__,                                       \
            __FUNCTION__,                                   \
            level,                                          \
            args                                            \
        );                                                  \
    } while (0);

#define PSUCCESS(args...) do {                              \
    fprintf(stderr, COLOR_GREEN);                           \
    fprintf(stderr, "[ SUCCESS ] ");                        \
    fprintf(stdout, args);                                  \
    fprintf(stderr, COLOR_GRAY);                            \
} while (0);

#define PINFO(args...) do {                                 \
    fprintf(stderr, "[ INFO ] ");                           \
    fprintf(stdout, args);                                  \
    fprintf(stderr, COLOR_GRAY);                            \
} while (0);

#define PALERT(args...) do {                                \
    fprintf(stderr, COLOR_BLUE);                            \
    fprintf(stderr, "[ ALERT ] ");                          \
    fprintf(stdout, args);                                  \
    fprintf(stderr, COLOR_GRAY);                            \
} while (0);

#define PWARN_POS(file, line, function, args...)            \
    do {                                                    \
        fprintf(stderr, COLOR_YELLOW);                      \
        fprintf(stderr, "[ WARNING ] [%s:%d (%s)] ",        \
            file, line, function                            \
        );                                                  \
        fprintf(stderr, args);                              \
        fprintf(stderr, COLOR_GRAY);                        \
    } while (0);

#define PWARN(args...)                                      \
    do {                                                    \
        PWARN_POS(                                          \
            __FILE__,                                       \
            __LINE__,                                       \
            __FUNCTION__,                                   \
            args                                            \
        );                                                  \
    } while (0);

#define PRINT_TIMING(                                       \
    file, line, function,                                   \
    time_max, time, args...)                                \
    do {                                                    \
        if (time < time_max){                               \
            break;                                          \
        }                                                   \
        fprintf(stderr, COLOR_RED);                         \
        fprintf(stderr,                                     \
            "[ TIMING %g (max %g ms) ] [%s:%d (%s)] ",      \
            time * 1000,                                    \
            time_max * 1000,                                \
            file,                                           \
            line,                                           \
            function                                        \
        );                                                  \
        fprintf(stderr, args);                              \
        fprintf(stderr, COLOR_GRAY);                        \
    } while (0);

#define PERROR_POS(file, line, function, args...)           \
    do {                                                    \
        fprintf(stderr, "--------- ERROR --------- {\n");   \
        fprintf(stderr, "%s:%d (%s)\n",                     \
            file, line, function                            \
        );                                                  \
        fprintf(stderr, COLOR_RED);                         \
        fprintf(stderr, args);                              \
        fprintf(stderr, COLOR_GRAY);                        \
        perror("last error no: ");                          \
        fprintf(stderr, "--------- ERROR --------- }\n");   \
    } while (0);

#define PERROR(args...)                                     \
    do {                                                    \
        PERROR_POS(                                         \
            __FILE__,                                       \
            __LINE__,                                       \
            __FUNCTION__,                                   \
            args                                            \
        );                                                  \
    } while (0);

#define PFATAL(args...)                                     \
    do {                                                    \
        PFATAL_POS(                                         \
            __FILE__,                                       \
            __LINE__,                                       \
            __FUNCTION__,                                   \
            args                                            \
        );                                                  \
    } while (0);

#define PFATAL_POS(file, line, function, args...)           \
    do {                                                    \
        fprintf(stderr, "--------- FATAL --------- {\n");   \
        fprintf(stderr, "%s:%d (%s)\n",                     \
            file, line, function                            \
        );                                                  \
        fprintf(stderr, COLOR_RED);                         \
        fprintf(stderr, args);                              \
        fprintf(stderr, COLOR_GRAY);                        \
        perror("last error no: ");                          \
        fprintf(stderr, "--------- FATAL --------- }\n");   \
        assert(0);                                          \
    } while (0);

#endif // end if not win

#endif

