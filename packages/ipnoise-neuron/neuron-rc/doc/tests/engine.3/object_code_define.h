#ifndef OBJECT_CODE_DEFINE_H
#define OBJECT_CODE_DEFINE_H

#include <stdint.h>

#include "object.h"
#include "object_string.h"
#include "object_array.h"

struct object_code_define
{
    Object          super;
    Object *        (*func)(
        Object          *a_parser,   // parser
        Object          *a_position, // file position or NULL
        ObjectArray     *a_args
    );
    ObjectArray     *args_names;
    ObjectArray     *instructions;
};
typedef struct object_code_define ObjectCodeDefine;

void               object_code_define__destructor_cb(
    Object  *
);
ObjectCodeDefine * object_code_define__constructor(
    Object  *a_prototype
);

void        object_code_define__serialize(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter,
    ObjectArray *a_args         // function call args or NULL
);

void        object_code_define__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter
);

const char * object_code_define__mem_c_str(
    ObjectCodeDefine *a_object
);

#endif

