#ifndef OBJECT_ACTION_H
#define OBJECT_ACTION_H

#include <stdint.h>

#include "object.h"

struct object_action
{
    Object  super;
};
typedef struct object_action ObjectAction;

void            object_action__destructor_cb(Object *);
ObjectAction *  object_action__constructor(
    const char  *,
    Object      *a_prototype
);

void            object_action__serialize_cb(
    Object          *a_object,
    Object          *a_out,
    const char      *a_delimeter
);
void            object_action__to_string__cb(
    Object          *a_object,
    Object          *a_out
);

#endif

