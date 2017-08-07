#include "log.h"
#include "object.h"

#include "object_window.h"

void void___object_window__destructor___Object_ptr(
    Object *a_object)
{
    void___object__destructor___Object_ptr(a_object);
}

ObjectWindow * ObjectWindow_ptr___object_window__constructor()
{
    ObjectWindow *ret = NULL;

    ret = (ObjectWindow *)
        Object_ptr___object__constructor___const_uint32t(
            sizeof(*ret)
        );

    ret->super.destructor = void___object_window__destructor___Object_ptr;

    return ret;
}


