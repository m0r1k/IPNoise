#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/dir/main.hpp"

ObjectDir::ObjectDir(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectDir::ObjectDir(
    const Object::CreateFlags   &a_flags,
    ObjectStringSptr            a_path)
    :   Object(a_flags)
{
    do_init(a_flags);

    if (    !a_path
        ||  a_path->empty())
    {
        PFATAL("missing argument: 'a_path'\n");
    }

    setPath(a_path->c_str());
}

ObjectDir::ObjectDir(
    const Object::CreateFlags   &a_flags,
    const char                  *a_path)
    :   Object(a_flags)
{
    do_init(a_flags);

    if (    !a_path
        ||  !strlen(a_path))
    {
        PFATAL("missing argument: 'a_path'\n");
    }

    setPath(a_path);
}

ObjectDir::~ObjectDir()
{
}

void ObjectDir::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectDir::getType()
{
    return ObjectDir::_getType();
}

void ObjectDir::do_init_props()
{
    ObjectVectorSptr    prop_dirs;

    Object::do_init_props();

    PROP_VECTOR(prop_dirs);

    INIT_PROP(this, Dirs, prop_dirs);
}

void ObjectDir::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectDir::getAllProps(
    ObjectMapSptr a_props)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SAVE_PROP("dirs", getDirs);

    Object::getAllProps(a_props);
}

/*
ObjectStringSptr ObjectDir::toStringSptr()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorIt      it;
    ObjectStringSptr    ret;
    ObjectStringSptr    full_path;
    ObjectStringSptr    file_name;

    PTIMING(0.0f, "ObjectDir::toStringSptr 1\n");
    PTIMING(0.0f, "ObjectDir::toStringSptr 2\n");
    PTIMING(0.0f, "ObjectDir::toStringSptr 3\n");

    {
        PTIMING(0.0f, "ObjectDir::toStringSptr 4\n");
        full_path = getDir();
    }

    {
        PTIMING(0.0f, "ObjectDir::toStringSptr 5\n");
        file_name   = getFileName();
    }

    {
        PTIMING(0.0f, "init prop string\n");
        PROP_STRING(ret);
    }

    if (full_path){
        PTIMING(0.0f, "add full_path\n");
        ret->add(full_path);
    }

    if (file_name){
        PTIMING(0.0f, "add file_name\n");
        ret->add(file_name);
    }

    return ret;
}
*/

string ObjectDir::serialize(
    const string &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->serialize(a_delim);
}

/*
 * directory must end on '/'
 */
void ObjectDir::setPath(
    const char *a_path)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    dirs;
    ObjectStringSptr    file_name;
    ObjectInt32Sptr     is_absolute_path;
    int32_t             path_len    = 0;
    uint8_t             char_first  = 0;
    uint8_t             char_last   = 0;

    if (!a_path){
        PFATAL("missing argument: 'a_path'\n");
    }

    path_len = strlen(a_path);
    if (!path_len){
        PFATAL("missing argument: 'a_path'\n");
    }

    char_first = a_path[0];
    char_last  = a_path[path_len - 1];

    PROP_VECTOR(dirs, a_path, '/');
    // TODO FIXME XXX use bool instead int32 here
    PROP_INT32(is_absolute_path, ('/' == char_first) ? 1 : 0);

    if ('/' != char_last){
        // it's not directory, so get file name
        PROP_STRING(file_name);
        file_name = dynamic_pointer_cast<ObjectString>(
            dirs->pop_back()
        );
        m_val->add("file_name", file_name);
    }

    m_val->add("dirs",             dirs);
    m_val->add("is_absolute_path", is_absolute_path);

    setDirty(1);
}

bool ObjectDir::isAbsolutePath()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectInt32Sptr res;
    bool            ret = false;

    res = dynamic_pointer_cast<ObjectInt32>(
        m_val->get("is_absolute_path")
    );

    if (res){
        ret = res->getVal() ? true : false;
    }

    return ret;
}

bool ObjectDir::isDirectory()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    dirs        = getDirs();
    ObjectStringSptr    file_name   = getFileName();
    bool                ret         = false;

    if (    dirs
        &&  !dirs->empty()
        &&  (!file_name || file_name->empty()))
    {
        ret = true;
    }

    return ret;
}

