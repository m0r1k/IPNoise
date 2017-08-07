#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/objectLink/main.hpp"
#include "core/object/objectLinkInfo/main.hpp"
#include "core/object/objectLinkAction/main.hpp"
#include "core/object/param/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/user/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/god/main.hpp"

ObjectGod::ObjectGod(
    const Object::CreateFlags   &a_flags)
    :   ObjectUser(a_flags)
{
    do_init(a_flags);
};

ObjectGod::~ObjectGod()
{
};

void ObjectGod::do_init(
    const Object::CreateFlags   &a_flags)
{
    setId("static.users.god");
}

string ObjectGod::getType()
{
    return ObjectGod::_getType();
}

void ObjectGod::do_init_props()
{
    ObjectStringSptr    prop_name;
    ObjectStringSptr    prop_password;

    ObjectUser::do_init_props();

    PROP_STRING(prop_name,      GOD_LOGIN);
    PROP_STRING(prop_password,  GOD_PASSWORD);

    INIT_PROP(this, Name,       prop_name);
    INIT_PROP(this, Password,   prop_password);
}

void ObjectGod::do_init_api()
{
    ObjectUser::do_init_api();

    // register actions
}

int32_t ObjectGod::do_autorun()
{
    int32_t err = 0;

/*
    ObjectParamSptr param_login;
    ObjectParamSptr param_passwd;

    // params
    param_login = getParam<ObjectParam>("login");
    if (!param_login){
        param_login = createParam<ObjectParam>("login");
        param_login->setValue(GOD_LOGIN);
        addNeigh(param_login);
    }
    param_passwd = getParam<ObjectParam>("password");
    if (!param_passwd){
        param_passwd = createParam<ObjectParam>("password");
        param_passwd->setValue(GOD_PASSWORD);
        addNeigh(param_passwd);
    }
*/

    return  err;
}

void ObjectGod::getAllProps(
    ObjectMapSptr a_props)
{
    ObjectUser::getAllProps(a_props);
}

int32_t ObjectGod::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = ObjectUser::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

string ObjectGod::_getType()
{
    return "core.object.god";
}

string ObjectGod::_getTypePerl()
{
    return "core::object::god::main";
}

int32_t ObjectGod::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectGod::_destroy()
{
}

ObjectSptr ObjectGod::_object_create()
{
    ObjectSptr object;
    OBJECT_GOD(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectGod::_getType,
    .type_perl      = ObjectGod::_getTypePerl,
    .init           = ObjectGod::_init,
    .destroy        = ObjectGod::_destroy,
    .object_create  = ObjectGod::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

