#ifndef OBJECT_IPNOISE_H
#define OBJECT_IPNOISE_H

#include <stdint.h>

#include "object.h"

struct object_ipnoise
{
    Object  super;
};
typedef struct object_ipnoise ObjectIPNoise;

#include "object_array.h"

void            object_ipnoise__destructor_cb(Object *);
ObjectIPNoise * object_ipnoise__constructor(
    Object *a_prototype
);

// api
Object *    object_ipnoise__api_getMemInfo(
    Object          *a_object,
    Object          *a_position, // file position or NULL
    ObjectArray     *a_args
);

#endif

