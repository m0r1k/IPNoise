#include "log.h"
#include <ipnoise-common/list.h>

#include "object_timer.h"

void object_timer__destructor_cb(
    Object *a_object)
{
    ObjectTimer *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectTimer, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    if (object->context){
        object__ref_dec(object->context);
        object->context = NULL;
    }

    if (object->code_define){
        //PWARN("object->code_define.super->ref_count: '%d'\n",
        //    object->code_define->super.ref_count
        //);
        object__ref_dec((Object *)object->code_define);
        object->code_define = NULL;
    }

    if (object->ev){
        event_free(object->ev);
        object->ev = NULL;
    }

    object__destructor_cb((Object *)object);
}

ObjectTimer * object_timer__constructor(
    Object *a_prototype)
{
    ObjectTimer  *object = NULL;
    uint32_t        size    = sizeof(*object);

    object = (ObjectTimer *)object__constructor_uint32_t(
        "timer",    // type
        "",         // class name
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

    object->super.destructor_cb = object_timer__destructor_cb;
    object->super.serialize_cb  = object_timer__serialize_cb;

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

out:
    return object;
fail:
    if (object){
        object__ref_dec((Object *)object);
        object = NULL;
    }
    goto out;
}

void object_timer__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    ObjectTimer  *timer = NULL;

    timer = dynamic_pointer_cast(
        ObjectTimer,
        a_object,
        "timer",
        ""
    );

    if (!timer){
        TFATAL("missing argument: 'a_object'"
            " or it's not 'timer' (%s)\n",
            a_object->type
        );
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }
    if (!a_delimeter){
        TFATAL("missing argument: 'a_delimeter'\n");
    }

    object_code_define__serialize(
        (Object *)timer->code_define,
        a_out,
        a_delimeter,
        NULL // TODO call arguments must be here
    );
}

