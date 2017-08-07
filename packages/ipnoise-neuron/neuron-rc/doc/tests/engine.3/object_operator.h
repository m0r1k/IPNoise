#ifndef OBJECT_OPERATOR_H
#define OBJECT_OPERATOR_H

#include <stdint.h>

#include "object.h"

struct object_operator
{
    Object      super;
    Object *    (*func)(
        Object          *a_context,
        ListItem        *a_code,
        ListItem        *a_operator,
        Flags           *a_flags
    );
    int32_t     weight;
};
typedef struct object_operator ObjectOperator;

void                object_operator__destructor_cb(Object *);
ObjectOperator *    object_operator__constructor(
    Object  *a_prototype
);

void    object_operator__serialize_cb(
    Object          *a_object,
    Object          *a_out,
    const char      *a_delimeter
);
void    object_operator__to_string__cb(
    Object          *a_object,
    Object          *a_out
);

const char * object_operator__mem_c_str(
    ObjectOperator *a_object
);

#endif

