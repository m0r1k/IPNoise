#ifndef LOG_HPP
#define LOG_HPP

#include <stdint.h>
#include <assert.h>

extern int32_t debug_level;

#define COLOR_RED       "\033[1;31m"
#define COLOR_YELLOW    "\033[1;33m"
#define COLOR_GREEN     "\033[0;32m"
#define COLOR_GRAY      "\033[0;39m"

#define PDEBUG(level, args...) do {                                     \
    if (debug_level >= level){                                          \
        fprintf(stderr, "[DEBUG: %2.2d] [%s:%d (%s)] ",                 \
            level,                                                      \
            __FILE__,                                                   \
            __LINE__,                                                   \
            __FUNCTION__                                                \
        );                                                              \
        fprintf(stderr, args);                                          \
    }                                                                   \
} while (0);

#define PSUCCESS(args...) do {                                          \
    fprintf(stderr, COLOR_GREEN);                                       \
    fprintf(stdout, args);                                              \
    fprintf(stderr, COLOR_GRAY);                                        \
} while (0);

#define PINFO(args...) do {                                             \
    fprintf(stdout, args);                                              \
    fprintf(stderr, COLOR_GRAY);                                        \
} while (0);

#define PWARN(args...) do {                                             \
    fprintf(stderr, COLOR_YELLOW);                                      \
    fprintf(stderr, args);                                              \
    fprintf(stderr, COLOR_GRAY);                                        \
} while (0);

#define PERROR(args...) do {                                            \
    fprintf(stderr, "--------- ERROR --------- {\n");                   \
    fprintf(stderr, "%s:%d (%s)\n", __FILE__, __LINE__, __FUNCTION__);  \
    fprintf(stderr, COLOR_RED);                                         \
    fprintf(stderr, args);                                              \
    fprintf(stderr, COLOR_GRAY);                                        \
    perror("last error no: ");                                          \
    fprintf(stderr, "--------- ERROR --------- }\n");                   \
} while (0);

#define PFATAL(args...) do {                                            \
    PERROR(args);                                                       \
    assert(0);                                                          \
} while (0);

#endif

