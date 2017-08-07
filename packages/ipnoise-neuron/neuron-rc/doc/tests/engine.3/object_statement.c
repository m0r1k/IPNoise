#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include <ipnoise-common/list.h>

#include "object_statement.h"

void object_statement__destructor_cb(
    Object *a_object)
{
    ObjectStatement *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectStatement, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    object__destructor_cb((Object *)object);
}

ObjectStatement * object_statement__constructor(
    Object *a_prototype)
{
    ObjectStatement *object = NULL;
    Object          *obj    = NULL;
    uint32_t        size    = sizeof(*object);

    object = (ObjectStatement *)object__constructor_uint32_t(
        "statement",  // type
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
    obj = (Object *)object;

    object->super.destructor_cb = object_statement__destructor_cb;
    object->super.serialize_cb  = object_statement__serialize_cb;
    object->super.to_string_cb  = object_statement__to_string__cb;

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

void object_statement__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    ObjectStatement *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = dynamic_pointer_cast(
        ObjectStatement,
        a_object,
        "statement",
        ""
    );
    if (!object){
        TFATAL("argument: 'a_object'"
            " is not 'statement' (%s)\n",
            a_object->type
        );
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }
    if (!a_delimeter){
        TFATAL("missing argument: 'a_delimeter'\n");
    }

    object_statement__to_string__cb(
        a_object,
        a_out
    );
}

void object_statement__to_string__cb(
    Object  *a_object,
    Object  *a_out)
{
    ObjectStatement *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'");
    }

    object = dynamic_pointer_cast(
        ObjectStatement,
        a_object,
        "statement",
        ""
    );
    if (!object){
        TFATAL("a_object type is not 'statement' (%s)\n",
            a_object->type
        );
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    object__mem_add_printf(
        a_out,
        "%s",
        object__mem_c_str((Object *)object)
    );
}

const char * object_statement__mem_c_str(
    ObjectStatement *a_object)
{
    const char *ret = NULL;

    OBJECT_CHECK(a_object);

    ret = object__mem_c_str((Object *)a_object);

    return ret;
}

