#ifndef OBJECT_STATEMENT_H
#define OBJECT_STATEMENT_H

#include "object.h"

struct object_statement
{
    Object  super;
};
typedef struct object_statement ObjectStatement;

void            void___object_statement__destructor__Object_ptr(
    Object *a_object
);
ObjectStatement * ObjectStatement_ptr___object_statement__constructor();

#endif

