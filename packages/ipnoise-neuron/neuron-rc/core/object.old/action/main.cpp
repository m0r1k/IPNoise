#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/objectLink/main.hpp"
//#include "core/object/objectLinkAction/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"
#include "db.hpp"
#include "db/mongo.hpp"

#include "core/object/action/main.hpp"

ObjectAction::ObjectAction(
    const Object::CreateFlags   &a_flags,
    const string                &a_name)
    :   Object(Object::CreateFlags(
            a_flags | Object::REMOVE_WHEN_LINKS_LOST_FORCE
        ))
{
    // Object::REMOVE_WHEN_LINKS_LOST_FORCE mean:
    // object will remove itself,
    // when last link will be removed
    do_init(a_flags, a_name);
}

ObjectAction::~ObjectAction()
{
}

void ObjectAction::do_init(
    const Object::CreateFlags   &a_flags,
    const string                &a_name)
{
    m_name = a_name;
}

string ObjectAction::getType()
{
    return ObjectAction::_getType();
}

void ObjectAction::do_init_props()
{
    ObjectStringSptr    prop_location;
    ObjectTimeSptr      prop_time_process;
    ObjectMapSptr       prop_info;
    ObjectMapSptr       prop_params;
    ObjectStringSptr    prop_name;

    Object::do_init_props();

    PROP_STRING(prop_location);
    PROP_TIME(prop_time_process, -0.1);
    PROP_MAP(prop_info);
    PROP_MAP(prop_params);
    PROP_STRING(prop_name, m_name);

    prop_info->add("params", prop_params);

    INIT_PROP(this, Location,    prop_location);
    INIT_PROP(this, TimeProcess, prop_time_process);
    INIT_PROP(this, Name,        prop_name);
    INIT_PROP(this, Info,        prop_info);
}

void ObjectAction::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectAction::getAllProps(
    ObjectMapSptr a_props)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SAVE_PROP("location",       getLocation);
    SAVE_PROP("time_process",   getTimeProcess);
    SAVE_PROP("info",           getInfo);

    Object::getAllProps(a_props);
}

int32_t ObjectAction::parseBSON(
    mongo::BSONObj  a_obj)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    int32_t err = -1;

    // Thread
    if (a_obj.hasField("location")){
        ObjectStringSptr    prop_location;
        string              location;

        location = a_obj.getStringField("location");
        PROP_STRING(prop_location, location);

        INIT_PROP(this, Location, prop_location);
    }

    // time process
    if (a_obj.hasField("time_process")){
        ObjectTimeSptr  prop_time_process;
        double          val = 0.0f;

        val = a_obj.getField("time_process").Double();
        PROP_TIME(prop_time_process, val);

        INIT_PROP(this, TimeProcess, prop_time_process);
    }

    // info
    if (a_obj.hasField("info")){
        ObjectMapSptr       info;
        mongo::BSONElement  el;
        mongo::BSONObj      info_obj;

        PROP_MAP(info);

        el = a_obj.getField("info");
        if (mongo::BSONType::Object != el.type()){
            PERROR("cannot parse 'info' attribute,"
                " object dump: '%s'\n",
                a_obj.toString()->c_str()
            );
            goto fail;
        }

        el.Val(info_obj);

        DbMongo::bsonToProps(
            info_obj,
            info
        );

        INIT_PROP(this, Info, info);
    }

    err = Object::parseBSON(a_obj);
    if (err){
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

/*
void ObjectAction::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   info = getInfo();

    info->toBSON(a_builder, a_field_name);
}

void ObjectAction::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   info = getInfo();

    info->toBSON(a_builder);
}

SV * ObjectAction::toPerl()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SV              *ret = NULL;
    ObjectMapSptr   info = getInfo();
    ret = info->toPerl();
    return ret;
}

string ObjectAction::serialize(
    const string &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string          ret;
    ObjectMapSptr   info = getInfo();
    ret = info->serialize(a_delim);
    return ret;
}

ObjectLinkSptr ObjectAction::addObject(
    Object  *a_object)
{
    ObjectLinkSptr    link;

    OBJECT_LINK(link);

    link->setup(this, a_object);

    return link;
}

ObjectLinkSptr ObjectAction::addObject(
    ObjectSptr  a_object)
{
    return addObject(a_object.get());
}
*/

void ObjectAction::schedule(
    ObjectTimeSptr a_time)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectTimeSptr time = a_time;
    if (!time){
        PROP_TIME(time, 0.1f);
    }
    setTimeProcess(time);
}

