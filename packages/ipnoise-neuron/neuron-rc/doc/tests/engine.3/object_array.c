#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include <ipnoise-common/list.h>

#include "object.h"
#include "object_number.h"
#include "object_string.h"

#include "object_array.h"

void object_array__destructor_cb(
    Object *a_object)
{
    //ObjectArray *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    //object = container_of(a_object, ObjectArray, super);

    object__destructor_cb(a_object);
}

ObjectArray * object_array__constructor(
    Object *a_prototype)
{
    ObjectArray    *object = NULL;
    Object          *obj    = NULL;
    uint32_t        size    = sizeof(*object);

    object = (ObjectArray *)object__constructor_uint32_t(
        "object",
        "Array",
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

    object->super.destructor_cb = object_array__destructor_cb;
    object->super.serialize_cb  = object_array__serialize_cb;

    object__length_set(obj, 0);

out:
    return object;
fail:
    if (object){
        object__ref_dec(obj);
        object = NULL;
    }
    goto out;
}

// ---------------- set ----------------

void  object_array__set_by_index(
    ObjectArray     *a_object,
    const double    a_index,
    Object          *a_val)
{
    double  length = 0;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    if (0 > a_index){
        TFATAL("0 > a_index (%f)\n", a_index);
    }

    // get cur array length
    object__length_get((Object *)a_object, &length);

    // replace value
    object__prop_set_double_obj(
        (Object *)a_object,
        a_index,
        a_val
    );

    // update length if need
    if (length <= a_index){
        object__length_set((Object *)a_object, a_index + 1);
    }
}

// ---------------- get ----------------

//
// value will be returned with reference count +1
// return 0 - if results was placed in a_out
//
int32_t object_array__get_by_index(
    ObjectArray     *a_object,
    const double    a_index,
    Object          **a_out)
{
    int32_t res, err = -1;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    res = object__prop_get_double_obj(
        (Object *)a_object,
        a_index,
        a_out
    );
    if (res){
        err = res;
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

//
// value will be returned with reference count +1
//
Object * object_array__get_last(
    ObjectArray *a_object)
{
    Object  *ret    = NULL;
    double  length  = 0;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object__lock((Object *)a_object);

    object__length_get(
        (Object *)a_object,
        &length
    );

    if (0 < length){
        object_array__get_by_index(
            a_object,
            length - 1,
            &ret
        );
    }

    object__unlock((Object *)a_object);

    return ret;
}

// ---------------- push ----------------

void object_array__push_obj(
    ObjectArray     *a_object,
    Object          *a_val)
{
    Object  *object = (Object *)a_object;
    double  length  = 0;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    // lock object for transaction (set val + update size)
    object__lock(object);

    object__length_get(object, &length);
    object__prop_set_double_obj(object, length, a_val);
    object__length_set(object, ++length);

    object__unlock(object);
}

void object_array__push_str(
    ObjectArray     *a_object,
    const char      *a_str)
{
    //Object          *obj    = (Object *)a_object;
    ObjectString    *val    = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }
    if (!a_str){
        TFATAL("missing argument: 'a_str'\n");
    }

    val = object_string__constructor_str(
        a_str,
        (Object *)a_object
    );
    if (!val){
        TFATAL("object_string__constructor() failed\n");
    }

    object_array__push_obj(a_object, (Object *)val);
}

void object_array__push_double(
    ObjectArray    *a_object,
    const double    a_val)
{
    ObjectNumber *val = NULL;

    val = object_number__constructor_double(
        a_val,
        (Object *)a_object
    );
    if (!val){
        TFATAL("object_number__constructor_double() failed\n");
    }

    object_array__push_obj(a_object, (Object *)val);
}

// ---------------- shift ----------------

// cut and return first array element
// return 0 if result was placed in a_out
int32_t object_array__shift(
    ObjectArray *a_object,
    Object      **a_out)
{
    int32_t err         = -1;
    double  pos, length = 0;

    object__lock((Object *)a_object);

    object__length_get((Object *)a_object, &length);

    for (pos = 0; pos < length; pos++){
        Object *val = NULL;

        object_array__get_by_index(
            a_object,
            pos,
            &val
        );

        if (!pos){
            *a_out  = val;
            err     = 0;
            continue;
        }

        // move all another elements
        object_array__set_by_index(
            a_object,
            pos - 1,
            val
        );
    }

    // free last element
    if (0 < length){
        Object *tmp = NULL;

        tmp = object_array__pop_obj(a_object);
        if (tmp){
            object__ref_dec(tmp);
        }
    }

    object__unlock((Object *)a_object);

    return err;
}

// ---------------- pop ----------------

Object * object_array__pop_obj(
    ObjectArray *a_object)
{
    Object  *ret    = NULL;
    double  length  = 0;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    // lock object for transaction (get val + update size)
    object__lock((Object *)a_object);

    // get current size
    object__length_get(
        (Object *)a_object,
        &length
    );

    if (0 < length){
        object_array__get_by_index(
            a_object,
            length - 1,
            &ret
        );
        object__prop_del_double(
            (Object *)a_object,
            length - 1
        );

        // update length
        object__length_set(
            (Object *)a_object,
            length - 1
        );
    }

    object__unlock((Object *)a_object);

    return ret;
}
// ---------------- serialize ----------------

void object_array__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    ObjectArray     *object = NULL;
    double          length  = 0;
    int32_t         res, i;

    object = dynamic_pointer_cast(
        ObjectArray,
        a_object,
        "object",
        "Array"
    );
    if (!object){
        TFATAL("missing argument: 'a_object'"
            " or it's not ObjectArray\n"
        );
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }
    if (!a_delimeter){
        TFATAL("missing argument: 'a_delimeter'\n");
    }

    object__lock(a_object);
    object__lock(a_out);

    object__mem_add_printf(a_out, "[");

    // get current size
    object__length_get(a_object, &length);

    for (i = 0; i < length; i++){
        Object *val = NULL;

        if (i){
            object__mem_add_printf(a_out, ", ");
        }
        res = object_array__get_by_index(
            object,
            i,
            &val
        );
        if (res){
            object__unlock(a_out);
            object__unlock(a_object);
            TFATAL("cannot get val by index: '%d'\n", i);
        }
        if (val){
            object__serialize(
                val,
                a_out,
                a_delimeter
            );
            object__ref_dec(val);
        } else {
            object__mem_add_printf(a_out, "undefined");
        }
    }

    object__mem_add_printf(a_out, "]");

    object__unlock(a_out);
    object__unlock(a_object);
}

int32_t object_array__copy_to_list(
    ObjectArray     *a_object,
    ListItem        *a_list)
{
    double      pos, length = 0;
    int32_t     res, err    = -1;

    OBJECT_CHECK(a_object);
    if (!a_list){
        TFATAL("missing argument: 'a_list'\n");
    }

    object__lock((Object *)a_object);

    object__length_get(
        (Object *)a_object,
        &length
    );

    for (pos = 0; pos < length; pos++){
        Object      *val        = NULL;
        ListItem    *new_item   = NULL;

        res = object_array__get_by_index(
            a_object,
            pos,
            &val
        );
        if (res){
            PERROR("cannot get element by index: '%g'\n",
                pos
            );
            object__unlock((Object *)a_object);
            goto fail;
        }

        new_item = list_item_alloc();
        if (!new_item){
            PERROR("list_item_alloc() failed\n");
            object__unlock((Object *)a_object);
            goto fail;
        }

        new_item->object = val;

        list_add_tail(
            &new_item->list,
            &a_list->list
        );
    }

    object__unlock((Object *)a_object);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

