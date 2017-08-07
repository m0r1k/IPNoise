#include <stdint.h>
#include <string.h>

#include <ipnoise-common/log_common.h>
#include <ipnoise-common/list.h>
#include <ipnoise-common/hexdump.h>

#include "object.h"
#include "module.h"

#include "main.h"

int32_t g_debug_level = 0;

Object * constructor(
    const void      *a_data,
    const uint64_t  a_data_size)
{
    ObjectDumper    *object = NULL;
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

out:
    return (Object *)object;
fail:
    goto out;
}

void destructor(
    Object *a_object)
{
    ObjectDumper *object = NULL;
    object = container_of(a_object, ObjectDumper, super);

#ifdef __x86_64__
    PDEBUG(100, "0x%lx destructor\n",
        (uint64_t)object
    );
#else
    PDEBUG(100, "0x%x destructor\n",
        (uint32_t)object
    );
#endif
}

void action(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size)
{
    ObjectDumper *object = NULL;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }
    if (!strlen(a_name)){
        PFATAL("empty argument: 'a_name'\n");
    }

    object = container_of(a_object, ObjectDumper, super);

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

#ifdef __x86_64__
    PWARN("object: '%s' (0x%lx),"
        " new action: '%s' received\n",
        object->super.id,
        (uint64_t)object,
        a_name
    );
#else
    PWARN("object: '%s' (0x%x),"
        " new action: '%s' received\n",
        object->super.id,
        (uint32_t)object,
        a_name
    );
#endif

    ipnoise_hexdump(
        a_data,
        a_data_size
    );

    return;
}

