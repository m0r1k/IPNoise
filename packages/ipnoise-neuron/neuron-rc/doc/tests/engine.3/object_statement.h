#ifndef OBJECT_STATEMENT_H
#define OBJECT_STATEMENT_H

#include <stdint.h>

#include "object.h"

struct object_statement
{
    Object      super;
    Object *    (*func)(
        Object          *a_context,
        ListItem        *a_code,
        ListItem        *a_statement,
        Flags           *a_flags
    );
};
typedef struct object_statement ObjectStatement;

void                object_statement__destructor_cb(Object *);
ObjectStatement *   object_statement__constructor(
    Object *a_prototype
);

void    object_statement__serialize_cb(
    Object          *a_object,
    Object          *a_out,
    const char      *a_delimeter
);
void    object_statement__to_string__cb(
    Object          *a_object,
    Object          *a_out
);

const char * object_statement__mem_c_str(
    ObjectStatement *a_object
);

#endif

