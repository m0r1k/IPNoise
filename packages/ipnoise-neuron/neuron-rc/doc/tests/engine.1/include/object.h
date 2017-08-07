#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include <ipnoise-common/list.h>

#define MODULES_DIR         "modules"
#define OBJECTS_DIR         "/db/objects"
#define OBJECT_PROPS_DIR    "props"

#include "id.h"
#include "transaction.h"
#include "module.h"

struct object
{
    Module          *module;
    char            *id;
    void            (*destructor)(struct object *);
    void            (*action)(
        struct object   *a_object,
        const char      *a_name,
        const void      *a_data,
        const uint64_t  a_data_size
    );
};
typedef struct object Object;

typedef Object * (*Constructor)(
    const void      *,
    const uint64_t
);

typedef void (*EngineSet)(void *);

void        object_free(Object *a_object);
Object *    object_alloc_void(
//    Transaction     *a_transaction,
    const char      *a_type,
    const void      *a_data,
    const uint64_t  a_data_size
);
Object *    object_alloc(
//    Transaction     *a_transaction,
    const char      *a_type
);
void        object_prop_set_void(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size
);
void object_prop_set_str(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data
);
void        object_action_process_void(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size
);
void        object_action_process_str(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data
);
void        object_child_unlink(
//    Transaction *a_transaction,
    Object      *a_parent,
    Object      *a_child
);
void        object_child_link(
//    Transaction     *a_transaction,
    Object          *a_parent,
    Object          *a_child
);
char *      object_dir(
//    Transaction     *a_transaction,
    Object          *a_object
);
void        object_action_emit_void(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size
);
void        object_dump(Object *a_object);

#endif

