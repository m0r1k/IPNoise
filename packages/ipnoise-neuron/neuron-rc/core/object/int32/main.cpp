#include <stdlib.h>

#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/dir/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>

#include "main.hpp"

ObjectInt32::ObjectInt32(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
    m_val = 0;
}

ObjectInt32::~ObjectInt32()
{
}

string ObjectInt32::getType()
{
    return ObjectInt32::s_getType();
}

int32_t ObjectInt32::do_init_as_prop(
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
        m_val = atoi(a_data);
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

int32_t ObjectInt32::do_init_as_object(
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
        m_val = atoi(a_data);
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

ObjectSptr ObjectInt32::copy()
{
    ObjectInt32Sptr ret;

    ret = OBJECT<ObjectInt32>(m_val);

    return ret;
}

void ObjectInt32::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        Object::getAllProps(a_out);
    }
}

ObjectStringSptr ObjectInt32::toString()
{
    return serializeAsProp();
}

ObjectStringSptr ObjectInt32::serializeAsProp(
    const char  *a_delim)
{
    char                buffer[512] = { 0x00 };
    ObjectStringSptr    ret;

    snprintf(buffer, sizeof(buffer),
        "%d",
        m_val
    );

    ret = OBJECT<ObjectString>(buffer);

    return ret;
}

int32_t ObjectInt32::loadAsProp(
    const char  *a_dir)
{
    char        buffer[512] = { 0x00 };
    int32_t     res, err    = 0;
    string      dir         = getObjectPath();
    FILE        *file       = NULL;
    string      val;
    string      val_path;

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

int32_t ObjectInt32::saveAsProp(
    const char  *a_dir)
{
    int32_t     ret     = -1;
    string      dir     = getObjectPath();
    int64_t     res;

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
            "%d",
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

int32_t ObjectInt32::getVal()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_val;
}

void ObjectInt32::setVal(
    const int32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_val = a_val;
}

void ObjectInt32::incVal(
    const uint32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_val += a_val;
}

void ObjectInt32::decVal(
    const uint32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_val -= a_val;
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectInt32::s_getType()
{
    return "core.object.int32";
}

int32_t ObjectInt32::s_init(
    EngineInfo *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectInt32::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectInt32::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectInt32(a_create_flags));

    return object;
}

void ObjectInt32::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_int32 = {
    .getType        = ObjectInt32::s_getType,
    .init           = ObjectInt32::s_init,
    .shutdown       = ObjectInt32::s_shutdown,
    .objectCreate   = ObjectInt32::s_objectCreate,
    .getTests       = ObjectInt32::s_getTests
};

