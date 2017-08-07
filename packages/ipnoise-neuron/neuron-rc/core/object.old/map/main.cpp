#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/double/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/map/main.hpp"

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
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectMap::ObjectMap(
    const Object::CreateFlags   &a_flags,
    ObjectMapSptr               a_val)
    :   Object(a_flags)
{
    do_init(a_flags);

    lock_guard<recursive_mutex> guard(a_val->m_mutex);

    ObjectMapIt it;
    for (it = a_val->begin();
        it != a_val->end();
        it++)
    {
        add(it->first, it->second);
    }
    setDirty(1);
}

ObjectMap::ObjectMap(
    const Object::CreateFlags           &a_flags,
    const map<ObjectSptr, ObjectSptr>   &a_vals)
    :   Object(a_flags)
{
    map<ObjectSptr, ObjectSptr>::const_iterator it;

    do_init(a_flags);

    for (it = a_vals.begin();
        it != a_vals.end();
        it++)
    {
        m_vals[it->first] = it->second;
    }
    setDirty(1);
}

ObjectMap::ObjectMap(
    const Object::CreateFlags   &a_flags,
    const ObjectSptr            &a_key,
    const ObjectSptr            &a_val)
    :   Object(a_flags)
{
    do_init(a_flags);

    add(a_key, a_val);
    setDirty(1);
}

ObjectMap::~ObjectMap()
{
}

void ObjectMap::do_init(
    const Object::CreateFlags &a_flags)
{
}

string ObjectMap::getType()
{
    return ObjectMap::_getType();
}

void ObjectMap::do_init_props()
{
    Object::do_init_props();
}

void ObjectMap::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectMap::add(
    ObjectSptr a_key,
    ObjectSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_vals[a_key] = a_val;
    setDirty(1);
}

void ObjectMap::add(
    const string &a_key,
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectStringSptr prop_key;
    ObjectStringSptr prop_val;

    PROP_STRING(prop_key, a_key);
    PROP_STRING(prop_val, a_val);

    m_vals[prop_key] = prop_val;
    setDirty(1);
}

void ObjectMap::add(
    const string    &a_key,
    ObjectSptr      a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr prop_key;

    PROP_STRING(prop_key, a_key);

    m_vals[prop_key] = a_val;

    setDirty(1);
}

void ObjectMap::add(
    ObjectSptr      a_key,
    const string    &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr prop_val;

    PROP_STRING(prop_val, a_val);

    m_vals[a_key] = prop_val;

    setDirty(1);
}

void ObjectMap::add(
    const string    &a_key,
    const int32_t   &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectStringSptr prop_key;
    ObjectInt32Sptr  prop_val;

    PROP_STRING(prop_key, a_key);
    PROP_INT32(prop_val, a_val);

    m_vals[prop_key] = prop_val;
    setDirty(1);
}

void ObjectMap::add(
    ObjectSptr      a_key,
    const int32_t   &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectInt32Sptr  prop_val;
    PROP_INT32(prop_val, a_val);

    m_vals[a_key] = prop_val;
    setDirty(1);
}

void ObjectMap::add(
    const int32_t   &a_key,
    ObjectSptr      a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectInt32Sptr  prop_key;
    PROP_INT32(prop_key, a_key);

    m_vals[prop_key] = a_val;
    setDirty(1);
}

void ObjectMap::add(
    const string    &a_key,
    const double    &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectStringSptr prop_key;
    ObjectDoubleSptr prop_val;

    PROP_STRING(prop_key, a_key);
    PROP_DOUBLE(prop_val, a_val);

    m_vals[prop_key] = prop_val;
    setDirty(1);
}

void ObjectMap::add(
    ObjectSptr      a_key,
    const double    &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDoubleSptr  prop_val;
    PROP_DOUBLE(prop_val, a_val);

    m_vals[a_key] = prop_val;
    setDirty(1);
}

bool ObjectMap::has(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectStringSptr prop_key;

    PROP_STRING(prop_key, a_key);

    return has(prop_key);
}

bool ObjectMap::has(
    const ObjectSptr &a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

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
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_vals.size();
}

string ObjectMap::serialize(
    const string &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string                  ret, data;
    ObjectMapValsConstIt    it;

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        ObjectSptr    key_object    = it->first;
        ObjectSptr    val_object    = it->second;

        if (data.size()){
            data += ", ";
        }

        data += key_object->serialize(a_delim);
        data += a_delim;
        data += val_object->serialize(a_delim);
    }

    ret += "{";
    if (data.size()){
        ret += " " + data + " ";
    }
    ret += "}";

    return ret;
}

