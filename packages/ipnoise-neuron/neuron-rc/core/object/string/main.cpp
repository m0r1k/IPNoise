#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include <algorithm>

#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/dir/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/unique/main.hpp>

#include "main.hpp"

ObjectString::ObjectString(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectString::~ObjectString()
{
}

string ObjectString::getType()
{
    return ObjectString::s_getType();
}

int32_t ObjectString::do_init_as_prop(
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
        m_val.assign(a_data, a_data_size);
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

int32_t ObjectString::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = -1;

    res = Object::do_init_as_object(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    if (a_data){
        m_val.assign(a_data, a_data_size);
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

ObjectSptr ObjectString::copy()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr    ret;

    ret = OBJECT<ObjectString>(m_val);

    return ret;
}

void ObjectString::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        Object::getAllProps(a_out);
    }
}

ObjectStringSptr ObjectString::toString()
{
    ObjectStringSptr ret;

    ret = OBJECT<ObjectString>(m_val);

    return ret;
}

ObjectStringSptr ObjectString::serializeAsProp(
    const char  *a_delim)
{
    ObjectStringSptr ret;

    ret = OBJECT<ObjectString>();
    ret->add('"');
    ret->add(m_val);
    ret->add('"');

    return ret;
}

int32_t ObjectString::loadAsProp(
    const char  *a_dir)
{
    char        buffer[65535]   = { 0x00 };
    int32_t     res, err        = 0;
    string      dir             = getObjectPath();
    FILE        *file           = NULL;
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

int32_t ObjectString::saveAsProp(
    const char  *a_dir)
{
    int32_t     ret     = -1;
    string      dir     = getObjectPath();
    FILE        *file   = NULL;
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

    PDEBUG(15, "attempt to save object to dir: '%s'\n",
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
        string  full_path;
        string  mode;

        full_path   = dir + OBJECT_FILE_VAL;
        mode        = "w";
        file        = fopen(full_path.c_str(), mode.c_str());
        if (!file){
            PERROR("cannot open file: '%s' for mode: '%s'\n",
                full_path.c_str(),
                mode.c_str()
            );
            goto fail;
        }

        res = fwrite(m_val.c_str(), 1, m_val.size(), file);
        if (res != int64_t(m_val.size())){
#ifdef __x86_64__
            PERROR("partial save file: '%s',"
                " attempt to save: '%ld' byte(s),"
                " but was wrote: '%ld' byte(s)\n",
                full_path.c_str(),
                int64_t(m_val.size()),
                int64_t(res)
            );
#else
            PERROR("partial save file: '%s',"
                " attempt to save: '%d' byte(s),"
                " but was wrote: '%d' byte(s)\n",
                full_path.c_str(),
                int32_t(m_val.size()),
                int32_t(res)
            );
#endif
            goto fail;
        }
    }

    // all ok
    ret = 0;

out:
    if (file){
        fclose(file);
    }
    return ret;
fail:
    goto out;
}

string ObjectString::toStringStd()
{
    return m_val;
}

bool ObjectString::empty()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_val.empty();
}

uint64_t ObjectString::size()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_val.size();
}

void ObjectString::assign(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_val = a_val;

    setDirty(1);
}

const char * ObjectString::c_str() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_val.c_str();
}

char ObjectString::at(
    const int32_t   &a_pos)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_val.at(a_pos);
}

void ObjectString::toLower()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    // force to be lower
    transform(
        m_val.begin(),
        m_val.end(),
        m_val.begin(),
        ::tolower
    );

    setDirty(1);
}

void ObjectString::add(
    const string        &a_val,
    ObjectUniqueSptr    a_flags)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    if (!a_flags){
        m_val += a_val;
        goto out;
    }

    if (    a_flags
        &&  a_flags->has("add_to_begin"))
    {
        if (a_flags->has("add_if_not_exist")){
            if (m_val.size() < a_val.size()){
                m_val = a_val + m_val;
                setDirty(1);
            } else if (m_val.substr(0, a_val.size()) != a_val){
                m_val = a_val + m_val;
                setDirty(1);
            } else {
                // already exist
            }
        } else {
            m_val = a_val + m_val;
            setDirty(1);
        }
    }

    if (    a_flags
        &&  a_flags->has("add_to_end"))
    {
        if (a_flags->has("add_if_not_exist")){
            if (m_val.size() < a_val.size()){
                m_val += a_val;
                setDirty(1);
            } else if (m_val.substr(m_val.size() - a_val.size()) != a_val)
            {
                m_val += a_val;
                setDirty(1);
            } else {
                // already exist
            }
        } else {
            m_val += a_val;
            setDirty(1);
        }
    }

out:
    return;
}

