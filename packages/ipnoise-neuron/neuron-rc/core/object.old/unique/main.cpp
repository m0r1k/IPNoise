#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/unique/main.hpp"

// ---------------- ObjectUniqueIt ----------------

ObjectUniqueIterator::ObjectUniqueIterator()
{
}

ObjectUniqueIterator::~ObjectUniqueIterator()
{
}

// !=
bool ObjectUniqueIterator::operator != (
    const ObjectUniqueIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool ObjectUniqueIterator::operator != (
    const ObjectUniqueConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

// ==
bool ObjectUniqueIterator::operator == (
    const ObjectUniqueIterator &a_right) const
{
    return m_it == a_right.m_it;
}

bool ObjectUniqueIterator::operator == (
    const ObjectUniqueConstIterator &a_right) const
{
    return m_it == a_right.m_it;
}

// =
ObjectUniqueIterator & ObjectUniqueIterator::operator = (
    const ObjectUniqueIterator &a_it)
{
    m_it = a_it.m_it;
    return *this;
};

// ++
void ObjectUniqueIterator::operator ++ (int32_t)
{
    m_it++;
}

// *
ObjectSptr ObjectUniqueIterator::operator * () const
{
    return m_it->first;
}

// ->
ObjectMapIt ObjectUniqueIterator::operator -> () const
{
    return m_it;
}

// ---------------- ObjectUniqueConstIt ----------------

ObjectUniqueConstIterator::ObjectUniqueConstIterator()
{
}

ObjectUniqueConstIterator::~ObjectUniqueConstIterator()
{
}

// !=
bool ObjectUniqueConstIterator::operator != (
    const ObjectUniqueConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool ObjectUniqueConstIterator::operator != (
    const ObjectUniqueIterator &a_right) const
{
    return m_it != a_right.m_it;
}

// =
ObjectUniqueConstIterator & ObjectUniqueConstIterator::operator = (
    const ObjectUniqueConstIterator &a_it)
{
    m_it = a_it.m_it;
    return *this;
};

// ++
void ObjectUniqueConstIterator::operator ++ (int32_t)
{
    m_it++;
}

// *
ObjectSptr ObjectUniqueConstIterator::operator * () const
{
    return m_it->first;
}

// ->
ObjectMapConstIt ObjectUniqueConstIterator::operator -> () const
{
    return m_it;
}

// ---------------- ObjectUnique ----------------

ObjectUnique::ObjectUnique(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectUnique::ObjectUnique(
    const Object::CreateFlags   &a_flags,
    ObjectVectorSptr            a_vals)
    :   Object(a_flags)
{
    do_init(a_flags, a_vals);
}

ObjectUnique::~ObjectUnique()
{
}

void ObjectUnique::do_init(
    const Object::CreateFlags   &a_flags,
    ObjectVectorSptr            a_vals)
{
    PROP_MAP(m_val);

    if (a_vals){
        ObjectVectorIt      it;

        lock_guard<recursive_mutex> guard(a_vals->m_mutex);

        for (it = a_vals->begin();
            it != a_vals->end();
            it++)
        {
            ObjectStringSptr empty;
            m_val->add(*it, empty);
        }
    }
}

string ObjectUnique::getType()
{
    return ObjectUnique::_getType();
}

void ObjectUnique::do_init_props()
{
    Object::do_init_props();
}

void ObjectUnique::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectUnique::add(
    ObjectSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectStringSptr empty;
    PROP_STRING(empty);
    m_val->add(a_val, empty);
    setDirty(1);
}

void ObjectUnique::add(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectStringSptr prop_val;
    PROP_STRING(prop_val, a_val);
    add(prop_val);
}

void ObjectUnique::add(
    const int32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectInt32Sptr  prop_val;
    PROP_INT32(prop_val, a_val);
    add(prop_val);
}

bool ObjectUnique::has(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->has(a_key);
}

bool ObjectUnique::has(
    const ObjectSptr &a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->has(a_key);
}

uint32_t ObjectUnique::size()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->size();
}

string ObjectUnique::serialize(
    const string &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    vals;
    string              ret;

    PROP_VECTOR(vals);
    getVals(vals);

    ret = vals->serialize(a_delim);

    return ret;
}

void ObjectUnique::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    vals;
    string              ret;

    PROP_VECTOR(vals);
    getVals(vals);

    vals->toBSON(a_builder, a_field_name);
}

void ObjectUnique::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    vals;
    string              ret;

    PROP_VECTOR(vals);
    getVals(vals);

    vals->toBSON(a_builder);
}

SV * ObjectUnique::toPerl()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SV                  *ret = NULL;
    ObjectVectorSptr    vals;

    PROP_VECTOR(vals);
    getVals(vals);

    ret = vals->toPerl();

    return ret;
}