void ObjectMap::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapValsConstIt      it;
    mongo::BSONObjBuilder   builder;

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        ObjectSptr    key_object    = it->first;
        ObjectSptr    val_object    = it->second;

        val_object->toBSON(builder, key_object->toString());
    }

    a_builder.append(a_field_name, builder.obj());
}

void ObjectMap::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapValsConstIt      it;
    mongo::BSONObjBuilder   builder;

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        ObjectSptr    key_object    = it->first;
        ObjectSptr    val_object    = it->second;

        val_object->toBSON(builder, key_object->toString());
    }

    a_builder.append(builder.obj());
}

SV * ObjectMap::toPerl()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SV *ret         = NULL;
    HV *perl_hash   = newHV();

    ObjectMapValsConstIt    it;

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        ObjectSptr  cur_name        = it->first;
        ObjectSptr  cur_val         = it->second;
        string      cur_name_str    = cur_name->toString();

        hv_store(
            perl_hash,
            cur_name_str.c_str(),
            cur_name_str.size(),
            cur_val->toPerl(),
            0
        );
    }

    ret = newRV_noinc((SV *)perl_hash);

    return ret;
}

// ---------------- begin ----------------

ObjectMap::iterator ObjectMap::begin() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMap::iterator it;
    it.m_it = m_vals.begin();
    return it;
}

// ---------------- cbegin ----------------

ObjectMap::const_iterator ObjectMap::cbegin() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMap::const_iterator it;
    it.m_it = m_vals.cbegin();
    return it;
}

// ---------------- end ----------------

ObjectMap::iterator ObjectMap::end() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMap::iterator it;
    it.m_it = m_vals.end();
    return it;
}

// ---------------- cend ----------------

ObjectMap::const_iterator ObjectMap::cend() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMap::const_iterator it;
    it.m_it = m_vals.cend();
    return it;
}

// ---------------- find ----------------

ObjectMap::iterator ObjectMap::find(
    const ObjectSptr &a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMap::iterator it;
    it.m_it = m_vals.find(a_key);
    return it;
}

ObjectMap::iterator ObjectMap::find(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMap::iterator it;
    ObjectStringSptr    prop_key;

    PROP_STRING(prop_key, a_key);

    it.m_it = m_vals.find(prop_key);

    return it;
}

// ---------------- erase ----------------

void ObjectMap::erase(
    const ObjectMap::iterator &a_it)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_vals.erase(a_it.m_it);
    setDirty(1);
}

void ObjectMap::erase(
    ObjectSptr a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMap::iterator it = find(a_key);
    if (end() != it){
        erase(it);
    }
}

void ObjectMap::erase(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectStringSptr key;
    PROP_STRING(key, a_key);
    erase(key);
    setDirty(1);
}

void ObjectMap::clear()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapVals empty;
    m_vals = empty;
}

ObjectSptr ObjectMap::get(
    ObjectSptr a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr ret;

    if (has(a_key)){
        ret = (*this)[a_key];
    }

    return ret;
}

ObjectSptr ObjectMap::get(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr ret;
    ObjectSptr key;

    PROP_STRING(key, a_key);

    if (has(key)){
        ret = (*this)[key];
    }

    return ret;
}

bool ObjectMap::empty()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_vals.empty();
}

// for assign only!
// Example:
// ObjectMapSptr map = PROP_MAP();
// map["key"] = PROP_STRING("value");
//
// method will create empty pair!
// Example:
// ObjectSptr val = (*(map.get())["key"];
// will create key "key" with empty val ObjectSptr()
//
ObjectSptr & ObjectMap::operator [] (
    const ObjectSptr &a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

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
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr prop_key;
    PROP_STRING(prop_key, a_key);
    prop_key->setDirty(0);
    return (*this)[prop_key];
}

// ---------------- module ----------------

string ObjectMap::_getType()
{
    return "core.object.map";
}

string ObjectMap::_getTypePerl()
{
    return "core::object::map::main";
}

int32_t ObjectMap::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectMap::_destroy()
{
}

ObjectSptr ObjectMap::_object_create()
{
    ObjectSptr object;
    _OBJECT_MAP(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectMap::_getType,
    .type_perl      = ObjectMap::_getTypePerl,
    .init           = ObjectMap::_init,
    .destroy        = ObjectMap::_destroy,
    .object_create  = ObjectMap::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

