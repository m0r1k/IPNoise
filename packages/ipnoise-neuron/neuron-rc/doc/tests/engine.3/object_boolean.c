#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include <ipnoise-common/list.h>

#include "object_code_define.h"

#include "object_boolean.h"

void object_boolean__destructor_cb(
    Object *a_object)
{
    ObjectBoolean *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectBoolean, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    object__destructor_cb((Object *)object);
}

ObjectBoolean * object_boolean__constructor(
    Object *a_prototype)
{
    ObjectBoolean   *object = NULL;
    Object          *obj    = NULL;
    uint32_t        size    = sizeof(*object);

    object = (ObjectBoolean *)object__constructor_uint32_t(
        "boolean",  // type
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

    object->super.destructor_cb = object_boolean__destructor_cb;
    object->super.serialize_cb  = object_boolean__serialize_cb;
    object->super.to_string_cb  = object_boolean__to_string__cb;

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

ObjectBoolean * object_boolean__constructor_double(
    const double    a_val,
    Object          *a_prototype)
{
    ObjectBoolean   *ret = NULL;

    ret = object_boolean__constructor(a_prototype);
    if (!ret){
        TFATAL("object_boolean__constructor() failed\n");
    }

    ret->val = a_val ? 1 : 0;

    return ret;
}

ObjectBoolean * object_boolean__constructor_str(
    const char  *a_val,
    Object      *a_prototype)
{
    ObjectBoolean *ret = NULL;
    double       val  = 0.0f;

    if (!a_val){
        TFATAL("missing argument: 'a_val'\n");
    }

    val = atof(a_val);
    ret = object_boolean__constructor_double(
        val,
        a_prototype
    );

    return ret;
}

void object_boolean__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    ObjectBoolean *object = NULL;

    OBJECT_CHECK(a_object);

    object = dynamic_pointer_cast(
        ObjectBoolean,
        a_object,
        "boolean",
        ""
    );
    if (!object){
        TFATAL("argument: 'a_object'"
            " is not ObjectBoolean (%s)\n",
            a_object->type
        );
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }
    if (!a_delimeter){
        TFATAL("missing argument: 'a_delimeter'\n");
    }

    object_boolean__to_string__cb(
        a_object,
        a_out
    );
}

void object_boolean__to_string__cb(
    Object  *a_object,
    Object  *a_out)
{
    ObjectBoolean *object = NULL;

    OBJECT_CHECK(a_object);

    object = dynamic_pointer_cast(
        ObjectBoolean,
        a_object,
        "boolean",
        ""
    );
    if (!object){
        TFATAL("a_object type is not 'boolean' (%s)\n",
            a_object->type
        );
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    object__mem_add_printf(
        a_out,
        "%s",
        object->val ? "true" : "false"
    );
}

double object_boolean__to_double(
    ObjectBoolean *a_object)
{
    double ret = 0;

    OBJECT_CHECK(a_object);

    ret = a_object->val;

    return ret;
}

uint8_t object_boolean__to_uint8_t(
    ObjectBoolean *a_object)
{
    int32_t ret = 0;

    OBJECT_CHECK(a_object);

    ret = a_object->val;

    return ret;
}

int32_t object_boolean__to_int32_t(
    ObjectBoolean *a_object)
{
    int32_t ret = 0;

    OBJECT_CHECK(a_object);

    ret = a_object->val;

    return ret;
}

// return boolean equivalent for object
int32_t object_boolean__get_bool_result(
    Object  *a_val,
    uint8_t *a_out)
{
    int32_t err = -1;

    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    *a_out = 0;

    if (a_val){
        if (!strcmp(a_val->type, "boolean")){
            *a_out = ((ObjectBoolean *)a_val)->val ? 1 : 0;
        } else if (!strcmp(a_val->type, "number")){
            *a_out = *(double *)(a_val->mem) ? 1 : 0;
        } else if (!strcmp(a_val->type, "object")){
            *a_out = 1;
        } else if (!strcmp(a_val->type, "string")){
            double length = 0;
            object__length_get(a_val, &length);
            *a_out = length ? 1 : 0;
        } else {
            goto fail;
        }
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    goto out;
}

