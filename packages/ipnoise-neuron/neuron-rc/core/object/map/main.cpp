#include <dirent.h>

#include <ipnoise-common/log_common.h>
#include <core/object/object/main.hpp>
#include <core/object/dir/main.hpp>
#include <core/object/double/main.hpp>
#include <core/object/int32/main.hpp>
#include <core/object/string/main.hpp>

#include "main.hpp"
#include <core/object/unique/main.hpp>

// ---------------- ObjectMapIt ----------------

ObjectMapIterator::ObjectMapIterator()
{
}

ObjectMapIterator::~ObjectMapIterator()
{
}

// !=
bool ObjectMapIterator::operator != (
    const ObjectMapIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool ObjectMapIterator::operator != (
    const ObjectMapConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

// ==
bool ObjectMapIterator::operator == (
    const ObjectMapIterator &a_right) const
{
    return m_it == a_right.m_it;
}

bool ObjectMapIterator::operator == (
    const ObjectMapConstIterator &a_right) const
{
    return m_it == a_right.m_it;
}

// =
ObjectMapIterator & ObjectMapIterator::operator = (
    const ObjectMapIterator &a_it)
{
    m_it = a_it.m_it;
    return *this;
};

// ++
void ObjectMapIterator::operator ++ (int32_t)
{
    m_it++;
}

// ->
ObjectMapValsIt ObjectMapIterator::operator -> () const
{
    return m_it;
}

// ---------------- ObjectMapConstIt ----------------

ObjectMapConstIterator::ObjectMapConstIterator()
{
}

ObjectMapConstIterator::~ObjectMapConstIterator()
{
}

bool ObjectMapConstIterator::operator != (
    const ObjectMapConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool ObjectMapConstIterator::operator != (
    const ObjectMapIterator &a_right) const
{
    return m_it != a_right.m_it;
}

// =
ObjectMapConstIterator & ObjectMapConstIterator::operator = (
    const ObjectMapConstIterator &a_it)
{
    m_it = a_it.m_it;
    return *this;
};

void ObjectMapConstIterator::operator ++ (int32_t)
{
    m_it++;
}

// ->
ObjectMapValsConstIt ObjectMapConstIterator::operator -> () const
{
    return m_it;
}

// ---------------- ObjectMap ----------------

ObjectMap::ObjectMap(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectMap::~ObjectMap()
{
}

string ObjectMap::getType()
{
    return ObjectMap::s_getType();
}

int32_t ObjectMap::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_prop(a_data, a_data_size);

    return err;
}

int32_t ObjectMap::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_object(a_data, a_data_size);

    return err;
}

ObjectSptr ObjectMap::copy()
{
    ObjectMapSptr   ret;
    ObjectMapValsIt vals_it;

    ret = OBJECT<ObjectMap>();

    for (vals_it = m_vals.begin();
        vals_it != m_vals.end();
        vals_it++)
    {
        ObjectSptr key = vals_it->first;
        ObjectSptr val = vals_it->second;

        ret->add(key->copy(), val->copy());
    }

    return ret;
}

void ObjectMap::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (flags & CREATE_OBJECT){
        Object::getAllProps(a_out);
    }
}

void ObjectMap::add(
    ObjectSptr a_key,
    ObjectSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_vals[a_key] = a_val;
    setDirty(1);
}

void ObjectMap::add(
    const string &a_key,
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr prop_key;
    ObjectStringSptr prop_val;

    prop_key = OBJECT<ObjectString>(a_key);
    prop_val = OBJECT<ObjectString>(a_val);

    m_vals[prop_key] = prop_val;
    setDirty(1);
}

void ObjectMap::add(
    const string    &a_key,
    ObjectSptr      a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr prop_key;

    prop_key = OBJECT<ObjectString>(a_key);

    m_vals[prop_key] = a_val;
    setDirty(1);
}

void ObjectMap::add(
    const char  *a_key,
    ObjectSptr  a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr prop_key;

    prop_key = OBJECT<ObjectString>(a_key);

    m_vals[prop_key] = a_val;
    setDirty(1);
}

void ObjectMap::add(
    ObjectSptr      a_key,
    const string    &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr prop_val;

    prop_val = OBJECT<ObjectString>(a_val);

    m_vals[a_key] = prop_val;
    setDirty(1);
}

void ObjectMap::add(
    const string    &a_key,
    const int32_t   &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr prop_key;
    ObjectInt32Sptr  prop_val;

    prop_key = OBJECT<ObjectString>(a_key);
    prop_val = OBJECT<ObjectInt32>(a_val);

    m_vals[prop_key] = prop_val;
    setDirty(1);
}

void ObjectMap::add(
    ObjectSptr      a_key,
    const int32_t   &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectInt32Sptr  prop_val;

    prop_val = OBJECT<ObjectInt32>(a_val);

    m_vals[a_key] = prop_val;
    setDirty(1);
}

void ObjectMap::add(
    const int32_t   &a_key,
    ObjectSptr      a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectInt32Sptr  prop_key;

    prop_key = OBJECT<ObjectInt32>(a_key);

    m_vals[prop_key] = a_val;
    setDirty(1);
}

void ObjectMap::add(
    const string    &a_key,
    const double    &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr prop_key;
    ObjectDoubleSptr prop_val;

    prop_key = OBJECT<ObjectString>(a_key);
    prop_val = OBJECT<ObjectDouble>(a_val);

    m_vals[prop_key] = prop_val;
    setDirty(1);
}

void ObjectMap::add(
    ObjectSptr      a_key,
    const double    &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectDoubleSptr  prop_val;

    prop_val = OBJECT<ObjectDouble>(a_val);

    m_vals[a_key] = prop_val;
    setDirty(1);
}

bool ObjectMap::has(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr prop_key;

    prop_key = OBJECT<ObjectString>(a_key);

    return has(prop_key);
}

bool ObjectMap::has(
    const ObjectSptr &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    bool                    ret = false;
    ObjectMapValsConstIt    it;

    it = m_vals.find(a_key);
    if (m_vals.end() != it){
        ret = true;
    }
    return ret;
}

uint32_t ObjectMap::size()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_vals.size();
}

