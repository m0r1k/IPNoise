#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "log.h"
#include <ipnoise-common/list.h>

#include "object_code_define.h"
#include "object_number.h"

#include "object_ipnoise.h"

void object_ipnoise__destructor_cb(
    Object *a_object)
{
    ObjectIPNoise *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectIPNoise, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    object__destructor_cb((Object *)object);
}

ObjectIPNoise * object_ipnoise__constructor(
    Object *a_prototype)
{
    ObjectIPNoise   *object = NULL;
    Object          *obj    = NULL;
    uint32_t        size    = sizeof(*object);

    object = (ObjectIPNoise *)object__constructor_uint32_t(
        "object",   // type
        "IPNoise",  // class name
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

    object->super.destructor_cb = object_ipnoise__destructor_cb;
    object->super.serialize_cb  = object__serialize_cb;
    object->super.to_string_cb  = object__to_string_cb;

    // populate methods
    object__code_define(
        obj, "getMemInfo", object_ipnoise__api_getMemInfo
    );

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

// ---------------- api ----------------

Object * object_ipnoise__api_getMemInfo(
    Object          *a_object,
    Object          *a_position, // file position or NULL
    ObjectArray     *a_args)
{
    Object              *ret    = NULL;
    //ObjectNumber        *val    = NULL;

    OBJECT_CHECK(a_object);
    OBJECT_CHECK(a_args);

    ret = object__constructor(a_object);
    if (!ret){
        TFATAL("object__constructor() failed\n");
    }

    // arena
    //val = object_number__constructor_double(info.arena);
    //if (!val){
    //    object__ref_dec(ret);
    //    TFATAL("object_number__constructor_double() failed\n");
    //}
    //object__prop_set_str_obj(ret, "arena", (Object *)val);

    return ret;
}

