#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>

#include "object.h"

struct engine
{
    char   *    (*generate_id)(const uint32_t);
    void        (*object_free)(struct object   *a_object);
    Object *    (*object_alloc_void)(
        Transaction     *a_transaction,
        const char      *a_type,
        const void      *a_data,
        const uint64_t  a_data_size
    );
    Object  *   (*object_alloc)(
        Transaction     *a_transaction,
        const char      *a_type
    );
    void        (*object_prop_set_void)(
        Object          *a_object,
        const char      *a_name,
        const void      *a_data,
        const uint64_t  a_data_size
    );
    void        (*object_prop_set_str)(
        Object          *a_object,
        const char      *a_name,
        const void      *a_data
    );
    void        (*object_action_process_void)(
        Object          *a_object,
        const char      *a_name,
        const void      *a_data,
        const uint64_t  a_data_size
    );
    void        (*object_action_process_str)(
        Object          *a_object,
        const char      *a_name,
        const void      *a_data
    );
    void        (*object_action_emit_void)(
        Object          *a_object,
        const char      *a_name,
        const void      *a_data,
        const uint64_t  a_data_size
    );
    void        (*object_dump)(Object *a_object);
};
typedef struct engine Engine;

void        engine_free(Engine *a_engine);
Engine *    engine_alloc();

#endif

