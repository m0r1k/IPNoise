#ifndef OBJECT_CONNECTION_H
#define OBJECT_CONNECTION_H

#include <stdint.h>

#include "object.h"
#include "object_string.h"
#include "object_array.h"
#include "libevent.h"

struct object_connection
{
    Object              super;
    Object              *context;
    struct bufferevent  *be;
    int32_t             fd;
};
typedef struct object_connection ObjectConnection;

void                object_connection__destructor_cb(Object *);
ObjectConnection *  object_connection__constructor(
    Object *a_prototype
);

// api
Object *    object_connection__api_close(
    Object          *a_connection,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args
);

#endif

