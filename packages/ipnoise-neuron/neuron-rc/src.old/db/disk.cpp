#include <sys/stat.h>
#include <unistd.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/action/main.hpp"
#include "core/object/bin/main.hpp"
#include "core/object/dir/main.hpp"
#include "core/object/double/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/int64/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/vec3/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"
#include "db.hpp"
#include "strings.hpp"

#include "db/disk.hpp"

DbDisk::DbDisk()
    :   Db()
{
}

DbDisk::~DbDisk()
{
}

int32_t DbDisk::do_init()
{
    int32_t err = -1;

    // DB_DIR_ROOT_OBJECTS
    {
        int32_t         res;
        ObjectDirSptr   dir;

        PROP_DIR(dir, DB_DIR_OBJECTS);

        res = dir->mkdir();
        if (res){
            err = res;
            goto fail;
        }
    }

    // DB_DIR_ROOT_ACTIONS
    {
        int32_t         res;
        ObjectDirSptr   dir;

        PROP_DIR(dir, DB_DIR_ACTIONS);

        res = dir->mkdir();
        if (res){
            err = res;
            goto fail;
        }
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

void DbDisk::getActionsForCurThread(
    ObjectVectorSptr    a_out,
    const int32_t       &a_get_removed,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    PDEBUG(60, "found size: '%d'\n", a_out->size());
}

void DbDisk::getActionsForAnyThread(
    ObjectVectorSptr    a_out,
    const int32_t       &a_get_removed,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    PDEBUG(60, "found size: '%d'\n", a_out->size());
}

void DbDisk::getActionsForProcess(
    ObjectVectorSptr    a_out,
    const int32_t       &a_get_removed,
    const int32_t       &a_do_autorun)
{
    getActionsForCurThread(
        a_out,
        a_get_removed,
        a_do_autorun
    );
    getActionsForAnyThread(
        a_out,
        a_get_removed,
        a_do_autorun
    );
}

void DbDisk::getObjectsForRemoveExpired(
    ObjectVectorSptr    a_out,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    PDEBUG(60, "found size: '%d'\n", a_out->size());
}

void DbDisk::getObjectsForRemove(
    ObjectVectorSptr    a_out,
    const int32_t       &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    getObjectsForRemoveExpired(a_out, a_do_autorun);
}

void DbDisk::unlockLocationObjects()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    PINFO("unlocking objects before clean..\n");
}

void DbDisk::cleanLocationObjects()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    PINFO("clean..\n");
}

ObjectDirSptr DbDisk::getObjectDir(
    const char *a_object_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDirSptr  dir;

    if (    !a_object_id
        ||  !strlen(a_object_id))
    {
        PFATAL("missing argument: 'a_object_id'\n");
    }

    PTIMING(0.000f, "'%s'\n", a_object_id);

    PROP_DIR(dir, DB_DIR_OBJECTS);
    dir->addDir(a_object_id);

    return dir;
}

ObjectDirSptr DbDisk::getObjectDir(
    const string &a_object_id)
{
    return getObjectDir(a_object_id.c_str());
}

ObjectDirSptr DbDisk::getCreateObjectDir(
    const char *a_object_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t         res;
    ObjectDirSptr   dir;

    if (    !a_object_id
        ||  !strlen(a_object_id))
    {
        PFATAL("missing argument: 'a_object_id'\n");
    }

    PTIMING(0.000f, "'%s'\n", a_object_id);

    dir = getObjectDir(a_object_id);
    if (!dir->isExist()){
        res = dir->mkdir();
        if (res){
            PFATAL("cannot create object directory,"
                " for object with ID: '%s', res: '%d'\n",
                a_object_id,
                res
            );
        }
    }

    return dir;
}

ObjectDirSptr DbDisk::getCreateObjectDir(
    const string &a_object_id)
{
    return getCreateObjectDir(a_object_id.c_str());
}

void DbDisk::do_clean()
{
    unlockLocationObjects();
    cleanLocationObjects();
}

ObjectSptr DbDisk::_getObjectById(
    const string    &a_object_id,
    const string    &a_object_type,
    const int32_t   &a_get_removed,
    const int32_t   &a_db_lock,
    const int32_t   &a_do_autorun)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr          ret;
    ObjectDirSptr       dir;
    ObjectMapSptr       props;
    ObjectMapIt         props_it;
    ObjectStringSptr    prop_object_type;
    string              object_type;

    PTIMING(0.0f, "get object by ID: '%s', type: '%s'\n",
        a_object_id.c_str(),
        a_object_type.c_str()
    );

    if (a_object_id.empty()){
        PFATAL("missing argument: 'a_object_id'\n");
    }

    dir = getObjectDir(a_object_id);
    if (!dir->isExist()){
        // object not found
        goto out;
    }

    // get type
    prop_object_type = dir->read("type");
    if (prop_object_type){
        object_type = prop_object_type->toString();
    }
    if (object_type.empty()){
        PFATAL("cannot get object type for object: '%s'\n",
            a_object_id.c_str()
        );
    }

    if (    !a_object_type.empty()
        &&  object_type != a_object_type)
    {
        // type not matched
        goto out;
    }

    // create object
    ret = Object::createObject(
        object_type,
        a_do_autorun
    );
    if (!ret){
        PFATAL("cannot load object: '%s'"
            " (cannot create object)\n",
            a_object_id.c_str()
        );
    }

    ret->getAllProps(props);
    for (props_it = props->begin();
        props_it != props->end();
        props_it++)
    {
        ObjectStringSptr    val;
        ObjectStringSptr    prop_name;
        string              prop_name_str;
        ObjectSptr          prop_val;

        prop_name = dynamic_pointer_cast<ObjectString>(
            props_it->first
        );
        prop_name_str = prop_name->getVal();

        if ("type" == prop_name_str){
            // already read
            continue;
        }

        PWARN("'%s' trying to load prop: '%s'\n",
            a_object_id.c_str(),
            prop_name->c_str()
        );

        DbDisk::loadProp(
            dir,
            prop_name->c_str(),
            props_it->second
        );
    }

