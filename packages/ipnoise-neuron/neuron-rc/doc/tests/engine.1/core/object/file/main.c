#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/list.h>

#include "object.h"
#include "engine.h"
#include "module.h"

#include "main.h"

int32_t  g_debug_level = 0;

Object * constructor(
    const void      *a_data,
    const uint64_t  a_data_size)
{
    ObjectFile  *object = NULL;
    uint32_t    size    = sizeof(*object);

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

out:
    return (Object *)object;
fail:
    goto out;
}

void destructor(
    Object *a_object)
{
    ObjectFile *object = NULL;
    object = container_of(a_object, ObjectFile, super);

#ifdef __x86_64__
    PDEBUG(100, "0x%lx destructor\n",
        (uint64_t)a_object
    );
#else
    PDEBUG(100, "0x%x destructor\n",
        (uint32_t)a_object
    );
#endif

    if (object->fname){
        free(object->fname);
        object->fname = NULL;
    }
}

void action(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size)
{
    ObjectFile *object = NULL;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }
    if (!strlen(a_name)){
        PFATAL("empty argument: 'a_name'\n");
    }

    object = container_of(a_object, ObjectFile, super);

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

    if (!strcmp("to.stream.of.bytes", a_name)){
        action_to_stream_of_bytes(
            object,
            a_data,
            a_data_size
        );
    } else {
#ifdef __x86_64__
        PERROR("object: '%s' (0x%lx),"
            " unsupported action: '%s'\n",
            object->super.id,
            (uint64_t)object,
            a_name
        );
#else
        PERROR("object: '%s' (0x%x),"
            " unsupported action: '%s'\n",
            object->super.id,
            (uint32_t)object,
            a_name
        );
#endif
    }

//out:
    return;
//fail:
//    goto out;
}

void action_to_stream_of_bytes(
    ObjectFile      *a_object,
    const void      *a_data,
    const uint64_t  a_data_size)
{
    uint64_t    size  = 0;
    uint8_t     c     = 0;
    char        *mode = "r";
    FILE        *file = NULL;
    int32_t     res;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }
    if (!a_data){
        PFATAL("missing argument: 'a_data'\n");
    }
    if (!a_data_size){
        PFATAL("empty argument: 'a_data_size'\n");
    }

    if (a_object->fname){
        free(a_object->fname);
        a_object->fname = NULL;
    }

    size            = a_data_size + 1;
    a_object->fname = malloc(size);
    if (!a_object->fname){
#ifdef __x86_64__
        PERROR("cannot allocate memory,"
            " was need: '%lu' byte(s)",
            size
        );
#else
        PERROR("cannot allocate memory,"
            " was need: '%llu' byte(s)",
            size
        );
#endif
        goto fail;
    }
    memcpy(a_object->fname, a_data, a_data_size);
    a_object->fname[a_data_size] = '\0';

    PWARN("a_object->fname: '%s'\n", a_object->fname);

    file = fopen(a_object->fname, mode);
    if (!file){
        PERROR("cannot open: '%s' for: '%s'\n",
            a_object->fname,
            mode
        );
        goto fail;
    }

    do {
        res = fread(&c, 1, sizeof(c), file);
        if (0 < res){
            PWARN("was read byte: '%c'\n", c);
        }
    } while (0 < res);

    fclose(file);

out:
    return;
fail:
    goto out;
}

void action_load(
    ObjectFile      *a_object,
    const void      *a_data,
    const uint64_t  a_data_size)
{
    uint64_t size = 0;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }
    if (!a_data){
        PFATAL("missing argument: 'a_data'\n");
    }
    if (!a_data_size){
        PFATAL("empty argument: 'a_data_size'\n");
    }

    if (a_object->fname){
        free(a_object->fname);
        a_object->fname = NULL;
    }

    size            = a_data_size + 1;
    a_object->fname = malloc(size);
    if (!a_object->fname){
#ifdef __x86_64__
        PERROR("cannot allocate memory,"
            " was need: '%lu' byte(s)",
            size
        );
#else
        PERROR("cannot allocate memory,"
            " was need: '%llu' byte(s)",
            size
        );
#endif
        goto fail;
    }
    memcpy(a_object->fname, a_data, a_data_size);
    a_object->fname[a_data_size] = '\0';

    PWARN("a_object->fname: '%s'\n", a_object->fname);

out:
    return;
fail:
    goto out;
}

