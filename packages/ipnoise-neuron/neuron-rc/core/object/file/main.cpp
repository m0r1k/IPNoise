#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/dir/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>

#include "main.hpp"

ObjectFile::ObjectFile(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectFile::~ObjectFile()
{
}

string ObjectFile::getType()
{
    return ObjectFile::s_getType();
}

int32_t ObjectFile::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = 0;

    res = Object::do_init_as_prop(a_data, a_data_size);
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

int32_t ObjectFile::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = -1;

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

ObjectSptr ObjectFile::copy()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectFileSptr  ret;

    ret = OBJECT<ObjectFile>();

    ret->setDir(dynamic_pointer_cast<ObjectDir>(
        m_dir->copy()
    ));
    // setFileName will recreate m_file_name_esc
    ret->setFileName(m_file_name->c_str());
    ret->setFileContent(dynamic_pointer_cast<ObjectString>(
        m_file_content->copy()
    ));

    return ret;
}

void ObjectFile::getAllProps(
    ObjectMapSptr   a_out)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        a_out->add("file_dir",      m_dir);
        a_out->add("file_name",     m_file_name);
        a_out->add("file_name_esc", m_file_name_esc);
        a_out->add("file_content",  m_file_content);
        Object::getAllProps(a_out);
    }
}

void ObjectFile::setDir(
    const char *a_dir)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    if (    !a_dir
        ||  !strlen(a_dir))
    {
        PFATAL("missing or empty argument: 'a_dir'\n");
    }

    m_dir = OBJECT<ObjectDir>(a_dir);

    setDirty(1);
}

ObjectDirSptr ObjectFile::getDir()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_dir;
}

void ObjectFile::setDir(
    ObjectDirSptr a_dir)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_dir = a_dir;
}

void ObjectFile::setFileName(
    const char *a_name)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    if (    !a_name
        ||  !strlen(a_name))
    {
        PFATAL("missing or empty argument: 'a_name'\n");
    }

    m_file_name     = OBJECT<ObjectString>(a_name);
    m_file_name_esc = ObjectDir::s_escape_file_name(a_name);

    setDirty(1);
}

ObjectStringSptr ObjectFile::getFileName()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_file_name;
}

ObjectStringSptr ObjectFile::getFileNameEsc()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_file_name_esc;
}

void ObjectFile::setFileContent(
    ObjectStringSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_file_content = a_val;

    setDirty(1);
}

ObjectStringSptr ObjectFile::getFileContent()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_file_content;
}

int32_t ObjectFile::touch()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    int32_t err = -1;

    if (!m_dir){
        PFATAL("unknown file dir\n");
    }

    if (!m_file_name_esc){
        PFATAL("unknown escaped file name\n");
    }

    err = m_dir->file_touch(
        m_file_name->c_str()
    );

    return err;
}

ObjectStringSptr ObjectFile::load()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    char                buffer[65535] = { 0x00 };
    FILE                *file = NULL;
    string              mode  = "r";
    int32_t             res;

    file = m_dir->file_open(
        m_file_name->c_str(),
        mode.c_str()
    );
    if (!file){
        PWARN("cannot open file: '%s'"
            " from dir: '%s' for '%s'\n",
            m_file_name->c_str(),
            m_dir->toString()->c_str(),
            mode.c_str()
        );
        goto fail;
    }

    m_file_content = OBJECT<ObjectString>();

    do {
        res = fread(buffer, 1, sizeof(buffer), file);
        if (0 < res){
            m_file_content->add(buffer, res);
        }
    } while (0 < res);

    fclose(file);

out:
    return m_file_content;
fail:
    goto out;
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectFile::s_getType()
{
    return "core.object.file";
}

int32_t ObjectFile::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectFile::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectFile::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectFile(a_create_flags));

    return object;
}

void ObjectFile::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_file = {
    .getType        = ObjectFile::s_getType,
    .init           = ObjectFile::s_init,
    .shutdown       = ObjectFile::s_shutdown,
    .objectCreate   = ObjectFile::s_objectCreate,
    .getTests       = ObjectFile::s_getTests
};