ObjectStringSptr ObjectMap::toString()
{
    return serializeAsProp();
}

ObjectStringSptr ObjectMap::serializeAsProp(
    const char *a_delim)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr        ret;
    ObjectMapValsConstIt    it;

    ret = OBJECT<ObjectString>();

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        ObjectSptr    key_object    = it->first;
        ObjectSptr    val_object    = it->second;

        if (!ret->empty()){
            ret->add(", ");
        }

        ret->add(
            key_object->serializeAsProp(a_delim)
        );
        ret->add(a_delim);
        ret->add(
            val_object->serializeAsProp(a_delim)
        );
    }

    {
        ObjectUniqueSptr flags;
        flags = OBJECT<ObjectUnique>();
        flags->add("add_to_begin");
        ret->add("{ ", flags);
    }

    {
        ObjectUniqueSptr flags;
        flags = OBJECT<ObjectUnique>();
        flags->add("add_to_end");
        ret->add(" }", flags);
    };

    return ret;
}

int32_t ObjectMap::loadAsProp(
    const char  *a_dir)
{
//    int32_t         res, err    = 0;
    int32_t         err         = 0;
    DIR             *dirp       = NULL;
    struct dirent   *dir_ent    = NULL;
    string          dir         = getObjectPath();
    string          vals_path;
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

    // load vals
    vals_path = dir;
    vals_path += OBJECT_DIR_VALS;

    dirp = opendir(vals_path.c_str());
    if (!dirp){
        goto out;
    }

    do {
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

        val = g_engine_info->objectLoad(val_path.c_str());

        add(dir_ent->d_name, val);
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

int32_t ObjectMap::saveAsProp(
    const char  *a_dir)
{
    int32_t         res, ret    = -1;
    string          dir         = getObjectPath();
    ObjectMapValsIt vals_it;
    string          vals_path;

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

    PDEBUG(10, "attempt to save object to dir: '%s'\n",
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

    // recreate vals dir
    res = ObjectDir::s_recreate_dir(vals_path.c_str());
    if (res){
        PERROR("cannot recreate dir: '%s', res: '%d'\n",
            vals_path.c_str(),
            res
        );
    }

    for (vals_it = m_vals.begin();
        vals_it != m_vals.end();
        vals_it++)
    {
        ObjectSptr key     = vals_it->first;
        ObjectSptr val     = vals_it->second;
        string     val_dir;

        val_dir = vals_path + key->toStringStd() + "/";
        val->save(val_dir.c_str());
    }

    // all ok
    ret = 0;

out:
    return ret;
fail:
    goto out;
}

// ---------------- begin ----------------

ObjectMap::iterator ObjectMap::begin() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMap::iterator it;
    it.m_it = m_vals.begin();
    return it;
}

// ---------------- cbegin ----------------

ObjectMap::const_iterator ObjectMap::cbegin() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMap::const_iterator it;
    it.m_it = m_vals.cbegin();
    return it;
}

// ---------------- end ----------------

ObjectMap::iterator ObjectMap::end() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMap::iterator it;
    it.m_it = m_vals.end();
    return it;
}