void ObjectAction::scheduleCurThread(
    ObjectTimeSptr a_time)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectTimeSptr      time        = a_time;
    ThreadSptr          thread      = Thread::getCurThread();
    string              location    = thread->getLocation();
    ObjectStringSptr    prop_location;

    PROP_STRING(prop_location, location);

    if (!time){
        PROP_TIME(time, 0.1f);
    }

    setTimeProcess(time);
    setLocation(prop_location);
}

ObjectMapSptr ObjectAction::getParams()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr params;
    ObjectMapSptr info = getInfo();

    params = dynamic_pointer_cast<ObjectMap>(
        info->get("params")
    );

    return params;
}

// ---------------- props ----------------

void ObjectAction::addProp(
    const string    &a_key,
    const string    &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   info = getInfo();

    info->add(a_key, a_val);

    setDirty(1);
}

void ObjectAction::addProp(
    const string    &a_key,
    ObjectSptr      a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   info = getInfo();

    info->add(a_key, a_val);

    setDirty(1);
}

void ObjectAction::addProp(
    ObjectSptr      a_key,
    const string    &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   info = getInfo();

    info->add(a_key, a_val);

    setDirty(1);
}

void ObjectAction::addProp(
    ObjectSptr      a_key,
    ObjectSptr      a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   info = getInfo();

    info->add(a_key, a_val);

    setDirty(1);
}

// ---------------- params ----------------

void ObjectAction::addParam(
    const string    &a_key,
    const string    &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   params = getParams();

    params->add(a_key, a_val);

    setDirty(1);
}

void ObjectAction::addParam(
    const string    &a_key,
    ObjectSptr      a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   params = getParams();

    params->add(a_key, a_val);

    setDirty(1);
}

void ObjectAction::addParam(
    ObjectSptr      a_key,
    const string    &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   params = getParams();

    params->add(a_key, a_val);

    setDirty(1);
}

void ObjectAction::addParam(
    ObjectSptr      a_key,
    ObjectSptr      a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   params = getParams();

    params->add(a_key, a_val);

    setDirty(1);
}

ObjectSptr ObjectAction::getParam(
    const string &a_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapSptr   params;
    params = getParams();
    return params->get(a_name);
}

// ---------------- static ----------------

bool ObjectAction::isAction(
    ObjectSptr  a_object)
{
    string          type;
    const string    prefix1     = "core.object.action";
    const string    prefix2     = "core.object.action.";
    bool            ret         = false;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    type = a_object->getType();

    if (    prefix1 == type
        ||  prefix2 == type.substr(0, prefix2.size()))
    {
        ret = true;
    }

    return ret;
}

// ---------------- module ----------------

string ObjectAction::_getType()
{
    return "core.object.action";
}

string ObjectAction::_getTypePerl()
{
    return "core::object::action::main";
}

int32_t ObjectAction::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectAction::_destroy()
{
}

ObjectSptr ObjectAction::_object_create()
{
    ObjectSptr object;
    _OBJECT_ACTION(object, 0, "");
    return object;
}

static ModuleInfo info = {
    .type           = ObjectAction::_getType,
    .type_perl      = ObjectAction::_getTypePerl,
    .init           = ObjectAction::_init,
    .destroy        = ObjectAction::_destroy,
    .object_create  = ObjectAction::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

