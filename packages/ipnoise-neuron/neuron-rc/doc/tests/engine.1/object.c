#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>

#include <ipnoise-common/log_common.h>
#include "dir.h"
#include "engine.h"

#include "object.h"

void object_free(
    Object *a_object)
{
    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    // call destructor if exist
    if (a_object->destructor){
        a_object->destructor(a_object);
    }

    // free module
    module_free(a_object->module);
    a_object->module = NULL;

    // free object id
    if (a_object->id){
        free(a_object->id);
        a_object->id = NULL;
    }

    // free object
    free(a_object);
}

Object * object_alloc_void(
//    Transaction     *a_transaction,
    const char      *a_type,
    const void      *a_data,
    const uint64_t  a_data_size)
{
    char        fname[65535]    = { 0x00 };
    Object      *object         = NULL;
    Module      *module         = NULL;
    Constructor constructor     = NULL;
//    char        *dir            = NULL;
//    int32_t     res;

    // prepare module name
    snprintf(fname, sizeof(fname),
        "%s/%s.so",
        MODULES_DIR,
        a_type
    );

    // load module
    module = module_load(fname);
    if (!module){
        PERROR("cannot load module: '%s'\n",
            fname
        );
        goto fail;
    }

    // search constructor symbol
    constructor = (Constructor)dlsym(
        module->dl,
        "constructor"
    );
    if (!constructor){
        PERROR("cannot create object from: '%s',"
            " constructor not found\n",
            fname
        );
        goto fail;
    }

    // alloc object
    object = constructor(a_data, a_data_size);
    if (!object){
        PERROR("cannot allocate object,"
            " from: '%s'\n",
            fname
        );
        goto fail;
    }

    // store module to object
    object->module = module;

    // generate id
    if (!object->id){
        object->id = generate_id(OBJECT_ID_BYTES);
    }

    // lookup symbols
    object->destructor = (void (*)(Object *))dlsym(
        module->dl,
        "destructor"
    );
    object->action = (
        void (*)(
            Object          *,
            const char      *,
            const void      *,
            const uint64_t
        )
    ) dlsym(
        module->dl,
        "action"
    );

/*
    // get object's dir
    dir = object_dir(
        a_transaction,
        object
    );
    if (!dir){
        PERROR("cannot get object dir\n");
        goto fail;
    }

    // create object's dir
    res = mkdirr(dir, DEFAULT_MKDIR_MODE);
    if (res){
        PERROR("cannot create object's dir: '%s'\n", dir);
        free(dir);
        goto fail;
    }

    free(dir);
*/

out:
    return object;
fail:
    if (object){
        // object was allocated
        object_free(object);
        object = NULL;
    } else if (module){
        // object was not allocated
        module_free(module);
        module = NULL;
    }
    goto out;
}

Object * object_alloc(
//    Transaction     *a_transaction,
    const char      *a_type)
{
    Object *ret = NULL;

    ret = object_alloc_void(
//        a_transaction,
        a_type,
        NULL,
        0
    );

    return ret;
}

void object_prop_set_void(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size)
{
    char        dir[65536]      = { 0x00 };
    char        fname[65536]    = { 0x00 };
    FILE        *file           = NULL;
    const char  *mode           = "w";
    int32_t     res;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

    PDEBUG(100, "object: '%s',"
        " attempt to set prop: '%s'\n",
        a_object->id,
        a_name
    );

    // prepare dir name
    res = snprintf(dir, sizeof(dir),
        "%s/%s/%s",
        OBJECTS_DIR,
        a_object->id,
        OBJECT_PROPS_DIR
    );
    if (0 >= res){
        PFATAL("snprintf failed, res: '%d'\n", res);
    }

    // create dir
    res = mkdirr(dir, 0755);
    if (res){
        PFATAL("mkdirr for: '%s' failed, res: '%d'\n",
            dir,
            res
        );
    }

    // prepare file name
    res = snprintf(fname, sizeof(fname),
        "%s/%s",
        dir,
        a_name
    );
    if (0 >= res){
        PFATAL("snprintf failed, res: '%d'\n", res);
    }

    PDEBUG(100, "object: '%s',"
        " attempt save prop: '%s' to: '%s'\n",
        a_object->id,
        a_name,
        fname
    );

    // write file
    file = fopen(fname, mode);
    if (!file){
        PFATAL("cannot open file: '%s' for: '%s'\n",
            fname,
            mode
        );
    }

    res = fwrite(a_data, 1, a_data_size, file);
    if (0 >= res){
        PFATAL("cannot write to file: '%s', res: '%d'\n",
            fname,
            res
        );
    }

    fclose(file);
}

void object_prop_set_str(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data)
{
    uint64_t len = 0;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

    len = strlen(a_data);
    if (0 < len){
        object_prop_set_void(
            a_object,
            a_name,
            a_data,
            len
        );
    }
}

void object_action_process_void(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size)
{
    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

    a_object->action(
        a_object,
        a_name,
        a_data,
        a_data_size
    );
}

void object_action_process_str(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data)
{
    uint64_t    data_size = 0;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }
    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }
    if (!a_data){
        PFATAL("missing argument: 'a_data'\n");
    }

    data_size = strlen(a_data);

    object_action_process_void(
        a_object,
        a_name,
        a_data,
        data_size
    );
}

