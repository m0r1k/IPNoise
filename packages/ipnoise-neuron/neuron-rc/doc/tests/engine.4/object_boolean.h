#ifndef OBJECT_BOOLEAN_H
#define OBJECT_BOOLEAN_H

#include "object.h"

struct object_boolean
{
    Object  super;
};
typedef struct object_boolean ObjectBoolean;

void            void___object_boolean__destructor__Object_ptr(
    Object *a_object
);
ObjectBoolean * ObjectBoolean_ptr___object_boolean__constructor();

int32_t int32_t___object_boolean__set_val___ObjectBoolean_ptr__const_double(
    ObjectBoolean   *a_object,
    const double    a_val
);

#endif

