#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/dreams/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/httpAnswer/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/dreamland/main.hpp"

ObjectDreamland::ObjectDreamland(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
};

ObjectDreamland::~ObjectDreamland()
{
};

void ObjectDreamland::do_init(
    const Object::CreateFlags   &a_flags)
{
    setId("static.dreamland");
}

string ObjectDreamland::getType()
{
    return ObjectDreamland::_getType();
}

void ObjectDreamland::do_init_props()
{
    Object::do_init_props();

//    ObjectIdSptr    prop_id;
//    PROP_ID(   prop_id,   "static.dreamland");
//    INIT_PROP(this, Id,   prop_id);
}

void ObjectDreamland::do_init_api()
{
    Object::do_init_api();

    // register actions
}

int32_t ObjectDreamland::do_autorun()
{
    int32_t err = 0;
    return err;
}

void ObjectDreamland::getAllProps(
    ObjectMapSptr a_props)
{
    Object::getAllProps(a_props);
}

int32_t ObjectDreamland::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = Object::parseBSON(a_obj);
    return err;
}

void ObjectDreamland::beforeRequest(
    ObjectHttpAnswerSptr    a_answer,
    const ObjectMapSptr     a_req_props,
    const ObjectMapSptr     a_req_params)
{
    PFATAL("FIXME\n");
/*
    ContextSptr         context;
    ObjectUserSptr      cur_user;

    // get current user
    cur_user = Context::getCurUser();

    // create dreams object
    cur_user->getCreateDreamsObject();

    // process supper
    Object::beforeRequest(
        a_answer,
        a_req_props,
        a_req_params
    );
*/
}

// ---------------- module ----------------

string ObjectDreamland::_getType()
{
    return "core.object.dreamland";
}

string ObjectDreamland::_getTypePerl()
{
    return "core::object::dreamland::main";
}

int32_t ObjectDreamland::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectDreamland::_destroy()
{
}

ObjectSptr ObjectDreamland::_object_create()
{
    ObjectSptr object;
    OBJECT_DREAMLAND(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectDreamland::_getType,
    .type_perl      = ObjectDreamland::_getTypePerl,
    .init           = ObjectDreamland::_init,
    .destroy        = ObjectDreamland::_destroy,
    .object_create  = ObjectDreamland::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