void object_child_unlink(
//    Transaction *a_transaction,
    Object      *a_parent,
    Object      *a_child)
{
    char        buffer[65535]  = { 0x00 };
    char        *dir_parent    = NULL;
    int32_t     res;
    struct stat st;

//    if (!a_transaction){
//        PFATAL("missing argument: 'a_transaction'\n");
//    }
    if (!a_parent){
        PFATAL("missing argument: 'a_parent'\n");
    }
    if (!a_child){
        PFATAL("missing argument: 'a_child'\n");
    }

#ifdef __x86_64__
    PDEBUG(100, "object: '%s' (%lx),"
        " child unlink: '%s' (%lx)\n",
        a_parent->id,
        (uint64_t)a_parent,
        a_child->id,
        (uint64_t)a_child
    );
#else
    PDEBUG(100, "object: '%s' (%x),"
        " child unlink: '%s' (%x)\n",
        a_parent->id,
        (uint32_t)a_parent,
        a_child->id,
        (uint32_t)a_child
    );
#endif

    dir_parent = object_dir(
//        a_transaction,
        a_parent
    );
    if (!dir_parent){
        PERROR("cannot get parent dir\n");
        goto fail;
    }

    // prepare symlink name
    snprintf(buffer, sizeof(buffer),
        "%s/child/%s",
        dir_parent,
        a_child->id
    );

    // attempt to delet symlink if path exist
    res = stat(buffer, &st);
    if (!res){
        res = unlink(buffer);
        if (res){
            PERROR("cannot unlink: '%s'\n",
                buffer
            );
            goto fail;
        }
    }

    // all ok

out:
    if (dir_parent){
        free(dir_parent);
        dir_parent = NULL;
    }
    return;
fail:
    goto out;
}

void object_child_link(
//    Transaction *a_transaction,
    Object      *a_parent,
    Object      *a_child)
{
    char        buffer[65535]  = { 0x00 };
    char        *dir_parent    = NULL;
    char        *dir_child     = NULL;
    int32_t     res;
    struct stat st;

//    if (!a_transaction){
//        PFATAL("missing argument: 'a_transaction'\n");
//    }
    if (!a_parent){
        PFATAL("missing argument: 'a_parent'\n");
    }
    if (!a_child){
        PFATAL("missing argument: 'a_child'\n");
    }

#ifdef __x86_64__
    PDEBUG(100, "object: '%s' (%lx),"
        " child link: '%s' (%lx)\n",
        a_parent->id,
        (uint64_t)a_parent,
        a_child->id,
        (uint64_t)a_child
    );
#else
    PDEBUG(100, "object: '%s' (%x),"
        " child link: '%s' (%x)\n",
        a_parent->id,
        (uint32_t)a_parent,
        a_child->id,
        (uint32_t)a_child
    );
#endif

    dir_parent = object_dir(
//        a_transaction,
        a_parent
    );
    if (!dir_parent){
        PERROR("cannot get parent dir\n");
        goto fail;
    }

    dir_child = object_dir(
//        a_transaction,
        a_child
    );
    if (!dir_child){
        PERROR("cannot get child dir\n");
        goto fail;
    }

    // create child dir
    snprintf(buffer, sizeof(buffer),
        "%s/child",
        dir_parent
    );
    res = mkdirr(buffer, DEFAULT_MKDIR_MODE);
    if (res){
        PERROR("mkdirr: '%s' failed\n",
            buffer
        );
        goto fail;
    }

    // prepare symlink name
    snprintf(buffer, sizeof(buffer),
        "%s/child/%s",
        dir_parent,
        a_child->id
    );

    // attempt to create symlink if path not exist
    res = stat(buffer, &st);
    if (res){
        res = symlink(dir_child, buffer);
        if (res){
            PERROR("cannot create symlink"
                " from: '%s' -> '%s'\n",
                dir_child,
                buffer
            );
            goto fail;
        }
    }

    // all ok

out:
    if (dir_child){
        free(dir_child);
        dir_child = NULL;
    }
    if (dir_parent){
        free(dir_parent);
        dir_parent = NULL;
    }
    return;
fail:
    goto out;
}

char * object_dir(
//    Transaction     *a_transaction,
    Object          *a_object)
{
    char buffer[65535]  = { 0x00 };
    char *ret           = NULL;
//    char *trans_dir     = NULL;

//    if (!a_transaction){
//        PFATAL("missing argument: 'a_transaction'\n");
//    }
    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

//    trans_dir = transaction_dir(a_transaction);
//    if (!trans_dir){
//        PERROR("cannot get trans dir\n");
//        goto fail;
//    }

    snprintf(buffer, sizeof(buffer),
        "%s/%s",
//        trans_dir,
        OBJECTS_DIR,
        a_object->id
    );

    ret = strdup(buffer);
    if (!ret){
        PERROR("strdup() failed\n");
        goto fail;
    }

out:
//    if (trans_dir){
//        free(trans_dir);
//        trans_dir = NULL;
//    }
    return ret;
fail:
    goto out;
}

void object_action_emit_void(
    Object          *a_object,
    const char      *a_name,
    const void      *a_data,
    const uint64_t  a_data_size)
{
#ifdef __x86_64__
    PWARN("object: '%s' (%lx),"
        " emit action: '%s'\n",
        a_object->id,
        (uint64_t)a_object,
        a_name
    );
#else
    PWARN("object: '%s' (%x),"
        " emit action: '%s'\n",
        a_object->id,
        (uint32_t)a_object,
        a_name
    );
#endif
}

void object_dump(
    Object *a_object)
{
    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

#ifdef __x86_64__
    PWARN("0x%lx, dump:\n"
        "ID: %s\n",
        (uint64_t)a_object,
        a_object->id ? a_object->id : "NULL"
    );
#else
    PWARN("0x%x, dump:\n"
        "ID: %s\n",
        (uint32_t)a_object,
        a_object->id ? a_object->id : "NULL"
    );
#endif
}