bool ObjectDir::isFile()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectStringSptr    file_name = getFileName();
    bool                ret       = false;

    if (    file_name
        &&  !file_name->empty())
    {
        ret = true;
    }

    return ret;
}

/*
ObjectVectorSptr ObjectDir::getDirs()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    dirs;

    dirs = dynamic_pointer_cast<ObjectVector>(
        m_val->get("dirs")
    );

    return dirs;
}
*/

ObjectStringSptr ObjectDir::toString()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorIt      it;
    ObjectStringSptr    full_path;
    ObjectVectorSptr    dirs;

    dirs = getDirs();

    if (isAbsolutePath()){
        PROP_STRING(full_path, "/");
    } else {
        PROP_STRING(full_path);
    }

    if (dirs){
        lock_guard<recursive_mutex> guard(dirs->m_mutex);

        for (it = dirs->begin();
            it != dirs->end();
            it++)
        {
            ObjectStringSptr prop_part;
            prop_part = dynamic_pointer_cast<ObjectString>(*it);
            full_path->add(prop_part);
            full_path->add("/");
        }
    }

    return full_path;
}

void ObjectDir::addPath(
    ObjectDirSptr  a_path)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    dirs;
    ObjectVectorSptr    add_dirs;

    if (!a_path){
        PFATAL("missing argument: 'a_path'\n");
    }

    if (!a_path->isDirectory()){
        PFATAL("argument: 'a_path' is not directory,"
            " dump below:\n"
            "%s\n",
            a_path->serialize().c_str()
        );
    }

    // get current dirs
    dirs = getDirs();

    add_dirs = a_path->getDirs();
    if (add_dirs){
        lock(dirs->m_mutex, add_dirs->m_mutex);
        lock_guard<recursive_mutex> guard1(
            dirs->m_mutex,
            adopt_lock
        );
        lock_guard<recursive_mutex> guard2(
            add_dirs->m_mutex,
            adopt_lock
        );

        ObjectVectorIt add_dirs_it;

        for (add_dirs_it = add_dirs->begin();
            add_dirs_it != add_dirs->end();
            add_dirs_it++)
        {
            ObjectStringSptr add_part;
            add_part = dynamic_pointer_cast<ObjectString>(
                *add_dirs_it
            );
            dirs->add(add_part);
        }
    } else {
        PFATAL("empty path:\n"
            "%s\n",
            a_path->serialize().c_str()
        );
    }
}

void ObjectDir::addDir(
    const char *a_dir)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t i, len = 0;

    if (!a_dir){
        PFATAL("missing argument: 'a_dir'\n");
    }

    PTIMING(0.0f, "add dir: '%s'\n", a_dir);

    len = strlen(a_dir);
    if (!len){
        PFATAL("empty argument: 'a_dir'\n");
    }

    // check on wrong symbols
    for (i = 0; i < len; i++){
        if ('/' == a_dir[i]){
            PFATAL("directory name cannot contain: '/'"
                " symbol ('%s')\n",
                a_dir
            );
        }
    }

    // add
    {
        ObjectStringSptr    new_dir;
        ObjectVectorSptr    dirs = getDirs();

        PROP_STRING(new_dir, a_dir);

        lock_guard<recursive_mutex> guard(dirs->m_mutex);

        dirs->add(new_dir);
    }
}

void ObjectDir::addDir(
    ObjectStringSptr a_dir)
{
    if (!a_dir){
        PFATAL("missing argument: 'a_dir'\n");
    }

    addDir(a_dir->c_str());
}

void ObjectDir::addDir(
    const string &a_dir)
{
    if (a_dir.empty()){
        PFATAL("missing argument: 'a_dir'\n");
    }

    addDir(a_dir.c_str());
}

void ObjectDir::addPath(
    ObjectStringSptr    a_path)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDirSptr  path;

    PROP_PATH_UNIX(path, a_path);

    if (!path->isDirectory()){
        PFATAL("argument: '%s' is not directory\n",
            a_path->c_str()
        );
    }

    addPath(path);
}

