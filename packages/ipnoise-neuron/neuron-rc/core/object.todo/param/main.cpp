#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/param/main.hpp"

ObjectParam::ObjectParam(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectParam::~ObjectParam()
{
};

void ObjectParam::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectParam::getType()
{
    return ObjectParam::_getType();
}

void ObjectParam::do_init_props()
{
    Object::do_init_props();

    ObjectStringSptr prop_value;
    PROP_STRING( prop_value);
    INIT_PROP(this, Value, prop_value);
}

void ObjectParam::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectParam::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectParam::getAllProps(
    ObjectMapSptr a_props)
{
    SAVE_PROP("value",  getValue);

    Object::getAllProps(a_props);
}

/*
int32_t ObjectParam::save()
{
    ObjectMapSptr props;
    PROP_MAP(props);
    getAllProps(props);
    return saveProps(props);
}
*/

int32_t ObjectParam::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    string  value;

    // value
    if (a_obj.hasField("value")){
        ObjectStringSptr    prop_value;
        value = a_obj.getStringField("value");
        PROP_STRING(prop_value, value);
        INIT_PROP(this, Value, prop_value);
    }

    err = Object::parseBSON(a_obj);
    return err;
}

void ObjectParam::actionUpdate(
    Object          *a_object,
    ObjectAction    *a_action)
{
    lock_guard<recursive_mutex> guard(a_object->m_mutex);

    PFATAL("FIXME\n");

/*

    ObjectStringSptr    object_value;
    ObjectParam         *object = NULL;

    object = static_cast<ObjectParam *>(a_object);

    // search value
    object_value = dynamic_pointer_cast<ObjectString>(
        a_req_params->get("value")
    );
    object->setValue(object_value);

    // process upper objects
    Object::actionUpdate(
        object,
        a_answer,
        a_req_props,
        a_req_params
    );
*/
}

void ObjectParam::setValue(
    const string &a_value)
{
    ObjectStringSptr    prop_value;
    PROP_STRING(prop_value, a_value);
    setValue(prop_value);
}

// ---------------- module ----------------

string ObjectParam::_getType()
{
    return "core.object.param";
}

string ObjectParam::_getTypePerl()
{
    return "core::object::param::main";
}

int32_t ObjectParam::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectParam::_destroy()
{
}

ObjectSptr ObjectParam::_object_create()
{
    ObjectSptr object;
    OBJECT_PARAM(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectParam::_getType,
    .type_perl      = ObjectParam::_getTypePerl,
    .init           = ObjectParam::_init,
    .destroy        = ObjectParam::_destroy,
    .object_create  = ObjectParam::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

