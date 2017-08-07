#include <stdint.h>
#include <string.h>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/list.h>

#include "object.h"
#include "module.h"

#include "main.h"

int32_t g_debug_level = 0;

Object * constructor(
    const void      *a_data,
    const uint64_t  a_data_size)
{
    ObjectSplitter  *object = NULL;
    uint32_t        size    = sizeof(*object);

    object = calloc(1, size);
    if (!object){
        PERROR("cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            size
        );
        goto fail;
    }

#ifdef __x86_64__
    PDEBUG(100, "0x%lx constructor\n",
        (uint64_t)object
    );
#else
    PDEBUG(100, "0x%x constructor\n",
        (uint32_t)object
    );
#endif


    object->delim      = (uint8_t *)strdup(" ");
    object->delim_pos  = 0;
    object->delim_size = strlen((const char *)object->delim);
    object->strict     = 0;

out:
    return (Object *)object;
fail:
    goto out;
}

void destructor(
    Object *a_object)
{
    ObjectSplitter *object = NULL;
    object = container_of(a_object, ObjectSplitter, super);

#ifdef __x86_64__
    PDEBUG(100, "0x%lx destructor\n",
        (uint64_t)a_object
    );
#else
    PDEBUG(100, "0x%x destructor\n",
        (uint32_t)a_object
    );
#endif

    if (object->delim){
        free(object->delim);
        object->delim = NULL;
    }
}

void action(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size)
{
    ObjectSplitter *object = NULL;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }
    if (!strlen(a_name)){
        PFATAL("empty argument: 'a_name'\n");
    }

    object = container_of(a_object, ObjectSplitter, super);

#ifdef __x86_64__
    PDEBUG(100, "0x%lx action: '%s'\n",
        (uint64_t)object,
        a_name
    );
#else
    PDEBUG(100, "0x%x action: '%s'\n",
        (uint32_t)object,
        a_name
    );
#endif

    if (!strcmp("parse", a_name)){
        action_parse(object, a_data, a_data_size);
    } else if (!strcmp("delim.set", a_name)){
        action_delim_set(object, a_data, a_data_size);
    } else {
        PERROR("object: '%s',"
            " unsupported action: '%s'\n",
            object->super.id,
            a_name
        );
    }

//out:
    return;
//fail:
//    goto out;
}

void action_parse(
    ObjectSplitter  *a_object,
    const void      *a_data,
    const uint64_t  a_data_size)
{
    uint8_t     *data       = (uint8_t *)a_data;
    uint8_t     *data_start = NULL;
    uint64_t    i;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }
    if (!a_data){
        PFATAL("missing argument: 'a_data'\n");
    }
    if (!a_data_size){
        PFATAL("empty argument: 'a_data_size'\n");
    }

    a_object->delim_pos = 0;

    for (i = 0; i < a_data_size; i++){
        uint8_t c = data[i];

        if (    a_object->delim
            &&  a_object->delim_size)
        {
            if (a_object->delim[a_object->delim_pos] == c){
                a_object->delim_pos++;
                if (a_object->delim_pos == a_object->delim_size){
                    // delim fully matched
                    if (data_start){
                        char        *chunk  = NULL;
                        uint64_t    size    = 0;

                        size = data + i - data_start;
                        size -= a_object->delim_pos - 1;

                        chunk = malloc(size + 1);
                        memcpy(chunk, data_start, size);
                        chunk[size] = '\0';

                        PWARN("'%s'\n", chunk);

                        free(chunk);
                    } else if (a_object->strict){
                        PWARN("empty delim\n");
                    }
                    a_object->delim_pos = 0;
                    data_start          = NULL;
                }
            } else {
                i -= a_object->delim_pos;
                a_object->delim_pos = 0;
                if (!data_start){
                    data_start = data + i;
                }
            }
        }
    }

    if (a_data_size){
        if (data_start){
            char        *chunk  = NULL;
            uint64_t    size    = 0;

            size = data + i - data_start;
            size -= a_object->delim_pos;

            chunk = malloc(size + 1);
            memcpy(chunk, data_start, size);
            chunk[size] = '\0';

            PWARN("'%s'\n", chunk);

            free(chunk);
        } else if (a_object->strict){
            PWARN("empty delim\n");
        }
    }
}

void action_delim_set(
    ObjectSplitter  *a_object,
    const void      *a_data,
    const uint64_t  a_data_size)
{
    uint8_t *delim  = NULL;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }
    if (!a_data){
        PFATAL("empty argument: 'a_data'\n");
    }
    if (!a_data_size){
        PFATAL("empty argument: 'a_data_size'\n");
    }

    // allocate new delim
    delim = malloc(a_data_size);
    if (!delim){
#ifdef __x86_64__
        PERROR("object: '%s' (0x%lx),"
            " cannot allocate memory,"
            " was needed: '%ld' byte(s)\n",
            a_object->super.id,
            (uint64_t)a_object,
            a_data_size
        );
#else
        PERROR("object: '%s' (0x%x),"
            " cannot allocate memory,"
            " was needed: '%lld' byte(s)\n",
            a_object->super.id,
            (uint32_t)a_object,
            a_data_size
        );
#endif
        goto fail;
    }

    // copy
    memcpy(delim, a_data, a_data_size);

    // free old delim
    if (a_object->delim){
        free(a_object->delim);
        a_object->delim = NULL;
    }

    // setup new delim
    a_object->delim       = delim;
    a_object->delim_size  = a_data_size;

out:
    return;
fail:
    goto out;
}