int32_t ObjectDir::mkdir(
    const char          *a_sub_path,
    const int32_t       &a_recursive,
    const int32_t       &a_mode)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t             err = -1;
    ObjectDirSptr       full_path;
    ObjectStringSptr    full_path_str;
    ObjectStringSptr    cur_dir;
    ObjectVectorIt      part_it;

    cur_dir = toString();

    if (cur_dir->empty()){
        PFATAL("empty dir\n");
    }

    PTIMING(0.0f, "mkdir: '%s%s'\n",
        cur_dir->c_str(),
        a_sub_path ? a_sub_path->c_str() : ""
    );

    // create full path
    PROP_DIR(full_path, cur_dir);
    if (a_sub_path){
        full_path->addPath(a_sub_path);
    }
    if (full_path->isExist()){
        err = 0;
        goto out;
    }

    full_path_str = full_path->toString();

    PDEBUG(90, "mkdir:\n"
        "  a_sub_path:              '%s'\n"
        "  a_recursive:             '%d'\n"
        "  a_mode:                  '%d'\n"
        "  cur_dir:                 '%s'\n"
        "  will be created dir:     '%s'\n",
        a_sub_path ? a_sub_path : "",
        a_recursive,
        a_mode,
        cur_dir->c_str(),
        full_path_str->c_str()
    );

    if (a_recursive){
        // create recursive
        string              full_path_tmp;
        ObjectVectorSptr    dirs;

        if (full_path->isAbsolutePath()){
            full_path_tmp = "/";
        }

        lock_guard<recursive_mutex> guard(dirs->m_mutex);

        PDEBUG(100, "create dirs for: '%s'\n",
            full_path_str->c_str()
        );

        for (part_it = dirs->begin();
            part_it != dirs->end();
            part_it++)
        {
            ObjectStringSptr    prop_part;

            prop_part = dynamic_pointer_cast<ObjectString>(
                *part_it
            );

            PTIMING(0.0f, "mkdir: '%s/', a_mode: '%d'\n",
                prop_part->c_str(),
                a_mode
            );

            if (prop_part){
                tmp_full_path += prop_part->c_str();
                tmp_full_path += "/";
            } else {
                PFATAL("empty prop_part\n");
            }

            // may be already exist?
            if (ObjectDir::s_isExistDir(tmp_full_path.c_str())){
                PDEBUG(100, "create dir: '%s'"
                    " (skip, because exist)\n",
                    tmp_full_path.c_str()
                );
                continue;
            }

            // create new
            PDEBUG(100, "create dir: '%s'\n",
                tmp_full_path.c_str()
            );

            {
                PTIMING(0.000f, "mkdir low level\n");
                res = ::mkdir(
                    tmp_full_path.c_str(),
                    mode_t(a_mode)
                );
            }
            if (    -1      == res
                &&  EEXIST  != errno)
            {
                PERROR("cannot create dir: '%s', info:\n"
                    " res:          '%d'\n"
                    " mode:         '%d'\n"
                    " full_path:    '%s'\n"
                    " cur_dir:      '%s'\n",
                    full_path_tmp.c_str(),
                    res,
                    a_mode,
                    full_path_str->c_str(),
                    cur_dir->c_str()
                );
                err = res;
                goto fail;
            }
       }
    } else {
        // not recursive
        PDEBUG(100, "create dir: '%s'\n",
            full_path_str->c_str()
        );

        // create new dir if not exist
        res = ::mkdir(
            tmp_full_path_str->c_str(),
            mode_t(a_mode)
        );
        if (    -1      == res
            &&  EEXIST  != errno)
        {
            PERROR("cannot create dir: '%s', info:\n"
                " res:         '%d'\n"
                " mode:        '%d'\n"
                " full_path:   '%s'\n"
                " cur_path:    '%s'\n",
                tmp_full_path_str->c_str(),
                res,
                a_mode,
                full_path->serialize().c_str(),
                serialize().c_str()
            );
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

bool ObjectDir::isExistDir()
{
    bool                ret = false;
    ObjectStringSptr    dir = getDir();

    if (dir->empty()){
        PFATAL("unknwon dir, dump below:\n"
            "%s\n",
            serialize().c_str()
        );
    }

    PTIMING(0.000f, "check what dir: '%s' exist\n",
        dir->c_str()
    );

    ret = ObjectDir::s_isExistDir(dir->c_str());

    return ret;
}

bool ObjectDir::isExistFile()
{
    int32_t             res;
    bool                ret         = false;
    ObjectStringSptr    file_name   = getFileName();
    struct stat         st;

    if (file_name->empty()){
        PFATAL("unknwon file name, dump below:\n"
            "%s\n",
            serialize().c_str()
        );
    }

    //memset(&st, 0x00, sizeof(st));
    res = ::stat(file_name->c_str(), &st);
    if (    !res
        &&  S_ISREG(st.st_mode))
    {
        ret = true;
    }

    return ret;
}

bool ObjectDir::isExist()
{
    int32_t             res;
    bool                ret  = false;
    ObjectStringSptr    path = toStringSptr();
    struct stat         st;

    if (path->empty()){
        PFATAL("unknwon path, dump below:\n"
            "%s\n",
            serialize().c_str()
        );
    }

    //memset(&st, 0x00, sizeof(st));
    res = ::stat(path->c_str(), &st);
    if (!res){
        ret = true;
    }

    return ret;
}

FILE * ObjectDir::fopen(
    const char *a_fname,
    const char *a_mode)
{
    FILE    *file = NULL;
    string  full_path;

    if (    !a_fname
        ||  !strlen(a_fname))
    {
        PFATAL("missing argument: 'a_fname'\n");
    }

    if (    !a_mode
        ||  !strlen(a_mode))
    {
        PFATAL("missing argument: 'a_mode'\n");
    }

    full_path = getDir()->getVal();
    full_path.append(a_fname);

    file = ::fopen(full_path.c_str(), a_mode);

    return file;
}

int32_t ObjectDir::unlink(
    const char *a_fname)
{
    int32_t err = -1;
    string  full_path;

    if (    !a_fname
        ||  !strlen(a_fname))
    {
        PFATAL("missing argument: 'a_fname'\n");
    }

    full_path = getDir()->getVal();
    full_path.append(a_fname);

    err = ::unlink(full_path.c_str());

    return err;
}

ObjectStringSptr ObjectDir::read(
    const char *a_fname)
{
    char                buffer[65535]   = { 0x00 };
    FILE                *file           = NULL;
    size_t              res;
    ObjectStringSptr    ret;

    if (    !a_fname
        ||  !strlen(a_fname))
    {
        PFATAL("missing argument: 'a_fname'\n");
    }

    file = fopen(a_fname, "r");
    if (!file){
        goto out;
    }

    PROP_STRING(ret);

    do {
        res = fread(buffer, 1, sizeof(buffer), file);
        if (0 < res){
            ret->add(buffer, res);
        }
    } while (0 < res);

    ret->isNew(0);

    fclose(file);

out:
    return ret;
}

// ---------------- static ----------------

bool ObjectDir::s_isExistDir(
    const char *a_dir)
{
    int32_t     res;
    bool        ret = false;
    struct stat st;

    if (    !a_dir
        ||  !strlen(a_dir))
    {
        PFATAL("missing argument: 'a_dir'\n");
    }

    PTIMING(0.000f, "check what dir: '%s' exist\n",
        a_dir
    );

    res = ::stat(a_dir, &st);
    if (    !res
        &&  S_ISDIR(st.st_mode))
    {
        ret = true;
    }

    return ret;
}

// ---------------- module ----------------

string ObjectDir::_getType()
{
    return "core.object.path.unix";
}

string ObjectDir::_getTypePerl()
{
    return "core::object::pathUnix::main";
}

int32_t ObjectDir::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectDir::_destroy()
{
}

ObjectSptr ObjectDir::_object_create()
{
    ObjectSptr object;
    _OBJECT_PATH_UNIX(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectDir::_getType,
    .type_perl      = ObjectDir::_getTypePerl,
    .init           = ObjectDir::_init,
    .destroy        = ObjectDir::_destroy,
    .object_create  = ObjectDir::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

