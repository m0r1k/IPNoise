#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include <ipnoise-common/list.h>

#include "object_code_define.h"

#include "object_number.h"

void object_number__destructor_cb(
    Object *a_object)
{
    ObjectNumber *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectNumber, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    object__destructor_cb((Object *)object);
}

ObjectNumber * object_number__constructor(
    Object  *a_prototype)
{
    ObjectNumber    *object = NULL;
    Object          *obj    = NULL;
    uint32_t        size    = sizeof(*object);

    object = (ObjectNumber *)object__constructor_uint32_t(
        "number",   // type
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

    object->super.destructor_cb = object_number__destructor_cb;
    object->super.serialize_cb  = object_number__serialize_cb;
    object->super.to_string_cb  = object_number__to_string__cb;

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

ObjectNumber * object_number__constructor_double(
    const double    a_val,
    Object          *a_prototype)
{
    ObjectNumber    *ret = NULL;
    double          val  = a_val;

    ret = object_number__constructor(a_prototype);
    if (!ret){
        TFATAL("object_number__constructor() failed\n");
    }

    ret->super.mem_size = sizeof(val);
    ret->super.mem      = malloc(ret->super.mem_size);
    if (!ret->super.mem){
#ifdef __x86_64__
        TFATAL("cannot allocate memory,"
            " was needed: '%lu' byte(s)\n",
            ret->super.mem_size
        );
#else
        TFATAL("cannot allocate memory,"
            " was needed: '%u' byte(s)\n",
            ret->super.mem_size
        );
#endif
    }
    memcpy(ret->super.mem, &val, ret->super.mem_size);

    return ret;
}

ObjectNumber * object_number__constructor_str(
    const char  *a_val,
    Object      *a_prototype)
{
    ObjectNumber *ret = NULL;
    double       val  = 0.0f;

    if (!a_val){
        TFATAL("missing argument: 'a_val'\n");
    }

    val = atof(a_val);
    ret = object_number__constructor_double(val, a_prototype);

    return ret;
}

void object_number__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    ObjectNumber *object = NULL;

    OBJECT_CHECK(a_object);

    object = dynamic_pointer_cast(
        ObjectNumber,
        a_object,
        "number",
        ""
    );
    if (!object){
        object__add_error(
            a_object,
            a_object, // position
            "argument: 'a_object'"
            " is not ObjectNumber (%s)\n",
            a_object->type
        );
        goto fail;
    }
    if (!a_out){
        object__add_error(
            a_object,
            a_object, // position
            "missing argument: 'a_out'\n"
        );
        goto fail;
    }
    if (!a_delimeter){
        object__add_error(
            a_object,
            a_object, // position
            "missing argument: 'a_delimeter'\n"
        );
        goto fail;
    }

    object_number__to_string__cb(
        a_object,
        a_out
    );

out:
    return;

fail:
    goto out;
}

void object_number__to_string__cb(
    Object  *a_object,
    Object  *a_out)
{
    ObjectNumber *object = NULL;

    OBJECT_CHECK(a_object);

    object = dynamic_pointer_cast(
        ObjectNumber,
        a_object,
        "number",
        ""
    );
    if (!object){
        TFATAL("a_object type is not 'number' (%s)\n",
            a_object->type
        );
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    if (object->nan){
        object__mem_add_printf(a_out, "NaN");
    } else {
        object__mem_add_printf(
            a_out,
            "%.16g",
            *((double *)object->super.mem)
        );
    }
}

void object_number__inc(
    ObjectNumber    *a_object,
    const double    a_val)
{
    Object  *obj = (Object *)a_object;
    double  *val = NULL;

    OBJECT_CHECK(a_object);

    object__lock(obj);
    val  = (double *)obj->mem;
    *val = *val + a_val;
    object__unlock(obj);
}

void object_number__dec(
    ObjectNumber    *a_object,
    const double    a_val)
{
    Object  *obj = (Object *)a_object;
    double  *val = NULL;

    OBJECT_CHECK(a_object);

    object__lock(obj);
    val  = (double *)obj->mem;
    *val = *val - a_val;
    object__unlock(obj);
}

double object_number__to_double(
    ObjectNumber *a_object)
{
    double ret = 0;

    OBJECT_CHECK(a_object);

    ret = *((double *)a_object->super.mem);

    return ret;
}

int32_t object_number__to_int32_t(
    ObjectNumber *a_object)
{
    int32_t ret = 0;

    OBJECT_CHECK(a_object);

    ret = *((double *)a_object->super.mem);

    return ret;
}

