#include "core/object/object/__BEGIN__.hpp"
#include "core/object/objectLink/main.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/objectLinkInfo/main.hpp"

ObjectLinkInfo::ObjectLinkInfo(
    const Object::CreateFlags   &a_flags)
    :   ObjectLink(a_flags)
{
    do_init(a_flags);
}

ObjectLinkInfo::ObjectLinkInfo(
    const Object::CreateFlags   &a_flags,
    ObjectLinkInfoSptr          a_val)
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

ObjectLinkInfo::~ObjectLinkInfo()
{
}

void ObjectLinkInfo::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectLinkInfo::getType()
{
    return ObjectLinkInfo::_getType();
}

void ObjectLinkInfo::do_init_props()
{
    ObjectLink::do_init_props();
}

void ObjectLinkInfo::do_init_api()
{
    ObjectLink::do_init_api();

    // register actions
}

void ObjectLinkInfo::setWalkers(
    ObjectMapSptr a_val)
{
    m_val->add(
        "walkers",
        a_val
    );
    setDirty(1);
}

void ObjectLinkInfo::addNeigh(
    ObjectSptr a_object)
{
    addNeigh(a_object.get());
}

void ObjectLinkInfo::addNeigh(
    Object *a_object)
{
    PFATAL("FIXME\n");

/*
    ObjectMapSptr   walkers;
    ObjectMapSptr   walker_info;
    ObjectUserSptr  user;
    ContextSptr     context;

    context = Thread::getCurContext();
    if (context){
        user = context->getUser();
    }

    // walk info
    PROP_MAP(
        walker_info,
        "walk_count",
        PROP_INT32(12)
    );

    PROP_MAP(
        walkers,
        "TODO__walker_object_id",
        walker_info
    );
    m_val->add(
        "walkers",
        walkers
    );

    // add link creator
    if (user){
        m_val->add(
            "link_creator",
            user->getId()
        );
    }

    setDirty(1);
*/
}

string ObjectLinkInfo::serialize(
    const string &a_delim)
{
    return m_val->serialize(a_delim);
}

void ObjectLinkInfo::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    m_val->toBSON(a_builder, a_field_name);
}

void ObjectLinkInfo::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    m_val->toBSON(a_builder);
}

SV * ObjectLinkInfo::toPerl()
{
    SV *ret = NULL;
    ret = m_val->toPerl();
    return ret;
}

// ---------------- module ----------------

string ObjectLinkInfo::_getType()
{
    return "core.object.link.info";
}

string ObjectLinkInfo::_getTypePerl()
{
    return "core::object::objectLinkInfo::main";
}

int32_t ObjectLinkInfo::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectLinkInfo::_destroy()
{
}

ObjectSptr ObjectLinkInfo::_object_create()
{
    ObjectSptr object;
    OBJECT_LINK_INFO(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectLinkInfo::_getType,
    .type_perl      = ObjectLinkInfo::_getTypePerl,
    .init           = ObjectLinkInfo::_init,
    .destroy        = ObjectLinkInfo::_destroy,
    .object_create  = ObjectLinkInfo::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

