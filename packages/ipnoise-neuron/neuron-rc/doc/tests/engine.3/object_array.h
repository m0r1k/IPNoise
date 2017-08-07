#ifndef OBJECT_ARRAY_H
#define OBJECT_ARRAY_H

#include <stdint.h>

#include "object.h"

struct object_array
{
    Object  super;
};
typedef struct object_array ObjectArray;

void                object_array__destructor_cb(Object *);
ObjectArray    *    object_array__constructor(
    Object  *a_prototype
);

// set val
void        object_array__set_by_index(
    ObjectArray     *a_object,
    const double    a_index,
    Object          *a_val
);

// get val
int32_t     object_array__get_by_index(
    ObjectArray     *a_object,
    const double    a_index,
    Object          **a_out
);
Object *    object_array__get_last(ObjectArray *);

// push
void        object_array__push_obj(
    ObjectArray     *a_object,
    Object          *a_val
);
void        object_array__push_str(
    ObjectArray     *a_object,
    const char      *a_str
);
void        object_array__push_double(
    ObjectArray     *a_object,
    const double    a_val
);

// shift
int32_t     object_array__shift(
    ObjectArray     *a_object,
    Object          **a_out
);

// pop
Object *    object_array__pop_obj(ObjectArray *);

// serialize
void        object_array__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter
);

// lists
int32_t     object_array__copy_to_list(
    ObjectArray     *a_object,
    ListItem        *a_list
);

#endif

