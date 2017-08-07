#include "log.h"
#include <ipnoise-common/list.h>

#include "object_code_define.h"

void object_code_define__destructor_cb(
    Object *a_object)
{
    ObjectCodeDefine *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectCodeDefine, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    if (object->args_names){
        object__ref_dec((Object *)object->args_names);
        object->args_names = NULL;
    }

    if (object->instructions){
        object__ref_dec((Object *)object->instructions);
        object->instructions = NULL;
    }

    object__destructor_cb((Object *)object);
}

ObjectCodeDefine * object_code_define__constructor(
    Object  *a_prototype)
{
    ObjectCodeDefine    *object = NULL;
    uint32_t            size    = sizeof(*object);

    object = (ObjectCodeDefine *)object__constructor_uint32_t(
        "code_define",  // type
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

    object->super.destructor_cb = object_code_define__destructor_cb;
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

void object_code_define__serialize(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter,
    ObjectArray *a_args)        // function call args or NULL
{
    ObjectCodeDefine  *code_define = NULL;
    int32_t     res;

    code_define = dynamic_pointer_cast(
        ObjectCodeDefine,
        a_object,
        "code_define",
        ""
    );

    if (!code_define){
        TFATAL("missing argument: 'a_object'"
            " or it's not ObjectCodeDefine\n"
        );
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }
    if (!a_delimeter){
        TFATAL("missing argument: 'a_delimeter'\n");
    }

    object__lock(a_object);

    if (code_define->func){
        // native code_define
        object__mem_add_printf(
            a_out,
            "function %s() { [native code] }",
            object_code_define__mem_c_str(code_define)
        );
    } else {
        double  pos, length = 0;

        object__mem_add_printf(
            a_out,
            "function %s",
            code_define->super.mem
                ?   object_code_define__mem_c_str(code_define)
                :   ""
        );

        // add args
        object__mem_add_printf(a_out, "(");
        if (code_define->args_names){
            object__lock((Object *)code_define->args_names);

            object__length_get(
                (Object *)code_define->args_names,
                &length
            );

            for (pos = 0; pos < length; pos++){
                Object *arg_name = NULL;

                object_array__get_by_index(
                    code_define->args_names,
                    pos,
                    &arg_name
                );
                object__mem_add_printf(
                    a_out,
                    "%s%s",
                    pos
                        ?   ", "
                        :   "",
                    arg_name
                        ?   object__mem_c_str(arg_name)
                        :   "undefined"
                );
                if (arg_name){
                    object__ref_dec(arg_name);
                }
                if (a_args){
                    // add arg value
                    Object *val         = NULL;
                    Object *serialized  = NULL;

                    object_array__get_by_index(
                        a_args,
                        pos,
                        &val
                    );

                    if (val){
                        serialized = object__constructor(
                            a_object
                        );
                        if (!serialized){
                            object__unlock(a_object);
                            TFATAL("object__constructor()"
                                " failed\n"
                            );
                        }
                        val->serialize_cb(
                            val,
                            serialized,
                            DEFAULT_DELIMETER
                        );
                        object__ref_dec(val);
                        val = NULL;
                    }

                    object__mem_add_printf(
                        a_out,
                        " = %s",
                        serialized
                            ?   object__mem_c_str(serialized)
                            :   "undefined"
                    );

                    if (serialized){
                        object__ref_dec(serialized);
                    }
                }
            }

            object__unlock((Object *)code_define->args_names);
        }
        object__mem_add_printf(a_out, ")");

        // add code_define
        object__mem_add_printf(a_out, " {");

        if (code_define->instructions){
            object__length_get(
                (Object *)code_define->instructions,
                &length
            );
            for (pos = 0; pos < length; pos++){
                Object  *cur_object = NULL;

                res = object_array__get_by_index(
                    code_define->instructions,
                    pos,
                    &cur_object
                );
                if (res){
                    object__unlock(a_object);
                    TFATAL("cannot get array item"
                        " by index: '%g'\n",
                        pos
                    );
                }

                if (cur_object){
                    Object  *serialized = NULL;

                    OBJECT_CHECK(cur_object);

                    serialized = object__constructor(
                        a_object
                    );
                    if (!serialized){
                        object__ref_dec(cur_object);
                        object__unlock(a_object);
                        TFATAL("object__constructor() failed\n");
                    }
                    cur_object->to_string_cb(
                        cur_object,
                        serialized
                    );
                    object__mem_add_printf(a_out,
                        "%s ",
                        object__mem_c_str(serialized)
                    );
                    object__ref_dec(serialized);
                    object__ref_dec(cur_object);
                } else {
                    object__mem_add_printf(a_out,
                        "undefined"
                    );
                }
            }
        }

        object__mem_add_printf(a_out, "}");
    }

    object__unlock(a_object);
}

void object_code_define__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    object_code_define__serialize(
        a_object,
        a_out,
        a_delimeter,
        NULL            // args
    );
}

const char * object_code_define__mem_c_str(
    ObjectCodeDefine *a_object)
{
    const char *ret = NULL;

    OBJECT_CHECK(a_object);

    ret = object__mem_c_str((Object *)a_object);

    return ret;
}

