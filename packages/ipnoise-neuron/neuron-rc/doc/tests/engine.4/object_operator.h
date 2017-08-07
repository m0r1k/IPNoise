#ifndef OBJECT_OPERATOR_H
#define OBJECT_OPERATOR_H

#include "object.h"

struct object_operator
{
    Object  super;
    int32_t weight;
};
typedef struct object_operator ObjectOperator;

void            void___object_operator__destructor__Object_ptr(
    Object *a_object
);
ObjectOperator * ObjectOperator_ptr___object_operator__constructor();

#endif

