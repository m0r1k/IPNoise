#include "log.h"
#include "object.h"

#include "object_statement.h"

#define object_statement__destructor // for ctags

void void___object_statement__destructor___Object_ptr(
    Object *a_object)
{
    void___object__destructor___Object_ptr(a_object);
}

#define object_statement__constructor // for ctags

ObjectStatement * ObjectStatement_ptr___object_statement__constructor()
{
    ObjectStatement *ret = NULL;

    ret = (ObjectStatement *)
        Object_ptr___object__constructor___const_uint32t(
            sizeof(*ret)
        );

    ret->super.destructor = void___object_statement__destructor___Object_ptr;

    return ret;
}

