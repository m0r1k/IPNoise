#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/dir/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>

#include "main.hpp"

ObjectLinks::ObjectLinks(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectLinks::~ObjectLinks()
{
}

string ObjectLinks::getType()
{
    return ObjectLinks::s_getType();
}

int32_t ObjectLinks::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_prop(a_data, a_data_size);

    m_vals = OBJECT<ObjectMap>(
        a_data,
        a_data_size,
        CREATE_PROP
    );
    if (a_data){
        setDirty(1);
    }
    return err;
}

int32_t ObjectLinks::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = 0;

    res = Object::do_init_as_object(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
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

ObjectSptr ObjectLinks::copy()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectLinksSptr ret;

    ret = OBJECT<ObjectLinks>();
    ret->setVals(dynamic_pointer_cast<ObjectMap>(
        m_vals->copy()
    ));

    return ret;
}

void ObjectLinks::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        Object::getAllProps(a_out);
    }
}

ObjectStringSptr ObjectLinks::toString()
{
    return serializeAsProp();
}

ObjectStringSptr ObjectLinks::serializeAsProp(
    const char  *a_delim)
{
    return m_vals->serializeAsProp(a_delim);
}

int32_t ObjectLinks::loadAsProp(
    const char  *a_dir)
{
    char            buffer[65535]   = { 0x00 };
    int32_t         err             = 0;
    DIR             *dirp           = NULL;
    struct dirent   *dir_ent        = NULL;
    string          dir             = getObjectPath();
    string          vals_path;

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

    // load vals
    vals_path = dir;
    vals_path += OBJECT_DIR_VALS;

    //PWARN("vals_path.c_str(): '%s'\n", vals_path.c_str());

    dirp = opendir(vals_path.c_str());
    if (!dirp){
        goto out;
    }

    do {
        int32_t     res;
        ObjectSptr  val;
        string      val_path;

        dir_ent = readdir(dirp);
        if (!dir_ent){
            break;
        }

        if (    !strcmp(dir_ent->d_name, ".")
            ||  !strcmp(dir_ent->d_name, ".."))
        {
            continue;
        }

        val_path = vals_path;
        val_path += dir_ent->d_name;

        PDEBUG(50, "object: '%s', was found val: '%s'\n",
            dir.c_str(),
            val_path.c_str()
        );

        res = readlink(
            val_path.c_str(),
            buffer, sizeof(buffer)
        );
        PDEBUG(50, "readlink: '%s', buffer: '%s', res: '%d'\n",
            val_path.c_str(),
            buffer,
            res
        );
        if (0 >= res){
            PWARN("cannot read symlink: '%s'\n",
                val_path.c_str()
            );
            continue;
        }

        m_vals->add(dir_ent->d_name, buffer);
    } while (dir_ent);

out:
    if (dirp){
        closedir(dirp);
    }
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t ObjectLinks::saveAsProp(
    const char  *a_dir)
{
    int32_t     res, ret    = -1;
    string      dir         = getObjectPath();
    ObjectMapIt vals_it;
    string      vals_path;

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
        // add '/'to end if not exist
        dir += "/";
    }

    PDEBUG(15, "attempt to save object to dir: '%s'\n",
        dir.c_str()
    );

    // check what object's dir exist
    res = ObjectDir::s_mkdir(dir.c_str());
    if (res){
        PERROR("cannot crate dir: '%s'\n", dir.c_str());
        goto fail;
    }

    vals_path = dir;
    vals_path += OBJECT_DIR_VALS;

    // recreate vals dir exist
    res = ObjectDir::s_recreate_dir(vals_path.c_str());
    if (res){
        PERROR("cannot crate dir: '%s'\n",
            vals_path.c_str()
        );
        goto fail;
    }

    for (vals_it = m_vals->begin();
        vals_it != m_vals->end();
        vals_it++)
    {
        ObjectSptr key  = vals_it->first;
        ObjectSptr val  = vals_it->second;
        string     to   = val->toStringStd();
        string     from = vals_path + key->toStringStd();

        res = symlink(to.c_str(), from.c_str());
        if (res){
            PERROR("symlink from: '%s' to: '%s' failed\n",
                from.c_str(),
                to.c_str()
            );
        }
    }

    // all ok
    ret = 0;

out:
    return ret;
fail:
    goto out;
}

void ObjectLinks::add(
    const char *a_path)
{
    string id;

    if (    !a_path
        ||  !strlen(a_path))
    {
        PFATAL("missing argument: 'a_path'\n");
    }

    id = getIdFromPath(a_path);

    m_vals->add(id, a_path);

    setDirty(1);
}

void ObjectLinks::erase(
    const char *a_path)
{
    string id;

    if (    !a_path
        ||  !strlen(a_path))
    {
        PFATAL("missing argument: 'a_path'\n");
    }

    id = getIdFromPath(a_path);

    m_vals->erase(id);

    setDirty(1);
}

ObjectMapSptr ObjectLinks::getVals()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_vals;
}

void ObjectLinks::setVals(
    ObjectMapSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_vals = a_val;

    setDirty(1);
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectLinks::s_getType()
{
    return "core.object.links";
}

int32_t ObjectLinks::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectLinks::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectLinks::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectLinks(a_create_flags));

    return object;
}

void ObjectLinks::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_links = {
    .getType        = ObjectLinks::s_getType,
    .init           = ObjectLinks::s_init,
    .shutdown       = ObjectLinks::s_shutdown,
    .objectCreate   = ObjectLinks::s_objectCreate,
    .getTests       = ObjectLinks::s_getTests
};

