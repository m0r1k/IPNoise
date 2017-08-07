#ifndef OBJECT_FILE_H
#define OBJECT_FILE_H

#include <stdint.h>

#include "object.h"

struct object_file
{
    Object  super;
};
typedef struct object_file ObjectFile;

void          object_file__destructor_cb(Object *);
ObjectFile *  object_file__constructor(
    Object *a_prototype
);

#endif

