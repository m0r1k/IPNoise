#include <algorithm>

#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/dir/main.hpp>
#include <core/object/int32/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>
#include <core/object/unique/main.hpp>

#include "main.hpp"

// ---------------- ObjectVectorIt ----------------

ObjectVectorIterator::ObjectVectorIterator()
{
}

ObjectVectorIterator::~ObjectVectorIterator()
{
}

// !=

bool ObjectVectorIterator::operator != (
    const ObjectVectorIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool ObjectVectorIterator::operator != (
    const ObjectVectorConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

// ==
bool ObjectVectorIterator::operator == (
    const ObjectVectorIterator &a_right) const
{
    return m_it == a_right.m_it;
}

bool ObjectVectorIterator::operator == (
    const ObjectVectorConstIterator &a_right) const
{
    return m_it == a_right.m_it;
}

// =
ObjectVectorIterator & ObjectVectorIterator::operator = (
    const ObjectVectorIterator &a_it)
{
    m_it = a_it.m_it;
    return *this;
};

// ++
void ObjectVectorIterator::operator ++ (int32_t)
{
    m_it++;
}

// *
ObjectSptr ObjectVectorIterator::operator * () const
{
    return *m_it;
}

// ->
ObjectVectorValsIt ObjectVectorIterator::operator -> () const
{
    return m_it;
}

// ---------------- ObjectVectorConstIt ----------------

ObjectVectorConstIterator::ObjectVectorConstIterator()
{
}

ObjectVectorConstIterator::~ObjectVectorConstIterator()
{
}

// !=
bool ObjectVectorConstIterator::operator != (
    const ObjectVectorConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool ObjectVectorConstIterator::operator != (
    const ObjectVectorIterator &a_right) const
{
    return m_it != a_right.m_it;
}

// =
ObjectVectorConstIterator & ObjectVectorConstIterator::operator = (
    const ObjectVectorConstIterator &a_it)
{
    m_it = a_it.m_it;
    return *this;
};

// ++
void ObjectVectorConstIterator::operator ++ (int32_t)
{
    m_it++;
}

// *
ObjectSptr ObjectVectorConstIterator::operator * () const
{
    return *m_it;
}

// ->
ObjectVectorValsConstIt ObjectVectorConstIterator::operator -> () const
{
    return m_it;
}

// ---------------- ObjectVector ----------------

ObjectVector::ObjectVector(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectVector::~ObjectVector()
{
}

string ObjectVector::getType()
{
    return ObjectVector::s_getType();
}

int32_t ObjectVector::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_prop(a_data, a_data_size);

    return err;
}

int32_t ObjectVector::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_object(a_data, a_data_size);

    return err;
}

ObjectSptr ObjectVector::copy()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectVectorSptr        ret;
    ObjectVectorValsConstIt it;

    ret = OBJECT<ObjectVector>();

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        ObjectSptr object = *it;
        ret->add(object->copy());
    }

    return ret;
}

void ObjectVector::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();
    if (!(flags & CREATE_PROP)){
        Object::getAllProps(a_out);
    }
}

ObjectStringSptr ObjectVector::toString()
{
    return serializeAsProp();
}

ObjectStringSptr ObjectVector::serializeAsProp(
    const char  *a_delim)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr        ret;
    ObjectVectorValsConstIt it;

    ret = OBJECT<ObjectString>();

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        ObjectSptr object = *it;
        if (!ret->empty()){
            ret->add(", ");
        }
        ret->add(
            object->serializeAsProp(a_delim)
        );
    }

    {
        ObjectUniqueSptr flags;
        flags = OBJECT<ObjectUnique>();
        flags->add("add_to_begin");
        ret->add("[ ", flags);
    }

    {
        ObjectUniqueSptr flags;
        flags = OBJECT<ObjectUnique>();
        flags->add("add_to_begin");
        ret->add(" ]", flags);
    }

    return ret;
}

