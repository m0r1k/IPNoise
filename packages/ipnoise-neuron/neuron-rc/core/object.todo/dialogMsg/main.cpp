#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/dialogMsg/main.hpp"

ObjectDialogMsg::ObjectDialogMsg(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectDialogMsg::~ObjectDialogMsg()
{
};

void ObjectDialogMsg::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectDialogMsg::getType()
{
    return ObjectDialogMsg::_getType();
}

void ObjectDialogMsg::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr    prop_type;
    ObjectStringSptr  prop_text;

//    PROP_TYPE(prop_type, "core.object.dialog.msg");
    PROP_STRING(prop_text);

//    INIT_PROP(this, Type, prop_type);
    INIT_PROP(this, Text, prop_text);
}

void ObjectDialogMsg::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectDialogMsg::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectDialogMsg::getAllProps(
    ObjectMapSptr a_props)
{
    SAVE_PROP("text", getText);

    Object::getAllProps(a_props);
}

int32_t ObjectDialogMsg::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    // text
    if (a_obj.hasField("text")){
        string              text;
        ObjectStringSptr    prop_text;

        text = a_obj.getStringField("text");
        PROP_STRING(prop_text, text);

        INIT_PROP(this, Text, prop_text);
    }

    err = Object::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectDialogMsg::_getType()
{
    return "core.object.dialog.msg";
}

string ObjectDialogMsg::_getTypePerl()
{
    return "core::object::dialogMsg::main";
}

int32_t ObjectDialogMsg::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectDialogMsg::_destroy()
{
}

ObjectSptr ObjectDialogMsg::_object_create()
{
    ObjectSptr object;
    OBJECT_DIALOG_MSG(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectDialogMsg::_getType,
    .type_perl      = ObjectDialogMsg::_getTypePerl,
    .init           = ObjectDialogMsg::_init,
    .destroy        = ObjectDialogMsg::_destroy,
    .object_create  = ObjectDialogMsg::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

