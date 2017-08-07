#include "log.h"
#include <ipnoise-common/list.h>

#include "object_code_call.h"

void object_code_call__destructor_cb(
    Object *a_object)
{
    ObjectCodeCall *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectCodeCall, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    if (object->args){
        object__ref_dec((Object *)object->args);
        object->args = NULL;
    }

    if (object->code_define){
        object__ref_dec((Object *)object->code_define);
        object->code_define = NULL;
    }

    //if (object->context){
    //    object__ref_dec(object->context);
    //}

    object__destructor_cb((Object *)object);
}

ObjectCodeCall * object_code_call__constructor(
    Object *a_prototype)
{
    ObjectCodeCall  *object = NULL;
    uint32_t        size    = sizeof(*object);

    object = (ObjectCodeCall *)object__constructor_uint32_t(
        "code_call",    // type
        "",             // class name
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

    object->super.destructor_cb = object_code_call__destructor_cb;
    object->super.serialize_cb  = object__serialize_cb;

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

void object_code_call__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    ObjectCodeCall  *code_call = NULL;

    code_call = dynamic_pointer_cast(
        ObjectCodeCall,
        a_object,
        "code_call",
        ""
    );

    if (!code_call){
        TFATAL("missing argument: 'a_object'"
            " or it's not 'code_call' (%s)\n",
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
        (Object *)code_call->code_define,
        a_out,
        a_delimeter,
        code_call->args
    );
}

Object * object_code_call__call_with_args(
    ObjectCodeDefine    *a_code_define,
    Object              *a_context,
    Object              *a_position,
    ObjectArray         *a_args)
{
    Object              *ret            = NULL;
    ListItem            *code_item      = NULL;
    ObjectCodeCall      *code_call      = NULL;
    Flags               flags           = 0;

    OBJECT_CHECK(a_context);
    OBJECT_CHECK(a_code_define);
    OBJECT_CHECK(a_args);

    code_item   = list_item_alloc();
    code_call   = object_code_call__constructor(a_context);

    object__ref_inc((Object *)a_args);
    code_call->args = a_args;

    object__ref_inc((Object *)a_code_define);
    code_call->code_define = a_code_define;

    code_item->object = (Object *)code_call;

    object__item_resolve(
        a_context,
        a_position,
        code_item,
        &flags
    );

    list_item_free(code_item);

    return ret;
}