// ---------------- begin ----------------

ObjectUnique::iterator ObjectUnique::begin() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectUnique::iterator it;
    it.m_it = m_val->begin();
    return it;
}

// ---------------- cbegin ----------------

ObjectUnique::const_iterator ObjectUnique::cbegin() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectUnique::const_iterator it;
    it.m_it = m_val->cbegin();
    return it;
}

// ---------------- end ----------------

ObjectUnique::iterator ObjectUnique::end() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectUnique::iterator it;
    it.m_it = m_val->end();
    return it;
}

// ---------------- cend ----------------

ObjectUnique::const_iterator ObjectUnique::cend() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectUnique::const_iterator it;
    it.m_it = m_val->cend();
    return it;
}

// ---------------- find ----------------

ObjectUnique::iterator ObjectUnique::find(
    const ObjectSptr &a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectUnique::iterator it;
    it.m_it = m_val->find(a_key);
    return it;
}

ObjectUnique::iterator ObjectUnique::find(
    const string &a_key)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectUnique::iterator it;
    ObjectStringSptr    prop_key;

    PROP_STRING(prop_key, a_key);

    it.m_it = m_val->find(prop_key);

    return it;
}

// ---------------- erase ----------------

void ObjectUnique::erase(
    const ObjectUnique::iterator &a_it)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_val->erase(a_it.m_it);
    setDirty(1);
}

void ObjectUnique::erase(
    ObjectSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_val->erase(a_val);

    setDirty(1);
}

void ObjectUnique::erase(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectStringSptr val;
    PROP_STRING(val, a_val);
    erase(val);
}

void ObjectUnique::erase(
    const int32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectInt32Sptr val;
    PROP_INT32(val, a_val);
    erase(val);
}

void ObjectUnique::clear()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_val->clear();
}

void ObjectUnique::getVals(
    ObjectVectorSptr    a_out)
{
    ObjectMapIt it;

    for (it = m_val->begin();
        it != m_val->end();
        it++)
    {
        ObjectSptr key = it->first;
        a_out->add(key);
    }
}

ObjectSptr ObjectUnique::get(
    ObjectSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr  ret;
    ObjectMapIt it;

    it = m_val->find(a_val);
    if (m_val->end() != it){
        ret = it->first;
    }

    return ret;
}

ObjectSptr ObjectUnique::get(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr          ret;
    ObjectStringSptr    val;

    PROP_STRING(val, a_val);

    ret = get(val);

    return ret;
}

bool ObjectUnique::empty()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->empty();
}

// ---------------- module ----------------

string ObjectUnique::_getType()
{
    return "core.object.unique";
}

string ObjectUnique::_getTypePerl()
{
    return "core::object::unique::main";
}

int32_t ObjectUnique::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectUnique::_destroy()
{
}

ObjectSptr ObjectUnique::_object_create()
{
    ObjectSptr object;
    _OBJECT_UNIQUE(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectUnique::_getType,
    .type_perl      = ObjectUnique::_getTypePerl,
    .init           = ObjectUnique::_init,
    .destroy        = ObjectUnique::_destroy,
    .object_create  = ObjectUnique::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

