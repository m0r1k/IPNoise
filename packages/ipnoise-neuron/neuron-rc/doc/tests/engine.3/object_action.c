#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include <ipnoise-common/list.h>

#include "object_code_define.h"

#include "object_action.h"

void object_action__destructor_cb(
    Object *a_object)
{
    ObjectAction *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectAction, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    object__destructor_cb((Object *)object);
}

ObjectAction * object_action__constructor(
    const char  *a_name,
    Object      *a_prototype)
{
    ObjectAction    *object = NULL;
    Object          *obj    = NULL;
    uint32_t        size    = sizeof(*object);

    if (    !a_name
        ||  !strlen(a_name))
    {
        TFATAL("missing argument: 'a_name'\n");
    }

    object = (ObjectAction *)object__constructor_uint32_t(
        "object",   // type
        "Action",   // class name
        size,
        a_prototype
    );
    if (!object){
        PERROR("cannot allocate memory,"
            " was needed: '%u' byte(s)\n",
            size
        );
        goto fail;
    }
    obj = (Object *)object;

    object->super.mem           = strdup(a_name);
    object->super.destructor_cb = object_action__destructor_cb;
    //object->super.serialize_cb  = object__serialize_cb;
    //object->super.to_string_cb  = object__to_string__cb;

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

out:
    return object;
fail:
    if (object){
        object__ref_dec(obj);
        object = NULL;
    }
    goto out;
}

