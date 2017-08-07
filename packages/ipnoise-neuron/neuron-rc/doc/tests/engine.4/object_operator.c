#include "log.h"
#include "object.h"

#include "object_operator.h"

#define object_operator__destructor // for ctags

void void___object_operator__destructor___Object_ptr(
    Object *a_object)
{
    void___object__destructor___Object_ptr(a_object);
}

#define object_operator__constructor // for ctags

ObjectOperator * ObjectOperator_ptr___object_operator__constructor()
{
    ObjectOperator *ret = NULL;

    ret = (ObjectOperator *)
        Object_ptr___object__constructor___const_uint32t(
            sizeof(*ret)
        );

    ret->super.destructor = void___object_operator__destructor___Object_ptr;

    return ret;
}

