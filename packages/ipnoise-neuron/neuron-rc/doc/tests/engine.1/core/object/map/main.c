#include <stdint.h>
#include <string.h>
#include <ipnoise-common/log_common.h>
#include "object.h"
#include "module.h"

#include "main.h"

int32_t g_debug_level = 0;

int32_t constructor(
    Object *a_object)
{
    int32_t err = -1;

#ifdef __x86_64__
    PDEBUG(100, "0x%lx constructor\n",
        (uint64_t)a_object
    );
#else
    PDEBUG(100, "0x%x constructor\n",
        (uint32_t)a_object
    );
#endif

    // all ok
    err = 0;

//out:
    return err;
//fail:
//    goto out;
}

void destructor(
    Object *a_object)
{
#ifdef __x86_64__
    PDEBUG(100, "0x%lx destructor\n",
        (uint64_t)a_object
    );
#else
    PDEBUG(100, "0x%x destructor\n",
        (uint32_t)a_object
    );
#endif
}

void action(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size)
{
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

#ifdef __x86_64__
    PDEBUG(100, "0x%lx action: '%s'\n",
        (uint64_t)a_object,
        a_name
    );
#else
    PDEBUG(100, "0x%x action: '%s'\n",
        (uint32_t)a_object,
        a_name
    );
#endif

    if (0){
    } else {
        PERROR("object: '%s',"
            " unsupported action: '%s'\n",
            a_object->id,
            a_name
        );
    }
}

