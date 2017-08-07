#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/object/__END__.hpp"
#include "thread.hpp"

#include "core/object/thread/main.hpp"

ObjectThread::ObjectThread(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectThread::ObjectThread(
    const Object::CreateFlags   &a_flags,
    ThreadSptr                  a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    m_val = a_val;
    setDirty(1);
}

ObjectThread::~ObjectThread()
{
}

void ObjectThread::do_init(
    const Object::CreateFlags &a_flags)
{
    m_val = 0;
}

string ObjectThread::getType()
{
    return ObjectThread::_getType();
}

void ObjectThread::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.thread");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectThread::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectThread::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    // TODO XXX FIXME
    PFATAL("method not implemented\n");
    //a_builder.append(a_field_name, m_val->getName());
}

void ObjectThread::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    // TODO XXX FIXME
    PFATAL("method not implemented\n");
    //a_builder.append(m_val->getName());
}

string ObjectThread::toString()
{
    string ret;
    ret = m_val->getName();
    return ret;
}

string ObjectThread::serialize(
    const string &a_delim)
{
    string ret;
    ret = "\"" + m_val->getName() + "\"";
    return ret;
}

ThreadSptr ObjectThread::getVal()
{
    return m_val;
}

// ---------------- module ----------------

string ObjectThread::_getType()
{
    return "core.object.thread";
}

string ObjectThread::_getTypePerl()
{
    return "core::object::thread::main";
}

int32_t ObjectThread::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectThread::_destroy()
{
}

ObjectSptr ObjectThread::_object_create()
{
    ObjectSptr object;
    OBJECT_THREAD(object);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectThread::_getType,
    .type_perl      = ObjectThread::_getTypePerl,
    .init           = ObjectThread::_init,
    .destroy        = ObjectThread::_destroy,
    .object_create  = ObjectThread::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