// ---------------- cend ----------------

ObjectMap::const_iterator ObjectMap::cend() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMap::const_iterator it;
    it.m_it = m_vals.cend();
    return it;
}

// ---------------- find ----------------

ObjectMap::iterator ObjectMap::find(
    const ObjectSptr &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMap::iterator it;
    it.m_it = m_vals.find(a_key);
    return it;
}

ObjectMap::iterator ObjectMap::find(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMap::iterator it;
    ObjectStringSptr    prop_key;

    prop_key = OBJECT<ObjectString>(a_key);
    it.m_it  = m_vals.find(prop_key);

    return it;
}

ObjectMap::iterator ObjectMap::find(
    const int32_t &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMap::iterator it;
    ObjectInt32Sptr     prop_key;

    prop_key = OBJECT<ObjectInt32>(a_key);
    it.m_it  = m_vals.find(prop_key);

    return it;
}

// ---------------- erase ----------------

void ObjectMap::erase(
    const ObjectMap::iterator &a_it)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_vals.erase(a_it.m_it);
    setDirty(1);
}

void ObjectMap::erase(
    ObjectSptr a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMap::iterator it = find(a_key);
    if (end() != it){
        erase(it);
    }
}

void ObjectMap::erase(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr prop_key;

    prop_key = OBJECT<ObjectString>(a_key);

    erase(prop_key);
    setDirty(1);
}

void ObjectMap::clear()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMapVals empty;
    m_vals = empty;

    setDirty(1);
}

ObjectSptr ObjectMap::get(
    ObjectSptr a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr ret;

    if (has(a_key)){
        ret = (*this)[a_key];
    }

    return ret;
}

ObjectSptr ObjectMap::get(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr          ret;
    ObjectStringSptr    prop_key;

    prop_key = OBJECT<ObjectString>(a_key);

    if (has(prop_key)){
        ret = (*this)[prop_key];
    }

    return ret;
}

bool ObjectMap::empty()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_vals.empty();
}

// for assign only!
// Example:
// ObjectMapSptr        map;
// ObjectStringSptr     val;
// map["key"] = val;
//
// method will create empty pair!
// Example:
// ObjectSptr val = (*(map.get())["key"];
// will create key "key" with empty val ObjectSptr()
//
ObjectSptr & ObjectMap::operator [] (
    const ObjectSptr &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMapValsIt it;

    // create new if not exist
    it = m_vals.find(a_key);
    if (m_vals.end() == it){
        m_vals[a_key] = ObjectSptr();
    }

    setDirty(1);

    // search again
    it = m_vals.find(a_key);
    return it->second;
}

ObjectSptr & ObjectMap::operator [] (
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr prop_key;

    prop_key = OBJECT<ObjectString>(a_key);
    prop_key->setDirty(0);

    return (*this)[prop_key];
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectMap::s_getType()
{
    return "core.object.map";
}

int32_t ObjectMap::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectMap::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectMap::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectMap(a_create_flags));

    return object;
}

void ObjectMap::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_map = {
    .getType        = ObjectMap::s_getType,
    .init           = ObjectMap::s_init,
    .shutdown       = ObjectMap::s_shutdown,
    .objectCreate   = ObjectMap::s_objectCreate,
    .getTests       = ObjectMap::s_getTests
};

