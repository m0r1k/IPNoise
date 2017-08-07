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

#include "object_string.h"

void object_string__destructor_cb(
    Object *a_object)
{
    ObjectString *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectString, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    object__destructor_cb((Object *)object);
}

ObjectString * object_string__constructor(
    Object *a_prototype)
{
    ObjectString    *object = NULL;
    Object          *obj    = NULL;
    uint32_t        size    = sizeof(*object);

    object = (ObjectString *)object__constructor_uint32_t(
        "string",   // type
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

    object->super.destructor_cb = object_string__destructor_cb;
    object->super.serialize_cb  = object_string__serialize_cb;
    object->super.to_string_cb  = object_string__to_string_cb;

    object__length_set(obj, 0);

    // populate methods
//    object__code_define(
//        obj,
//        "charAt",
//        object_string__api_charAt
//    );

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

ObjectString * object_string__constructor_str(
    const char  *a_val,
    Object      *a_prototype)
{
    ObjectString    *ret = NULL;
    double          len  = 0;

    if (!a_val){
        TFATAL("missing argument: 'a_val'\n");
    }

    ret = object_string__constructor(a_prototype);
    if (!ret){
        TFATAL("object_string__constructor()\n");
    }

    object__mem_set_copy_str((Object *)ret, a_val);

    object__length_set((Object *)ret, len);

    return ret;
}

ObjectString * object_string__constructor_raw(
    const char  *a_data,
    uint64_t    a_data_size,
    Object      *a_prototype)
{
    ObjectString    *ret = NULL;

    if (!a_data){
        TFATAL("missing argument: 'a_data'\n");
    }

    ret = object_string__constructor(a_prototype);
    if (!ret){
        TFATAL("object_string__constructor()\n");
    }

    object__mem_set_copy_raw(
        (Object *)ret,
        a_data,
        a_data_size
    );

    object__length_set((Object *)ret, a_data_size);

    return ret;
}

void object_string__realloc_and_concat(
    char        **a_out,
    const char  *a_format,
    ...)
{
    va_list     ap;
    int32_t     res, len = 0;
    char        *tmp     = NULL;

    if (!*a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    if (!a_format){
        TFATAL("missing argument: 'a_format'\n");
    }

    // store cur value length
    len = strlen(*a_out);

    va_start(ap, a_format);

    res = vasprintf(&tmp, a_format, ap);
    if (0 >= res){
        va_end(ap);
        TFATAL("vasprintf failed, res: '%d'\n", res);
    }
    if (!tmp){
        va_end(ap);
        TFATAL("vasprintf failed\n");
    }

    *a_out = (char *)(realloc(*a_out, len + res + 1));

    strncpy(&(*a_out)[len], tmp, res);

    va_end(ap);
}

void object_string__serialize_cb(
    Object      *a_object,
    Object      *a_out,
    const char  *a_delimeter)
{
    ObjectString    *object = NULL;
    uint8_t         c;

    object = dynamic_pointer_cast(
        ObjectString,
        a_object,
        "string",
        ""
    );
    if (!object){
        TFATAL("missing argument: 'a_object'"
            " or it's not ObjectString\n"
        );
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }
    if (!a_delimeter){
        TFATAL("missing argument: 'a_delimeter'\n");
    }

    object__mem_add_printf(a_out, "\"");
    if (object->super.mem){
        uint8_t *pos = NULL;

        pos = (uint8_t *)object->super.mem;
        do {
            c = *pos;
            if ('\0' == c){
                break;
            } else if ('"' == c){
                object__mem_add_printf(a_out, "\\%c", c);
            } else {
                object__mem_add_printf(a_out, "%c", c);
            }
            pos++;
        } while (1);
    }
    object__mem_add_printf(a_out, "\"");
}

void object_string__to_string_cb(
    Object      *a_object,
    Object      *a_out)
{
    ObjectString    *object = NULL;
    uint8_t         *pos    = NULL;
    uint8_t         c;

    object = dynamic_pointer_cast(
        ObjectString,
        a_object,
        "string",
        ""
    );
    if (!object){
        TFATAL("missing argument: 'a_object'"
            " or it's not ObjectString\n"
        );
    }
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    if (object->super.mem){
        object__lock(a_object);

        pos = (uint8_t *)object->super.mem;

        do {
            c = *pos;
            if ('\0' == c){
                break;
            } else {
                object__mem_add_printf(a_out, "%c", c);
            }
            pos++;
        } while (1);

        object__unlock(a_object);
    }
}

int32_t object_string__charAt(
    ObjectString    *a_object,
    const double    a_pos,
    uint8_t         *a_out)
{
    int32_t err     = -1;
    double  length  = 0;

    OBJECT_CHECK(a_object);
    if (!a_out){
        TFATAL("missing argument: 'a_out'\n");
    }

    object__lock((Object *)a_object);

    // setup default
    *a_out = 0;

    // get string length
    object__length_get((Object *)a_object, &length);

    if (length <= a_pos){
        // out of range
        goto fail;
    }

    // prepare ret
    *a_out = ((uint8_t *)a_object->super.mem)[
        (uint64_t)a_pos
    ];

    // all ok
    err = 0;

out:
    object__unlock((Object *)a_object);
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

ObjectString * object_string__char_at_str(
    ObjectString    *a_object,
    const uint32_t  a_pos)
{
    ObjectString    *ret = NULL;
    uint8_t         c    = 0;
    int32_t         res;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    res = object_string__charAt(
        a_object,
        a_pos,
        &c
    );
    if (res){
        goto out;
    }

    // prepare ret
    ret = object_string__constructor((Object *)a_object);
    if (!ret){
        TFATAL("object_string__constructor() failed\n");
    }
    object__mem_add_printf((Object *)ret, "%c", c);
    object__length_set((Object *)ret, 1);

out:
    return ret;
}

int32_t object_string__cmp_str(
    ObjectString    *a_object,
    const char      *a_str)
{
    int32_t ret = 0;

    OBJECT_CHECK(a_object);
    if (!a_str){
        PFATAL("missing argument: 'a_str'\n");
    }

    ret = strcmp(
        object_string__mem_c_str(a_object),
        a_str
    );

    return ret;
}

int32_t object_string__cmp_obj(
    ObjectString    *a_left,
    ObjectString    *a_right)
{
    int32_t ret = 0;

    OBJECT_CHECK(a_left);
    OBJECT_CHECK(a_right);

    ret = strcmp(
        object_string__mem_c_str(a_left),
        object_string__mem_c_str(a_right)
    );

    PWARN("compare string,"
        " left: '%s',"
        " right: '%s',"
        " res: '%d'"
        "\n",
        object_string__mem_c_str(a_left),
        object_string__mem_c_str(a_right),
        ret
    );

    return ret;
}

int32_t object_string__to_int32_t(
    ObjectString    *a_object)
{
    int32_t ret = 0;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    ret = atoi(object_string__mem_c_str(a_object));

    return ret;
}

double object_string__to_double(
    ObjectString    *a_object)
{
    double ret = 0.0f;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    ret = atof(object_string__mem_c_str(a_object));

    return ret;
}

// ---------------- api ----------------

/*
void object_string__api_charAt(
    Object          *a_object,
    ObjectArray     *a_args,
    ListItem        **a_return_value)
{
    ObjectString    *ret            = NULL;
    ObjectString    *object         = NULL;
    Object          *val            = NULL;
    ObjectNumber    *arg_pos        = NULL;
    double          pos             = 0;
    int32_t         res;

    object = dynamic_pointer_cast(
        ObjectString,
        a_object,
        "string",
        ""
    );
    if (!object){
        TFATAL("missing argument: 'a_object'"
            " or it's not ObjectString\n"
        );
    }

    object__lock((Object *)a_object);

    // it will increase val's ref count
    res = object_array__get_by_index(a_args, 0, &val);
    if (!res){
        object__unlock((Object *)a_object);
        TFATAL("cannot get argument 'pos'\n");
    }
    arg_pos = dynamic_pointer_cast(
        ObjectNumber,
        val,
        "number",
        ""
    );
    if (!arg_pos){
        object__ref_dec(val);
        object__unlock((Object *)a_object);
        TFATAL("argument: 'pos' is not 'int32' (%s)\n",
            val->type
        );
    }

    pos = object_number__to_double(arg_pos);
    ret = object_string__char_at_str(object, pos);

    object__ref_dec((Object *)arg_pos);
    object__unlock((Object *)a_object);

    return (Object *)ret;
}
*/

int32_t object_string__load_from_file(
    ObjectString    *a_object,
    ObjectString    *a_path)
{
    int32_t         err         = -1;
    int32_t         res, fd     = -1;
    size_t          res2        = 0;
    size_t          was_read    = 0;
    size_t          size        = 0;
    struct stat     st;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }
    if (!a_path){
        TFATAL("missing argument: 'a_path'\n");
    }

    // open file
    fd = open(object_string__mem_c_str(a_path), O_RDONLY);
    if (0 > fd){
        TFATAL("cannot open: '%s' for read\n",
            object_string__mem_c_str(a_path)
        );
    }

    // read size
    res = fstat(fd, &st);
    if (res){
        TFATAL("stat for fd: '%d', file: '%s' failed\n",
            fd,
            object_string__mem_c_str(a_path)
        );
    }

    if (a_object->super.mem){
        free(a_object->super.mem);
        a_object->super.mem_size = 0;
        a_object->super.mem      = NULL;
    }

    a_object->super.mem_size = st.st_size + 1;
    size                     = st.st_size;

    a_object->super.mem = malloc(a_object->super.mem_size);
    if (!a_object->super.mem){
#ifdef __x86_64__
        TFATAL("cannot allocate memory,"
            " was needed: '%lu' byte(s)\n",
            a_object->super.mem_size
        );
#else
        TFATAL("cannot allocate memory,"
            " was needed: '%u' byte(s)\n",
            a_object->super.mem_size
        );
#endif
    }

    while (size){
        res2 = read(fd, a_object->super.mem + was_read, size);
        if (0 < res2){
            was_read += res2;
            size     -= res2;
        } else if (!res2){
            break;
        } else {
#ifdef __x86_64__
            TFATAL("cannot read file: '%s',"
                " fd: '%d',"
                " res: '%ld'\n",
                object_string__mem_c_str(a_path),
                fd,
                res2
            );
#else
            TFATAL("cannot read file: '%s',"
                " fd: '%d',"
                " res: '%d'\n",
                object_string__mem_c_str(a_path),
                fd,
                res2
            );
#endif
        }

        ((uint8_t *)a_object->super.mem)[was_read] = '\0';

        if (was_read != st.st_size){
#ifdef __x86_64__
            TFATAL("partial read, from file: '%s',"
                " fd: '%d',"
                " was_read: '%lu' != st.st_size: '%lu'\n",
                object_string__mem_c_str(a_path),
                fd,
                was_read,
                st.st_size
            );
#else
            TFATAL("partial read, from file: '%s',"
                " fd: '%d',"
                " was_read: '%u' != st.st_size: '%u'\n",
                object_string__mem_c_str(a_path),
                fd,
                was_read,
                st.st_size
            );
#endif
        }
    }

    close(fd);

    object__length_set(
        (Object *)a_object,
        (double)was_read
    );

    // all ok
    err = 0;

    return err;
}

void object_string__remove_last_char(
    ObjectString    *a_string)
{
    double  length = 0;

    if (!a_string){
        TFATAL("missing argument: 'a_string'\n");
    }

    object__lock((Object *)a_string);

    object__length_get(
        (Object *)a_string,
        &length
    );

    if (0 < length){
        length--;
        ((uint8_t *)a_string->super.mem)[
            (uint64_t)length
        ] = '\0';
        object__length_set(
            (Object *)a_string,
            length
        );
    }

    object__unlock((Object *)a_string);
}

const char * object_string__mem_c_str(
    ObjectString *a_object)
{
    const char *ret = NULL;

    OBJECT_CHECK(a_object);

    ret = object__mem_c_str((Object *)a_object);

    return ret;
}

