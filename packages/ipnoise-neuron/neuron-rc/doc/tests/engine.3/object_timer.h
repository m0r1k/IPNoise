#ifndef OBJECT_TIMER_H
#define OBJECT_TIMER_H

#include <stdint.h>

#include "object.h"
#include "object_string.h"
#include "object_array.h"
#include "object_code_define.h"

struct object_timer
{
    Object              super;
    double              id;
    Object              *context;
    ObjectCodeDefine    *code_define;
    ObjectArray         *args;
    struct event        *ev;
};
typedef struct object_timer ObjectTimer;

void            object_timer__destructor_cb(Object *);
ObjectTimer *   object_timer__constructor(
    Object *a_prototype
);

void    object_timer__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter
);

#endif

