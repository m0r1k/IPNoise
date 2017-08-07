#include <stdio.h>
#include <stdlib.h>

#include <ipnoise-common/log_common.h>

#include "log.h"
#include "id.h"
#include "object.h"
#include "object_string.h"
#include "object_array.h"

#include "main.h"

int32_t g_debug_level   = 0;

int32_t main(
    int32_t a_argc,
    char    **a_argv)
{
    int32_t err = -1;

    if (g_debug_level){
        event_enable_debug_mode();
    }

    if (0){
        // 1e3 iterations, time per iteration:
        // hostos:  0.061989 us
        // ipnoise: 3.108978 us
        Object      *object = NULL;
        uint32_t    size    = sizeof(*object);

        object = malloc(size);
        free(object);
    }

    //if (0){
    //    // 1e3 iteration, time per iteration:
    //    // hostos:  51.884890 us
    //    // ipnoise: 57.966948 us
    //    Object *object = NULL;
    //    object = object_alloc("core.object.file");
    //
    //    if (object){
    //        object_free(object);
    //        object = NULL;
    //    }
    //}

    if (0){
        // 1e3 iteration, time per iteration:
        // ipnoise: 10.984898 us
        char *id = generate_id(20);
        free(id);
    }

    if (0){
        // 1e3 iteration, time per iteration:
        // ipnoise: 10.984898 us
        char *id = generate_id(20);
        free(id);
    }

    //if (0){
    //    // 1e3 iteration, time per iteration:
    //    // hostos:  42.474031 us
    //    // ipnoise: 18.826962 us (with ramfs)
    //    // ipnoise: 43.967962 us (without ramfs)
    //    char *id = generate_id(20);
    //    snprintf(buffer, sizeof(buffer),
    //        "%s/%s",
    //        OBJECTS_DIR,
    //        id
    //    );
    //    mkdirr(buffer, DEFAULT_MKDIR_MODE);
    //    free(id);
    //}

    //if (0){
    //    // 1e3 iteration, time per iteration:
    //    // hostos:  13.196230 us
    //    // ipnoise: 10.167837 us (with ramfs)
    //    // ipnoise: 10.060072 us (without ramfs)
    //    char *id = "123";
    //    snprintf(buffer, sizeof(buffer),
    //        "%s/%s",
    //        OBJECTS_DIR,
    //        id
    //    );
    //    mkdirr(buffer, DEFAULT_MKDIR_MODE);
    //}

    if (1){
        Object          *ret     = NULL;
        ObjectString    *path    = NULL;
        Object          *context = NULL;

        // alloc context
        context = object__constructor_window();
        if (!context){
            PERROR("object__constructor_window() failed\n");
            goto fail;
        }

        // prepare path
        path = object_string__constructor_str(
            "test.js",
            context
        );
        if (!path){
            object__ref_dec(context);
            PERROR("object_string__constructor() failed\n");
            goto fail;
        }

        // parse
        ret = object__parse_file(
            context,
            path
        );

        //object__dump("parser: '%s'\n", (Object *)parser);

        if (ret){
            object__ref_dec(ret);
            ret = NULL;
        }

        OBJECT__DUMP(context, "main, context:\n");

        object__ref_dec((Object *)path);
        // because of things in context can ref on context,
        // we cannot free it via object__ref_dec
        object__destructor_cb(context);
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

