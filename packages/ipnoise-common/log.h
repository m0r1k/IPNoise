/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2009 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#ifndef LOG_H
#define LOG_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// TODO delete this #include <execinfo.h>

#include <string>
#include <vector>

#include "log_common.h"

using namespace std;

#define SIZE 4096

#define PLOG_OBJ(obj, type, args...) do {                               \
    char                                ___buffer[SIZE];                \
    string                              ___msg;                         \
    string                              ___path;                        \
    vector<string>                      ___paths;                       \
    vector<string>::reverse_iterator    ___paths_i;                     \
    DomElement                          *___cur_element = NULL;         \
                                                                        \
    assert(obj);                                                        \
                                                                        \
    ___cur_element = (DomElement *)obj;                                 \
    while (___cur_element){                                             \
        ___paths.push_back(___cur_element->getFuseFullName());          \
        ___cur_element = ___cur_element->getParentNode();               \
    }                                                                   \
    for (___paths_i = ___paths.rbegin();                                \
        ___paths_i != ___paths.rend();                                  \
        ___paths_i++)                                                   \
    {                                                                   \
        ___path += "/";                                                 \
        ___path += *___paths_i;                                         \
    }                                                                   \
    snprintf(___buffer, sizeof(___buffer),                              \
        "%s[%s] ",                                                      \
        type,                                                           \
        ___path.c_str()                                                 \
    );                                                                  \
    ___msg += ___buffer;                                                \
    snprintf(___buffer, sizeof(___buffer), args);                       \
    ___msg += ___buffer;                                                \
    ((DomElement *)obj)->pDebug(___msg);                                \
    if (!strcmp("[ ERROR ]", type)){                                    \
        PERROR("%s\n", ___msg.c_str());                                 \
    }                                                                   \
} while(0);

#define PINFO_OBJ(obj, args...) do {                                    \
    if ((void *)obj == NULL){                                           \
        PINFO(args);                                                    \
        break;                                                          \
    } else {                                                            \
        PLOG_OBJ(obj, "[ INFO  ]", args);                               \
    }                                                                   \
} while(0);

#define PWARN_OBJ(obj, args...) do {                                    \
    if ((void *)obj == NULL){                                           \
        PWARN(args);                                                    \
        break;                                                          \
    } else {                                                            \
        PLOG_OBJ(obj, "[ WARN  ]", args);                               \
    }                                                                   \
} while(0);

#define PERROR_OBJ(obj, args...) do {                                   \
    if ((void *)obj == NULL){                                           \
        PERROR(args);                                                   \
        break;                                                          \
    }                                                                   \
    PLOG_OBJ(obj, "[ ERROR ]", args);                                   \
} while(0);

#define PFATAL_OBJ(obj, args...) do {                                   \
    PERROR_OBJ(obj, args);                                              \
    assert(0);                                                          \
} while(0);

#define PDEBUG_OBJ(obj, level, args...) do {                            \
    char __buffer[SIZE];                                                \
    PDEBUG(level, args);                                                \
    if ((void *)obj == NULL){                                           \
        break;                                                          \
    }                                                                   \
    if (debug_level >= level){                                          \
        snprintf(__buffer, sizeof(__buffer),                            \
            "[ DEBUG (%2.2d) ]",                                        \
            level                                                       \
        );                                                              \
        PLOG_OBJ(obj, __buffer, args);                                  \
    }                                                                   \
} while(0);

#endif

