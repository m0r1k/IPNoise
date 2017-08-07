//#include <algorithm>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/vector/main.hpp"

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
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectVector::ObjectVector(
    const Object::CreateFlags   &a_flags,
    ObjectVectorSptr            a_vals)
    :   Object(a_flags)
{
    ObjectVectorIt                it;
    lock_guard<recursive_mutex> guard(a_vals->m_mutex);

    do_init(a_flags);

    for (it = a_vals->begin();
        it != a_vals->end();
        it++)
    {
        m_vals.push_back(*it);
    }

    setDirty(1);
}

ObjectVector::ObjectVector(
    const Object::CreateFlags   &a_flags,
    const vector<ObjectSptr>    &a_vals)
    :   Object(a_flags)
{
    do_init(a_flags);

    m_vals = a_vals;
    setDirty(1);
}

ObjectVector::ObjectVector(
    const Object::CreateFlags   &a_flags,
    const char                  *a_val,
    const char                  &a_delim,
    const bool                  &a_delim_can_repeat)
    :   Object(a_flags)
{
    ObjectVectorSptr    res;
    ObjectVectorIt      res_it;

    res = ObjectString::split(
        a_val,
        a_delim,
        a_delim_can_repeat
    );

    for (res_it = res->begin();
        res_it != res->end();
        res_it++)
    {
        ObjectSptr  cur_object = *res_it;
        m_vals.push_back(cur_object);
    }

    setDirty(1);
}

ObjectVector::~ObjectVector()
{
}

void ObjectVector::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectVector::getType()
{
    return ObjectVector::_getType();
}

void ObjectVector::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.vector");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectVector::do_init_api()
{
    Object::do_init_api();

    // register actions
}

// ---------------- begin ----------------

ObjectVector::iterator ObjectVector::begin() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVector::iterator it;
    it.m_it = m_vals.begin();
    return it;
}

/*
ObjectVector::const_iterator ObjectVector::begin() const noexcept
{
    ObjectVector::const_iterator it;
    lock_guard<recursive_mutex> guard(m_mutex);
    it.m_it = m_vals.begin();
    return it;
}
*/

// ---------------- cbegin ----------------

ObjectVector::iterator ObjectVector::cbegin() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVector::iterator it;
    it.m_it = m_vals.begin();
    return it;
}

/*
ObjectVector::const_iterator ObjectVector::cbegin() const noexcept
{
    ObjectVector::const_iterator it;
    it.m_it = m_vals.begin();
    return it;
}
*/

// ---------------- end ----------------

ObjectVector::iterator ObjectVector::end() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVector::iterator it;
    it.m_it = m_vals.end();
    return it;
}

/*
ObjectVector::const_iterator ObjectVector::end() const noexcept
{
    ObjectVector::const_iterator it;
    it.m_it = m_vals.end();
    return it;
}
*/

// ---------------- cend ----------------

ObjectVector::iterator ObjectVector::cend() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVector::iterator  it;
    it.m_it = m_vals.end();
    return it;
}

/*
ObjectVector::const_iterator ObjectVector::cend() const noexcept
{
    ObjectVector::const_iterator it;
    it.m_it = m_vals.end();
    return it;
}
*/

void ObjectVector::add(
    ObjectSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    push_back(a_val);
}

void ObjectVector::add(
    const int32_t &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectInt32Sptr prop_val;
    PROP_INT32(prop_val, a_val);

    add(prop_val);
}

void ObjectVector::add(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectStringSptr prop_val;
    PROP_STRING(prop_val, a_val);

    add(prop_val);
}

ObjectVector::iterator ObjectVector::erase(
    ObjectVector::iterator a_it)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVector::iterator it;
    it.m_it = m_vals.erase(a_it.m_it);
    return it;
}

ObjectSptr ObjectVector::at(
    const uint32_t &a_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr ret;
    ret = m_vals.at(a_id);
    return ret;
}

ObjectSptr ObjectVector::get(
    const uint32_t &a_id)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr ret;
    ret = at(a_id);
    return ret;
}

void ObjectVector::push_back(
    ObjectSptr    a_val)
{
    lock(m_mutex, a_val->m_mutex);
    lock_guard<recursive_mutex> guard1(m_mutex,        adopt_lock);
    lock_guard<recursive_mutex> guard2(a_val->m_mutex, adopt_lock);

    m_vals.push_back(a_val);
    setDirty(1);
}

ObjectSptr ObjectVector::back()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr ret;

    // get last element
    ret = m_vals.back();

    return ret;
}

ObjectSptr ObjectVector::pop_back()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectSptr ret;

    // get last element
    ret = back();

    // remove last element
    m_vals.pop_back();

    setDirty(1);

    return ret;
}

string ObjectVector::serialize(
    const string &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string                  ret, data;
    ObjectVectorValsConstIt it;

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        ObjectSptr object = *it;
        if (data.size()){
            data += ", ";
        }
        data += object->serialize(a_delim);
    }

    ret += "[";
    if (data.size()){
        ret += " " + data + " ";
    }
    ret += "]";

    return ret;
}

size_t ObjectVector::size()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_vals.size();
}

bool ObjectVector::empty()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_vals.empty();
}

void ObjectVector::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    mongo::BSONArrayBuilder     builder;
    ObjectVectorValsConstIt     it;

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        ObjectSptr object = *it;
        object->toBSON(builder);
    }

    a_builder.append(a_field_name, builder.arr());
}

void ObjectVector::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorValsConstIt       it;
    mongo::BSONArrayBuilder     builder;

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        ObjectSptr object = *it;
        object->toBSON(builder);
    }

    a_builder.append(builder.arr());
}

SV * ObjectVector::toPerl()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SV  *ret        = NULL;
    AV  *perl_array = newAV();

    ObjectVectorValsIt it;

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        ObjectSptr  cur_val     = *it;
        SV          *res        = NULL;
        res = cur_val->toPerl();
        av_push(perl_array, res);
    }
    // ret = (SV *)perl_array;
    ret = newRV_noinc((SV *)perl_array);

    return ret;
}

void ObjectVector::sort()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    std::sort(m_vals.begin(), m_vals.end());
    setDirty(1);
}

void ObjectVector::clear()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_vals.clear();
}

ObjectSptr & ObjectVector::operator [](
    const uint32_t &a_pos)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorValsIt it;


    // create new if not exist
    it = m_vals.begin() + a_pos;
    if (m_vals.end() == it){
        m_vals[a_pos] = ObjectSptr();
    }

    setDirty(1);

    // search again
    it = m_vals.begin() + a_pos;
    return *it;
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectVector::_getType()
{
    return "core.object.vector";
}

string ObjectVector::_getTypePerl()
{
    return "core::object::vector::main";
}

int32_t ObjectVector::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectVector::_destroy()
{
}

ObjectSptr ObjectVector::_object_create()
{
    ObjectSptr object;
    _OBJECT_VECTOR(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectVector::_getType,
    .type_perl      = ObjectVector::_getTypePerl,
    .init           = ObjectVector::_init,
    .destroy        = ObjectVector::_destroy,
    .object_create  = ObjectVector::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