int32_t ObjectVector::loadAsProp(
    const char  *a_dir)
{
    char    buffer[65535]   = { 0x00 };
    int32_t i, err          = -1;
    string  dir             = getObjectPath();
    string  vals_path;

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

    for (i = 0;;i++){
        ObjectSptr  val;

        snprintf(buffer, sizeof(buffer),
            "%s%d/",
            vals_path.c_str(),
            i
        );

        val = g_engine_info->objectLoad(buffer);
        if (!val){
            break;
        }

        add(val);
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

int32_t ObjectVector::saveAsProp(
    const char  *a_dir)
{
    char                buffer[65535]   = { 0x00 };
    int32_t             i, res, ret     = -1;
    string              dir             = getObjectPath();
    ObjectVectorValsIt  vals_it;
    string              vals_path;

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
        PERROR("cannot recrate dir: '%s'\n",
            vals_path.c_str()
        );
        goto fail;
    }

    for (i = 0, vals_it = m_vals.begin();
        vals_it != m_vals.end();
        vals_it++,
        i++)
    {
        ObjectSptr val = *vals_it;

        snprintf(buffer, sizeof(buffer),
            "%s%d/",
            vals_path.c_str(),
            i
        );

        val->save(buffer);
    }

    // all ok
    ret = 0;

out:
    return ret;
fail:
    goto out;
}

// ---------------- begin ----------------

ObjectVector::iterator ObjectVector::begin() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectVector::iterator it;
    it.m_it = m_vals.begin();
    return it;
}

// ---------------- cbegin ----------------

ObjectVector::iterator ObjectVector::cbegin() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectVector::iterator it;
    it.m_it = m_vals.begin();
    return it;
}

// ---------------- end ----------------

ObjectVector::iterator ObjectVector::end() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectVector::iterator it;
    it.m_it = m_vals.end();
    return it;
}

// ---------------- cend ----------------

ObjectVector::iterator ObjectVector::cend() noexcept
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectVector::iterator  it;
    it.m_it = m_vals.end();
    return it;
}

void ObjectVector::add(
    ObjectSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    push_back(a_val);

    setDirty(1);
}

void ObjectVector::add(
    const int32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectInt32Sptr prop_val;
    prop_val = OBJECT<ObjectInt32>(a_val);
    add(prop_val);
}

void ObjectVector::add(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectStringSptr prop_val;
    prop_val = OBJECT<ObjectString>(a_val);
    add(prop_val);
}

ObjectVector::iterator ObjectVector::erase(
    ObjectVector::iterator a_it)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectVector::iterator it;
    it.m_it = m_vals.erase(a_it.m_it);

    setDirty(1);

    return it;
}

ObjectSptr ObjectVector::at(
    const uint32_t &a_id)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr ret;
    ret = m_vals.at(a_id);
    return ret;
}

ObjectSptr ObjectVector::get(
    const uint32_t &a_id)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr ret;
    ret = at(a_id);
    return ret;
}

void ObjectVector::push_back(
    ObjectSptr    a_val)
{
    lock(m_rmutex, a_val->m_rmutex);
    lock_guard<recursive_mutex> guard1(m_rmutex,        adopt_lock);
    lock_guard<recursive_mutex> guard2(a_val->m_rmutex, adopt_lock);

    m_vals.push_back(a_val);

    setDirty(1);
}

ObjectSptr ObjectVector::back()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr ret;

    // get last element
    ret = m_vals.back();

    return ret;
}

ObjectSptr ObjectVector::pop_back()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectSptr ret;

    // get last element
    ret = back();

    // remove last element
    m_vals.pop_back();

    setDirty(1);

    return ret;
}

size_t ObjectVector::size()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_vals.size();
}

bool ObjectVector::empty()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    return m_vals.empty();
}

void ObjectVector::sort()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    std::sort(m_vals.begin(), m_vals.end());
    setDirty(1);
}

void ObjectVector::clear()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    m_vals.clear();

    setDirty(1);
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectVector::s_getType()
{
    return "core.object.vector";
}

int32_t ObjectVector::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectVector::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectVector::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectVector(a_create_flags));

    return object;
}

void ObjectVector::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_vector = {
    .getType        = ObjectVector::s_getType,
    .init           = ObjectVector::s_init,
    .shutdown       = ObjectVector::s_shutdown,
    .objectCreate   = ObjectVector::s_objectCreate,
    .getTests       = ObjectVector::s_getTests
};

