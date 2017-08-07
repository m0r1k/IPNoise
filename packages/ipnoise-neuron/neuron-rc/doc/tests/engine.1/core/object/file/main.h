#ifndef MAIN_H
#define MAIN_H

#include "object.h"

struct object_file
{
    Object  super;
    char    *fname;
};
typedef struct object_file ObjectFile;

Object * constructor(
    const void      *a_data,
    const uint64_t  a_data_size
);
void    destructor(Object *a_object);
void    action(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size
);
void action_to_stream_of_bytes(
    ObjectFile      *a_object,
    const void      *a_data,
    const uint64_t  a_data_size
);
void    action_load(
    ObjectFile      *a_object,
    const void      *a_data,
    const uint64_t  a_data_size
);

#endif

