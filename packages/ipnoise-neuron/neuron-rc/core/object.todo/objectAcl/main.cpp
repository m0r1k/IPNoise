#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/objectAcl/main.hpp"

ObjectAclIterator::ObjectAclIterator()
{
}

ObjectAclIterator::~ObjectAclIterator()
{
}

bool ObjectAclIterator::operator != (
    const ObjectAclIterator &a_right) const
{
    return m_it != a_right.m_it;
}

void ObjectAclIterator::operator ++ (int32_t){
    m_it++;
}

ObjectMapIt ObjectAclIterator::operator -> (){
    return m_it;
}

// -------------------------- ObjectAcl ------------------------

ObjectAcl::ObjectAcl(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectAcl::ObjectAcl(
    const Object::CreateFlags   &a_flags,
    ObjectAclSptr               a_val)
    :   Object(a_flags)
{
    ObjectMapConstIt it;

    do_init(a_flags);

    for (it = a_val->m_val->cbegin();
        it != a_val->m_val->cend();
        it++)
    {
        m_val->add(it->first, it->second);
    }

    setDirty(1);
}

ObjectAcl::ObjectAcl(
    const Object::CreateFlags           &a_flags,
    const map<ObjectSptr, ObjectSptr>   &a_vals)
    :   Object(a_flags)
{
    map<ObjectSptr, ObjectSptr>::const_iterator it;

    do_init(a_flags);

    for (it = a_vals.cbegin();
        it != a_vals.cend();
        it++)
    {
        m_val->add(it->first, it->second);
    }

    setDirty(1);
}

ObjectAcl::ObjectAcl(
    const Object::CreateFlags   &a_flags,
    const ObjectSptr            &a_key,
    const ObjectSptr            &a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    add(a_key, a_val);
    setDirty(1);
}

ObjectAcl::~ObjectAcl()
{
}

void ObjectAcl::do_init(
    const Object::CreateFlags   &a_flags)
{
    PROP_MAP(m_val);
}

string ObjectAcl::getType()
{
    return ObjectAcl::_getType();
}


void ObjectAcl::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.acl");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectAcl::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectAcl::add(
    const ObjectSptr &a_key,
    const ObjectSptr &a_val)
{
    m_val->add(a_key, a_val);
    setDirty(1);
}

bool ObjectAcl::has(
    const ObjectSptr &a_key)
{
    return m_val->has(a_key);
}

string ObjectAcl::serialize(
    const string &a_delim)
{
    return m_val->serialize(a_delim);
}

void ObjectAcl::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    m_val->toBSON(a_builder, a_field_name);
}

void ObjectAcl::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    m_val->toBSON(a_builder);
}

SV * ObjectAcl::toPerl()
{
    SV *ret = NULL;
    ret = m_val->toPerl();
    return ret;
}

ObjectAcl::iterator ObjectAcl::begin()
{
    ObjectAcl::iterator it;
    it.m_it = m_val->begin();
    return it;
}

ObjectAcl::iterator ObjectAcl::end()
{
    ObjectAcl::iterator it;
    it.m_it = m_val->end();
    return it;
}

ObjectAcl::iterator ObjectAcl::find(
    const ObjectSptr &a_key)
{
    ObjectAcl::iterator it;
    it.m_it = m_val->find(a_key);
    return it;
}

void ObjectAcl::erase(
    const ObjectAcl::iterator &a_it)
{
    m_val->erase(a_it.m_it);
    setDirty(1);
}

void ObjectAcl::erase(const ObjectSptr &a_key)
{
    ObjectAcl::iterator it = find(a_key);
    erase(it);
}

ObjectSptr & ObjectAcl::operator [] (const ObjectSptr &a_key)
{
    ObjectMapIt it;

    // create new if not exist
    it = m_val->find(a_key);
    if (m_val->end() == it){
        m_val->add(a_key, ObjectSptr());
    }

    setDirty(1);

    // search again
    it = m_val->find(a_key);
    return it->second;
}

// ---------------- module ----------------

string ObjectAcl::_getType()
{
    return "core.object.acl";
}

string ObjectAcl::_getTypePerl()
{
    return "core::object::objectAcl::main";
}

int32_t ObjectAcl::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectAcl::_destroy()
{
}

ObjectSptr ObjectAcl::_object_create()
{
    ObjectSptr object;
    OBJECT_ACL(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectAcl::_getType,
    .type_perl      = ObjectAcl::_getTypePerl,
    .init           = ObjectAcl::_init,
    .destroy        = ObjectAcl::_destroy,
    .object_create  = ObjectAcl::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

