#include "log.h"
#include "libevent.h"
#include <ipnoise-common/list.h>

#include "object_connection.h"

void object_connection__destructor_cb(
    Object *a_object)
{
    ObjectConnection *object = NULL;

    if (!a_object){
        TFATAL("missing argument: 'a_object'\n");
    }

    object = container_of(a_object, ObjectConnection, super);

#ifdef __x86_64__
    PDEBUG(100, "object: (0x%lx)\n", (uint64_t)object);
#else
    PDEBUG(100, "object: (0x%x)\n", (uint32_t)object);
#endif

    if (object->be){
        bufferevent_free(object->be);
        object->be = NULL;
    }

    object__destructor_cb((Object *)object);
}

ObjectConnection * object_connection__constructor(
    Object *a_prototype)
{
    ObjectConnection    *object = NULL;
    uint32_t            size    = sizeof(*object);

    object = (ObjectConnection *)object__constructor_uint32_t(
        "object",       // type
        "Connection",   // class name
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

    object->super.destructor_cb = object_connection__destructor_cb;
    object->super.serialize_cb  = object__serialize_cb;

    // populate methods
    object__code_define((Object *)object,
        "close", object_connection__api_close
    );


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

Object *    object_connection__api_close(
    Object          *a_context,
    Object          *a_position,    // file position or NULL
    ObjectArray     *a_args)
{
    Object              *ret        = NULL;
    ObjectConnection    *connection = NULL;

    OBJECT_CHECK(a_context);
    OBJECT_CHECK(a_args);

    connection = dynamic_pointer_cast(
        ObjectConnection,
        a_context,
        "object",
        "Connection"
    );
    if (!connection){
        TFATAL("a_connection type is not object connection"
            " (type: '%s', class: '%s')"
            "\n",
            a_context->type,
            a_context->class_name
        );
    }

    bufferevent_free(connection->be);
    connection->be = NULL;

//    object__ref_dec((Object *)connection);

    return ret;
}

