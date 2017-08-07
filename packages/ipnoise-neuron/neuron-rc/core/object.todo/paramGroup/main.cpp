#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/param/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/paramGroup/main.hpp"

ObjectParamGroup::ObjectParamGroup(
    const Object::CreateFlags   &a_flags)
    :   ObjectParam(a_flags)
{
    do_init(a_flags);
};

ObjectParamGroup::~ObjectParamGroup()
{
};

void ObjectParamGroup::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectParamGroup::getType()
{
    return ObjectParamGroup::_getType();
}

void ObjectParamGroup::do_init_props()
{
    ObjectParam::do_init_props();

//    ObjectTypeSptr      prop_type;
    ObjectStringSptr    prop_name;

//    PROP_TYPE(   prop_type, "core.object.param.group");
    PROP_STRING( prop_name, "group");

//    INIT_PROP(this, Type, prop_type);
    INIT_PROP(this, Name, prop_name);
}

void ObjectParamGroup::do_init_api()
{
    ObjectParam::do_init_api();

    // register actions
}

int32_t ObjectParamGroup::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectParamGroup::getAllProps(
    ObjectMapSptr a_props)
{
    ObjectParam::getAllProps(a_props);
}

int32_t ObjectParamGroup::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    err = ObjectParam::parseBSON(a_obj);
    return err;
}

bool ObjectParamGroup::isAdmin()
{
    bool        ret = false;
    ObjectSptr  object_value;

    object_value = getValue();
    if (    object_value
        &&  GROUP_NAME_ADMIN == object_value->toString())
    {
        ret = true;
    }
    return ret;
}

// ---------------- module ----------------

string ObjectParamGroup::_getType()
{
    return "core.object.param.group";
}

string ObjectParamGroup::_getTypePerl()
{
    return "core::object::paramGroup::main";
}

int32_t ObjectParamGroup::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectParamGroup::_destroy()
{
}

ObjectSptr ObjectParamGroup::_object_create()
{
    ObjectSptr object;
    OBJECT_PARAM_GROUP(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectParamGroup::_getType,
    .type_perl      = ObjectParamGroup::_getTypePerl,
    .init           = ObjectParamGroup::_init,
    .destroy        = ObjectParamGroup::_destroy,
    .object_create  = ObjectParamGroup::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

