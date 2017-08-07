#ifndef OBJECT_STRING_H
#define OBJECT_STRING_H

#include <stdint.h>

#include "object.h"

struct object_string
{
    Object  super;
};
typedef struct object_string ObjectString;

#include "object_array.h"

void            object_string__destructor_cb(Object *);
ObjectString *  object_string__constructor(
    Object      *a_prototype
);
ObjectString *  object_string__constructor_str(
    const char  *,
    Object      *a_prototype
);
ObjectString *  object_string__constructor_raw(
    const char  *a_data,
    uint64_t    a_data_size,
    Object      *a_prototype
);


void        object_string__realloc_and_concat(
    char        **a_out,
    const char  *a_format,
    ...
);
void        object_string__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter
);
void        object_string__to_string_cb(
    Object      *a_object,
    Object      *a_out
);

// generic
int32_t         object_string__cmp_str(
    ObjectString    *a_object,
    const char      *a_str
);
int32_t         object_string__cmp_obj(
    ObjectString    *a_left,
    ObjectString    *a_right
);
int32_t         object_string__to_int32_t(
    ObjectString    *a_object
);
double          object_string__to_double(
    ObjectString    *a_object
);
const char *    object_string__mem_c_str(
    ObjectString *a_object
);
int32_t         object_string__charAt(
    ObjectString    *a_object,
    const double    a_pos,
    uint8_t         *a_out
);
ObjectString *  object_string__charAt_str(
    ObjectString    *a_object,
    const uint32_t  a_pos
);
int32_t         object_string__load_from_file(
    ObjectString    *a_object,
    ObjectString    *a_path
);
void            object_string__remove_last_char(
    ObjectString    *a_string
);

// api
Object *        object_string__api_charAt(
    Object          *a_object,
    ObjectArray     *a_args
);

#endif