out:
    return ret;
}

void DbDisk::loadProp(
    ObjectDirSptr   a_object_dir,
    const char      *a_prop_name,
    ObjectSptr      a_prop)
{
    ObjectStringSptr    val;

    if (!a_object_dir){
        PFATAL("missing argument: 'a_object_dir'\n");
    }

    if (!a_prop_name){
        PFATAL("missing argument: 'a_prop_name'\n");
    }

    val = a_object_dir->read(a_prop_name);

    a_prop->parseBuff(val->c_str(), val->size());
}

void DbDisk::saveProp(
    ObjectDirSptr   a_dir,
    const char      *a_name,
    ObjectSptr      a_prop)
{
    FILE        *file   = NULL;
    const char  *mode   = "w";
    int64_t     res     = -1;
    string      val;

    if (!a_dir){
        PFATAL("missing argument: 'a_dir'\n");
    }

    if (!a_name){
        PFATAL("missing argument: 'a_name'\n");
    }

    if (!a_prop){
        PFATAL("missing argument: 'a_prop'\n");
    }

    // get data
    val = a_prop->toString();
    if (val.empty()){
        a_dir->unlink(a_name);
        goto out;
    }

    file = a_dir->fopen(a_name, mode);
    if (!file){
        PFATAL("cannot open file: '%s' for: '%s'\n",
            a_name,
            mode
        );
    }

    // write header
    if (0){
        char    buffer[512] = { 0x00 };
        int64_t size        = 0;

        size = val.size();
        res  = snprintf(buffer, sizeof(buffer),
            "Content-Type: '%s'\n"
            "Content-Size: '%lld'\n"
            "\n",
            a_prop->getType().c_str(),
            size
        );
        if (0 > res){
            PFATAL("snprintf failed, res: '%lld'\n", res);
        }

        size = res;
        res  = fwrite(
            buffer,
            1,
            size,
            file
        );
        if (res != size){
            PFATAL("fwrite: '%s' failed:\n"
                "  data:      '%s'\n"
                "  data size: '%lld'\n"
                "  res:       '%lld'\n",
                a_name,
                val.c_str(),
                size,
                res
            );
        }

    }

    // write data
    {
        res = fwrite(
            val.c_str(),
            1,
            val.size(),
            file
        );
        if (res != val.size()){
            PFATAL("fwrite: '%s' failed:\n"
                "  data:      '%s'\n"
                "  data size: '%lld'\n"
                "  res:       '%lld'\n",
                a_name,
                val.c_str(),
                int64_t(val.size()),
                res
            );
        }
    }

    fclose(file);

out:
    return;
}

int32_t DbDisk::insert(
    ObjectSptr a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t         res, err = -1;
    string          object_id;
    ObjectDirSptr   dir;
    ObjectMapSptr   props;
    ObjectMapIt     props_it;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    object_id = a_object->getId();

    PTIMING(0.000f, "'%s' save (insert)\n", object_id.c_str());

    dir = getCreateObjectDir(object_id);

    PROP_MAP(props);
    a_object->getAllPropsDirty(props);

    for (props_it = props->begin();
        props_it != props->end();
        props_it++)
    {
        ObjectStringSptr    prop_name;
        prop_name = dynamic_pointer_cast<ObjectString>(
            props_it->first
        );
        saveProp(
            dir,
            prop_name->c_str(),
            props_it->second
        );
    }

    if (ObjectAction::isAction(a_object)){
        ObjectDirSptr       dir_dst;
        ObjectStringSptr    src;
        ObjectStringSptr    dst;

        PROP_DIR(dir_dst, DB_DIR_ACTIONS);
        dir_dst->addDir(object_id);

        src = dir->toStringSptr();
        dst = dir_dst->toStringSptr();

        res = link(src->c_str(), dst->c_str());
        if (res){
            PFATAL("'%s' link '%s' => '%s' failed\n",
                object_id.c_str(),
                src->c_str(),
                dst->c_str()
            );
        }
    }

    //PWARN("dir: '%s'\n", dir->toString().c_str());

    // all ok
    err = 0;

    return err;
}

int32_t DbDisk::update(
    ObjectSptr a_object)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t         err = -1;
    string          object_id;
    ObjectDirSptr   dir;
    ObjectMapSptr   props;
    ObjectMapIt     props_it;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    object_id = a_object->getId();

    PTIMING(0.000f, "'%s' save (update)\n", object_id.c_str());

    dir = getCreateObjectDir(object_id.c_str());

    PROP_MAP(props);
    a_object->getAllPropsDirty(props);

    for (props_it = props->begin();
        props_it != props->end();
        props_it++)
    {
        ObjectStringSptr    prop_name;
        prop_name = dynamic_pointer_cast<ObjectString>(
            props_it->first
        );
        saveProp(
            dir,
            prop_name->c_str(),
            props_it->second
        );
    }

    // all ok
    err = 0;

    return err;
}

void DbDisk::remove(
    const string    &a_object_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    PTIMING(0.000f, "'%s' remove\n", a_object_id.c_str());

    if (a_object_id.empty()){
        PFATAL("missing argument 'a_object_id'\n");
    }
}

// ---------------- static ----------------

