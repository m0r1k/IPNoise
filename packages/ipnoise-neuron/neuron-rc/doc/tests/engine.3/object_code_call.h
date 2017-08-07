#ifndef OBJECT_CODE_CALL_H
#define OBJECT_CODE_CALL_H

#include <stdint.h>

#include "object.h"
#include "object_string.h"
#include "object_array.h"
#include "object_code_define.h"

struct object_code_call
{
    Object              super;
    ObjectCodeDefine    *code_define;
    ObjectArray         *args;
    Object              *context;
};
typedef struct object_code_call ObjectCodeCall;

void                object_code_call__destructor_cb(
    Object *
);
ObjectCodeCall *    object_code_call__constructor(
    Object  *a_prototype
);

void    object_code_call__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter
);

Object *    object_code_call__call_with_args(
    ObjectCodeDefine    *a_code_define,
    Object              *a_context,
    Object              *a_position,
    ObjectArray         *a_args
);

#endif

