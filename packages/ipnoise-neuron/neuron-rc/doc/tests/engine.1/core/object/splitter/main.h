#ifndef MAIN_H
#define MAIN_H

#include "object.h"

struct object_splitter
{
    Object      super;
    uint8_t     *delim;
    uint64_t    delim_pos;
    uint64_t    delim_size;
    uint8_t     strict;
};
typedef struct object_splitter ObjectSplitter;

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
void    action_parse(
    ObjectSplitter  *a_object,
    const void      *a_data,
    const uint64_t  a_data_size
);

void action_delim_set(
    ObjectSplitter  *a_object,
    const void      *a_data,
    const uint64_t  a_data_size
);

#endif

