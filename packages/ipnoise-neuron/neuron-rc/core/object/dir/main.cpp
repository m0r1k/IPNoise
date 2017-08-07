#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>
#include <core/object/vector/main.hpp>
#include <core/object/unique/main.hpp>

#include "main.hpp"

ObjectDir::ObjectDir(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectDir::~ObjectDir()
{
}

string ObjectDir::getType()
{
    return ObjectDir::s_getType();
}

int32_t ObjectDir::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_prop(a_data, a_data_size);

    m_val = OBJECT<ObjectString>(a_data, a_data_size);
    if (a_data){
        ObjectUniqueSptr flags;
        flags = OBJECT<ObjectUnique>();
        flags->add("add_to_end");
        flags->add("add_if_not_exist");
        m_val->add("/", flags);
        setDirty(1);
    }

    return err;
}

int32_t ObjectDir::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = 0;

    res = Object::do_init_as_object(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    m_val = OBJECT<ObjectString>(
        a_data,
        a_data_size,
        CREATE_PROP
    );
    if (a_data){
        ObjectUniqueSptr flags;
        flags = OBJECT<ObjectUnique>();
        flags->add("add_to_end");
        flags->add("add_if_not_exist");
        m_val->add("/", flags);
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

ObjectSptr ObjectDir::copy()
{
    ObjectDirSptr   ret;

    ret = OBJECT<ObjectDir>(m_val->c_str());

    return ret;
}

void ObjectDir::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();
    if (!(flags & CREATE_PROP)){
        a_out->add("dir_cur", m_val);
        Object::getAllProps(a_out);
    }
}

ObjectStringSptr ObjectDir::toString()
{
    return serializeAsProp();
}

ObjectStringSptr ObjectDir::serializeAsProp(
    const char  *a_delim)
{
    return m_val->serializeAsProp(a_delim);
}

int32_t ObjectDir::file_touch(
    const char *a_file_name)
{
    int32_t             err     = -1;
    FILE                *file   = NULL;
    string              mode    = "w+";

    ObjectStringSptr    full_path;
    ObjectStringSptr    file_name_esc;

    if (    !a_file_name
        ||  !strlen(a_file_name))
    {
        PFATAL("missing or empty argument: 'a_file_name'\n");
    }

    full_path     = OBJECT<ObjectString>(m_val->c_str());
    file_name_esc = ObjectDir::s_escape_file_name(a_file_name);
    full_path->add(file_name_esc);

    file = fopen(full_path->c_str(), mode.c_str());
    if (!file){
        PDEBUG(100, "cannot open: '%s' for: '%s'\n",
            full_path->c_str(),
            mode.c_str()
        );
        goto fail;
    }

    fclose(file);

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

FILE * ObjectDir::file_open(
    const char *a_fname,
    const char *a_mode)
{
    string              dir;
    string              full_path;
    ObjectStringSptr    file_name_esc;
    FILE                *ret = NULL;

    if (!a_fname){
        PFATAL("missing argument: 'a_fname'\n");
    }

    dir           = m_val->getVal();
    file_name_esc = ObjectDir::s_escape_file_name(a_fname);
    full_path     = dir + file_name_esc->toStringStd();

    ret = fopen(full_path.c_str(), a_mode);
    if (!ret){
        PWARN("cannot open file: '%s' for mode: '%s'\n",
            full_path.c_str(),
            a_mode
        );
    }

    return ret;
}

// ---------------- static ----------------

ObjectStringSptr ObjectDir::s_escape_file_name(
    const char *a_path)
{
    ObjectStringSptr    ret;
    uint64_t            i = 0;
    char                c = 0;

    if (    !a_path
        ||  !strlen(a_path))
    {
        PFATAL("missing or empty argument: 'a_path'");
    }

    ret = OBJECT<ObjectString>();

    do {
        c = a_path[i++];
        if ('\0' == c){
            break;
        }

        if ('/' == c){
            ret->add("%2f");
        } else {
            ret->add(c);
        }

    } while (c);

    return ret;
}

int32_t ObjectDir::s_mkdir(
    const char      *a_path,
    const int32_t   &a_recursive)
{
    int32_t err = -1;

    err = ::s_mkdir(a_path, a_recursive);

    return err;
}

int32_t ObjectDir::s_mkdir(
    ObjectVectorSptr    a_dirs,
    const int32_t       &a_recursive)
{
    int32_t         res, err = -1;
    ObjectVectorIt  dirs_it;

    for (dirs_it = a_dirs->begin();
        dirs_it != a_dirs->end();
        dirs_it++)
    {
        ObjectStringSptr    full_path;

        full_path = dynamic_pointer_cast<ObjectString>(
            *dirs_it
        );

        res = ObjectDir::s_mkdir(
            full_path->c_str(),
            a_recursive
        );
        if (res){
            PERROR("cannot crate dir: '%s'\n",
                full_path->c_str()
            );
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

int32_t ObjectDir::s_rm(
    const char      *a_path,
    const int32_t   &a_recursive,
    const int32_t   &a_remove_top_dir)
{
    int32_t err = -1;

    err = ::s_rm(
        a_path,
        a_recursive,
        a_remove_top_dir
    );

    return err;
}

int32_t ObjectDir::s_recreate_dir(
    const char  *a_path)
{
    int32_t err = -1;

    err = ::s_recreate_dir(a_path);

    return err;
}

// ---------------- tests ----------------

int32_t ObjectDir::s_test_mkdir(
    const char *a_tmp_path)
{
    int32_t err = -1;
    string  path;

    path = a_tmp_path;
    path += "1/2/3/4/5";

    err = ObjectDir::s_mkdir(path.c_str(), 0);
    PDEBUG(15, "after s_mkdir: '%s', err: '%d'\n",
        path.c_str(),
        err
    );

    return err;
}

int32_t ObjectDir::s_test_mkdirr(
    const char *a_tmp_path)
{
    int32_t err = -1;
    string  path;

    path = a_tmp_path;
    path += "1/2/3/4/5";

    err = ObjectDir::s_mkdir(path.c_str(), 1);
    PDEBUG(15, "after s_mkdir: '%s', err: '%d'\n",
        path.c_str(),
        err
    );

    return err;
}

int32_t ObjectDir::s_test_rm(
    const char *a_tmp_path)
{
    int32_t err = -1;
    string  path;

    path = a_tmp_path;
    path += "1";

    err = ObjectDir::s_rm(path.c_str(), 0);
    PDEBUG(15, "after s_rm: '%s', res: '%d'\n",
        path.c_str(),
        err
    );

    return err;
}

int32_t ObjectDir::s_test_rmr(
    const char *a_tmp_path)
{
    int32_t err = -1;
    string  path;

    path = a_tmp_path;
    path += "1";

    err = ObjectDir::s_rm(path.c_str(), 1);
    PDEBUG(15, "after s_rm: '%s', res: '%d'\n",
        path.c_str(),
        err
    );

    return err;
}

// ---------------- module ----------------

string ObjectDir::s_getType()
{
    return "core.object.dir";
}

int32_t ObjectDir::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectDir::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectDir::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectDir(a_create_flags));

    return object;
}

void ObjectDir::s_getTests(
    Tests &a_out)
{
    TestInfo    test;

    test = TestInfo();
    test.name         = "create dir without recursive flag";
    test.success_code = -1;
    test.run          = ObjectDir::s_test_mkdir;
    a_out.push_back(test);

    test = TestInfo();
    test.name         = "create dir with recursive flag";
    test.success_code = 0;
    test.run          = ObjectDir::s_test_mkdirr;
    a_out.push_back(test);

    test = TestInfo();
    test.name         = "rm dir without recursive flag";
    test.success_code = -1;
    test.run          = ObjectDir::s_test_rm;
    a_out.push_back(test);

    test = TestInfo();
    test.name         = "rm dir with recursive flag";
    test.success_code = 0;
    test.run          = ObjectDir::s_test_rmr;
    a_out.push_back(test);
}

ModuleInfo core_object_dir = {
    .getType        = ObjectDir::s_getType,
    .init           = ObjectDir::s_init,
    .shutdown       = ObjectDir::s_shutdown,
    .objectCreate   = ObjectDir::s_objectCreate,
    .getTests       = ObjectDir::s_getTests
};

