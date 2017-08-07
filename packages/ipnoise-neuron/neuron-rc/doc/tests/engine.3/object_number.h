#ifndef OBJECT_NUMBER_H
#define OBJECT_NUMBER_H

#include <stdint.h>

#include "object.h"

struct object_number
{
    Object  super;
    int32_t nan;    // Not-a-Number
};
typedef struct object_number ObjectNumber;

void            object_number__destructor_cb(Object *);
ObjectNumber *  object_number__constructor(
    Object      *a_prototype
);
ObjectNumber *  object_number__constructor_double(
    const double,
    Object      *a_prototype
);
ObjectNumber *  object_number__constructor_str(
    const char  *,
    Object      *a_prototype
);

void            object_number__serialize_cb(
    Object          *a_object,
    Object          *a_out,
    const char      *a_delimeter
);
void            object_number__to_string__cb(
    Object          *a_object,
    Object          *a_out
);

void            object_number__inc(
    ObjectNumber    *a_object,
    const double    a_val
);
void            object_number__dec(
    ObjectNumber    *a_object,
    const double    a_val
);

double         object_number__to_double(
    ObjectNumber    *a_object
);

int32_t         object_number__to_int32_t(
    ObjectNumber    *a_object
);

#endif