void ObjectString::add(
    const char          &a_val,
    ObjectUniqueSptr    a_flags)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    if (!a_flags){
        m_val += a_val;
        goto out;
    }

    if (    a_flags
        &&  a_flags->has("add_to_begin"))
    {
        if (a_flags->has("add_if_not_exist")){
            if (m_val.empty()){
                m_val = a_val + m_val;
                setDirty(1);
            } else if (a_val != m_val.at(0)){
                m_val = a_val + m_val;
                setDirty(1);
            } else {
                // already exist
            }
        } else {
            m_val = a_val + m_val;
            setDirty(1);
        }
    }

    if (    a_flags
        &&  a_flags->has("add_to_end"))
    {
        if (a_flags->has("add_if_not_exist")){
            if (m_val.empty()){
                m_val += a_val;
                setDirty(1);
            } else if (m_val.at(m_val.size() - 1) != a_val)
            {
                m_val += a_val;
                setDirty(1);
            } else {
                // already exist
            }
        } else {
            m_val += a_val;
            setDirty(1);
        }
    }

out:
    return;
}

void ObjectString::add(
    const char          *a_buff,
    const uint64_t      &a_len,
    ObjectUniqueSptr    a_flags)
{
    string tmp;

    if (!a_buff){
        PFATAL("missing argument: 'a_buff'\n");
    }

    tmp.assign(a_buff, a_len);
    add(tmp, a_flags);
}

void ObjectString::add(
    ObjectStringSptr    a_val,
    ObjectUniqueSptr    a_flags)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    string tmp;

    if (!a_val){
        PFATAL("missing argument: 'a_val'\n");
    }

    tmp = a_val->toStringStd();
    add(tmp, a_flags);
}

void ObjectString::clear()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_val.clear();

    setDirty(1);
}

ObjectStringSptr ObjectString::substr(
    const uint64_t   &a_pos,
    const uint64_t   &a_len)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr    ret;

    if (m_val.size() < a_pos){
#ifdef __x86_64__
        PFATAL("out of range,"
            " m_val.size(): '%lu',"
            " a_pos: '%lu',"
            " a_len: '%lu'\n",
            m_val.size(),
            a_pos,
            a_len
        );
#else
        PFATAL("out of range,"
            " m_val.size(): '%u',"
            " a_pos: '%llu',"
            " a_len: '%llu'\n",
            m_val.size(),
            a_pos,
            a_len
        );
#endif
    }

    if (0 < a_len){
        ret = OBJECT<ObjectString>(
            m_val.c_str() + a_pos,
            min(m_val.size() - a_pos, a_len)
        );
    }

    return ret;
}

ObjectStringSptr ObjectString::substr(
    const uint64_t   &a_pos)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr    ret;

    if (m_val.size() < a_pos){
#ifdef __x86_64__
        PFATAL("out of range,"
            " m_val.size(): '%lu',"
            " a_pos: '%lu'\n",
            m_val.size(),
            a_pos
        );
#else
        PFATAL("out of range,"
            " m_val.size(): '%u',"
            " a_pos: '%llu'\n",
            m_val.size(),
            a_pos
        );
#endif
    }
    ret = OBJECT<ObjectString>(
        m_val.c_str() + a_pos,
        m_val.size() - a_pos
    );

    return ret;
}

string ObjectString::getVal()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_val;
}

string ObjectString::hexdump()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    string ret;

    ret = Utils::dump(
        m_val.c_str(),
        m_val.size()
    );

    return ret;
}

ObjectString::operator string()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_val;
}

// ---------------- static ----------------

void ObjectString::s_split(
    const char      *a_str,
    const char      *a_delim,
    vector<string>  &a_out,
    const int32_t   &a_strict)
{
    ::s_split(
        a_str,
        a_delim,
        a_out,
        a_strict
    );
}

//
// if a_len > length of a_str, a_symbol(s) will be added,
// otherwise a_str will be truncated to a_len length
//
string ObjectString::s_align_len(
    const char      *a_str,
    const uint32_t  a_len,
    const char      a_symbol)
{
    string ret;
    ret = ::s_align_len(a_str, a_len, a_symbol);
    return ret;
}

void ObjectString::s_chomp(
    string &a_str)
{
    ::s_chomp(a_str);
}

void ObjectString::s_chomp2(
    string &a_str)
{
    ::s_chomp2(a_str);
}

void ObjectString::s_removeSpacesFromStart(
    string &a_str)
{
    ::s_removeSpacesFromStart(a_str);
}

void ObjectString::s_removeSpacesFromEnd(
    string &a_str)
{
    ::s_removeSpacesFromEnd(a_str);
}

void ObjectString::s_removeSpaces(
    string          &a_str,
    const int32_t   &a_from_start,
    const int32_t   &a_from_end)
{
    ::s_removeSpaces(
        a_str,
        a_from_start,
        a_from_end
    );
}

// ---------------- module ----------------

string ObjectString::s_getType()
{
    return "core.object.string";
}

int32_t ObjectString::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectString::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectString::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectString(a_create_flags));

    return object;
}

void ObjectString::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_string = {
    .getType        = ObjectString::s_getType,
    .init           = ObjectString::s_init,
    .shutdown       = ObjectString::s_shutdown,
    .objectCreate   = ObjectString::s_objectCreate,
    .getTests       = ObjectString::s_getTests
};

