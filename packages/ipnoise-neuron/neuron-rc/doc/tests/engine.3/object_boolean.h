#ifndef OBJECT_BOOLEAN_H
#define OBJECT_BOOLEAN_H

#include <stdint.h>

#include "object.h"

struct object_boolean
{
    Object  super;
    uint8_t val;
};
typedef struct object_boolean ObjectBoolean;

void            object_boolean__destructor_cb(Object *);
ObjectBoolean * object_boolean__constructor(
    Object      *a_prototype
);
ObjectBoolean * object_boolean__constructor_double(
    const double,
    Object      *a_prototype
);
ObjectBoolean * object_boolean__constructor_str(
    const char  *,
    Object      *a_prototype
);

void    object_boolean__serialize_cb(
    Object          *a_object,
    Object          *a_out,
    const char      *a_delimeter
);
void    object_boolean__to_string__cb(
    Object          *a_object,
    Object          *a_out
);

double  object_boolean__to_double(
    ObjectBoolean    *a_object
);

uint8_t object_boolean__to_uint8_t(
    ObjectBoolean    *a_object
);

int32_t object_boolean__to_int32_t(
    ObjectBoolean    *a_object
);

int32_t object_boolean__get_bool_result(
    Object  *a_val,
    uint8_t *a_out
);

#endif

