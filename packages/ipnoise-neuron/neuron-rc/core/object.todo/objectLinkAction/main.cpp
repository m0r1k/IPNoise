#include "core/object/object/__BEGIN__.hpp"
#include "core/object/objectLink/main.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/objectLinkAction/main.hpp"

ObjectLinkAction::ObjectLinkAction(
    const Object::CreateFlags   &a_flags)
    :   ObjectLink(a_flags)
{
    do_init(a_flags);
}

ObjectLinkAction::ObjectLinkAction(
    const Object::CreateFlags   &a_flags,
    ObjectLinkActionSptr         a_val)
    :   ObjectLink(a_flags)
{
    ObjectMapConstIt    it;
    ObjectMapSptr       val;

    do_init(a_flags);

    val = a_val->getVal();
    for (it = val->cbegin();
        it != val->cend();
        it++)
    {
        m_val->add(it->first, it->second);
    }

    setDirty(1);
}

ObjectLinkAction::~ObjectLinkAction()
{
}

void ObjectLinkAction::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectLinkAction::getType()
{
    return ObjectLinkAction::_getType();
}

void ObjectLinkAction::do_init_props()
{
    ObjectLink::do_init_props();
}

void ObjectLinkAction::do_init_api()
{
    ObjectLink::do_init_api();

    // register actions
}

string ObjectLinkAction::serialize(
    const string &a_delim)
{
    return m_val->serialize(a_delim);
}

void ObjectLinkAction::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    m_val->toBSON(a_builder, a_field_name);
}

void ObjectLinkAction::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    m_val->toBSON(a_builder);
}

SV * ObjectLinkAction::toPerl()
{
    SV *ret = NULL;
    ret = m_val->toPerl();
    return ret;
}

void ObjectLinkAction::setup(
    Object  *a_src,
    Object  *a_dst)
{
    lock(m_mutex, a_src->m_mutex, a_dst->m_mutex);
    lock_guard<recursive_mutex> guard1(m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(a_src->m_mutex, adopt_lock);
    lock_guard<recursive_mutex> guard3(a_dst->m_mutex, adopt_lock);

    // one direction link:
    // object_src <-> link_action -> object_dst

    ObjectLinkSptr  fake;

    addNeigh(a_src, fake, 0);
    addNeigh(a_dst, fake, 0);

    a_src->addNeigh(this, fake, 0);
    // don't create back link!
    // a_dst->addNeigh(this, fake, 0);
}

// ---------------- module ----------------

string ObjectLinkAction::_getType()
{
    return "core.object.link.action";
}

string ObjectLinkAction::_getTypePerl()
{
    return "core::object::objectLinkAction::main";
}

int32_t ObjectLinkAction::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectLinkAction::_destroy()
{
}

ObjectSptr ObjectLinkAction::_object_create()
{
    ObjectSptr object;
    OBJECT_LINK_ACTION(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectLinkAction::_getType,
    .type_perl      = ObjectLinkAction::_getTypePerl,
    .init           = ObjectLinkAction::_init,
    .destroy        = ObjectLinkAction::_destroy,
    .object_create  = ObjectLinkAction::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

