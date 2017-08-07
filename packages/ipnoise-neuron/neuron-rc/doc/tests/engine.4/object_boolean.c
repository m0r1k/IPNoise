#include "log.h"
#include "object.h"

#include "object_boolean.h"

#define object_boolean__destructor // for ctags

void void___object_boolean__destructor___Object_ptr(
    Object *a_object)
{
    void___object__destructor___Object_ptr(a_object);
}

#define object_boolean__constructor // for ctags

ObjectBoolean * ObjectBoolean_ptr___object_boolean__constructor()
{
    ObjectBoolean *ret = NULL;

    ret = (ObjectBoolean *)
        Object_ptr___object__constructor___const_uint32t(
            sizeof(*ret)
        );

    ret->super.destructor = void___object_boolean__destructor___Object_ptr;

    return ret;
}

#define object_boolean__set_val // for ctags

int32_t int32_t___object_boolean__set_val___ObjectBoolean_ptr__const_double(
    ObjectBoolean   *a_object,
    const double    a_val)
{
    int32_t err  = -1;

    OBJECT_CHECK(a_object);

    err = int32t___object__mem_set___Object_ptr__const_double(
        (Object *)a_object,
        a_val
    );

    return err;
}

