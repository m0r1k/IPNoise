#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>

#include <ipnoise-common/log_common.h>
#include "id.h"
#include "object.h"

#include "engine.h"

void engine_free(
    Engine *a_engine)
{
    if (!a_engine){
        PFATAL("missing argument: 'a_engine'\n");
    }

    free(a_engine);
}

Engine * engine_alloc()
{
    Engine      *engine = NULL;
    uint32_t    size    = sizeof(*engine);

    engine = calloc(1, size);
    if (!engine){
        PERROR("cannot allocate memory,"
            " was needed: '%d' bute(s)\n",
            size
        );
        goto fail;
    }

    engine->generate_id             = generate_id;
    engine->object_free             = object_free;
//    engine->object_alloc_void       = object_alloc_void;
//    engine->object_alloc            = object_alloc;
    engine->object_prop_set_void    = object_prop_set_void;
    engine->object_prop_set_str     = object_prop_set_str;
    engine->object_action_process_void = object_action_process_void;
    engine->object_action_process_str  = object_action_process_str;
    engine->object_action_emit_void = object_action_emit_void;
    engine->object_dump             = object_dump;

out:
    return engine;
fail:
    if (engine){
        engine_free(engine);
        engine = NULL;
    }
    goto out;
}

