#include <stdlib.h>

#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/dir/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>

#include "main.hpp"

ObjectDouble::ObjectDouble(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
    m_val = 0.0f;
}

ObjectDouble::~ObjectDouble()
{
}

string ObjectDouble::getType()
{
    return ObjectDouble::s_getType();
}

int32_t ObjectDouble::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = 0;

    res = Object::do_init_as_prop(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    if (a_data){
        m_val = atof(a_data);
        setDirty(1);
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

int32_t ObjectDouble::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = 0;

    res = Object::do_init_as_object(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    if (a_data){
        m_val = atof(a_data);
        setDirty(1);
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

ObjectSptr ObjectDouble::copy()
{
    ObjectDoubleSptr    ret;

    ret = OBJECT<ObjectDouble>(m_val);

    return ret;
}

void ObjectDouble::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        Object::getAllProps(a_out);
    }
}

ObjectStringSptr ObjectDouble::serializeAsProp(
    const char  *a_delim)
{
    char                buffer[512] = { 0x00 };
    ObjectStringSptr    ret;

    snprintf(buffer, sizeof(buffer),
        "%f",
        m_val
    );

    ret = OBJECT<ObjectString>(buffer);

    return ret;
}

int32_t ObjectDouble::loadAsProp(
    const char  *a_dir)
{
    char        buffer[512] = { 0x00 };
    int32_t     res, err    = 0;
    string      dir         = getObjectPath();
    FILE        *file       = NULL;
    string      val;
    string      val_path;

/*
    res = Object::load(a_dir);
    if (res){
        PERROR("cannot load object\n");
        goto fail;
    }
*/

    if (a_dir){
        dir = a_dir;
    }

    if (dir.empty()){
        PERROR("cannot load object,"
            " unknown object directory\n"
        );
        goto fail;
    }

    if ('/' != dir.at(dir.size() - 1)){
        // add '/' to end if not exist
        dir += "/";
    }

    // load val
    do {
        val_path = dir + OBJECT_FILE_VAL;
        file     = fopen(val_path.c_str(), "r");
        if (!file){
            break;
        }

        do {
            res = fread(buffer, 1, sizeof(buffer), file);
            if (0 < res){
                val.append(buffer, res);
            } else if (0 > res){
                fclose(file);
                goto fail;
            } else {
                break;
            }
        } while (0 < res);

        fclose(file);

        err = do_init_as_prop(buffer, strlen(buffer));
    } while (0);

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

int32_t ObjectDouble::saveAsProp(
    const char  *a_dir)
{
    int32_t     ret     = -1;
    string      dir     = getObjectPath();
    int64_t     res;

    res = Object::saveAsProp(a_dir);
    if (res){
        goto fail;
    }

    if (a_dir){
        dir = a_dir;
    }

    if (dir.empty()){
        PERROR("cannot save object,"
            " unknown object directory\n"
        );
        goto fail;
    }

    if ('/' != dir.at(dir.size() - 1)){
        // add '/' to end if not exist
        dir += "/";
    }

    PDEBUG(10, "attempt to save object to dir: '%s'\n",
        dir.c_str()
    );

    // check what object's dir exist
    res = ObjectDir::s_mkdir(dir.c_str());
    if (res){
        PERROR("cannot crate dir: '%s'\n", dir.c_str());
        goto fail;
    }

    // save val
    {
        FILE    *file       = NULL;
        char    buffer[512] = { 0x00 };
        int32_t size        = 0;
        string  full_path;
        string  mode;

        full_path = dir;
        full_path += "/val";

        mode = "w";
        file = fopen(full_path.c_str(), mode.c_str());
        if (!file){
            PERROR("cannot open file: '%s' for mode: '%s'\n",
                full_path.c_str(),
                mode.c_str()
            );
            goto fail;
        }

        size = snprintf(buffer, sizeof(buffer),
            "%f",
            m_val
        );
        if (0 >= size){
            fclose(file);
            PERROR("snprintf failed, size: '%d'\n", size);
            goto fail;
        }

        res = fwrite(buffer, 1, size, file);
        if (res != size){
            fclose(file);
#ifdef __x86_64__
            PERROR("partial save file: '%s',"
                " attempt to save: '%ld' byte(s),"
                " but was wrote: '%ld' byte(s)\n",
                full_path.c_str(),
                int64_t(size),
                int64_t(res)
            );
#else
            PERROR("partial save file: '%s',"
                " attempt to save: '%d' byte(s),"
                " but was wrote: '%d' byte(s)\n",
                full_path.c_str(),
                int32_t(size),
                int32_t(res)
            );
#endif
            goto fail;
        }

        fclose(file);
    }

    // all ok
    ret = 0;

out:
    return ret;
fail:
    goto out;
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectDouble::s_getType()
{
    return "core.object.double";
}

int32_t ObjectDouble::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectDouble::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectDouble::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectDouble(a_create_flags));

    return object;
}

void ObjectDouble::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_double = {
    .getType        = ObjectDouble::s_getType,
    .init           = ObjectDouble::s_init,
    .shutdown       = ObjectDouble::s_shutdown,
    .objectCreate   = ObjectDouble::s_objectCreate,
    .getTests       = ObjectDouble::s_getTests
};

