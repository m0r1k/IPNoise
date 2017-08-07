#include <math.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/objectLink/main.hpp"

ObjectLink::ObjectLink(
    const Object::CreateFlags   &a_flags)
    :   Object(Object::CreateFlags(
            a_flags | Object::REMOVE_WHEN_LINKS_LOST_FORCE
        ))
{
    // Object::REMOVE_WHEN_LINKS_LOST_FORCE mean:
    // object will remove itself,
    // when last link will be removed
    do_init(a_flags);
}

ObjectLink::~ObjectLink()
{
}

void ObjectLink::do_init(
    const Object::CreateFlags   &a_flags)
{
    PROP_MAP(m_val);
}

string ObjectLink::getType()
{
    return ObjectLink::_getType();
}

void ObjectLink::do_init_props()
{
    Object::do_init_props();
}

void ObjectLink::do_init_api()
{
    Object::do_init_api();

    // register actions
}

string ObjectLink::serialize(
    const string &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->serialize();
}

void ObjectLink::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->toBSON(a_builder, a_field_name);
}

void ObjectLink::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val->toBSON(a_builder);
}

SV * ObjectLink::toPerl()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SV *ret = NULL;
    ret = m_val->toPerl();
    return ret;
}

/*
void ObjectLink::remove(
    const int32_t &a_force)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectVectorSptr    links;
    ObjectVectorIt      links_it;
    string              object_id = getId();

    PROP_VECTOR(links);
    getLinks(links);

    if (a_force){
        for (links_it = links->begin();
            links_it != links->end();
            links_it++)
        {
            ObjectSptr link = *links_it;
            link->removeLink(object_id);
            link->save();
        }
    }

    Object::remove(a_force);
}
*/

ObjectMapSptr ObjectLink::getVal()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val;
}

void ObjectLink::setLinkCreator(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectStringSptr prop_val;

    PROP_STRING(prop_val, a_val);

    m_val->add(
        "link_creator",
        prop_val
    );
    setDirty(1);
}

/*
void ObjectLink::setup(
    ObjectSptr  a_src,
    ObjectSptr  a_dst)
{
    lock(m_mutex, a_src->m_mutex, a_dst->m_mutex);
    lock_guard<recursive_mutex> guard1(m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_src->m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard3(a_dst->m_mutex, adopt_lock);

    // bi-directional link:
    // object_src <-> object_link <-> object_dst

    ObjectLinkSptr  fake;

    addNeigh(a_src, fake, 0);
    addNeigh(a_dst, fake, 0);

    a_src->addNeigh(this, fake, 0);
    a_dst->addNeigh(this, fake, 0);
}
*/

// ---------------- module ----------------

string ObjectLink::_getType()
{
    return "core.object.link";
}

string ObjectLink::_getTypePerl()
{
    return "core::object::objectLink::main";
}

int32_t ObjectLink::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectLink::_destroy()
{
}

ObjectSptr ObjectLink::_object_create()
{
    ObjectSptr object;
    _OBJECT_LINK(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectLink::_getType,
    .type_perl      = ObjectLink::_getTypePerl,
    .init           = ObjectLink::_init,
    .destroy        = ObjectLink::_destroy,
    .object_create  = ObjectLink::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

