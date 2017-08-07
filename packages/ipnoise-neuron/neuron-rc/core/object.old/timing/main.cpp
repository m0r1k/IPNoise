#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/double/main.hpp"
#include "core/object/int32/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/timing/main.hpp"

ObjectTiming::ObjectTiming(
    const Object::CreateFlags   &a_flags,
    const double                &a_time_max_ms,
    const string                &a_msg,
    const string                &a_file,
    const int32_t               &a_line,
    const string                &a_function)
    :   Object(a_flags)
{
    do_init(
        a_flags,
        a_time_max_ms,
        a_msg,
        a_file,
        a_line,
        a_function
    );
}

ObjectTiming::~ObjectTiming()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    m_time_end      = tv.tv_sec + tv.tv_usec / 1e6;
    m_time_delta    = m_time_end - m_time_init;

    dumpToStderr();
}

void ObjectTiming::do_init(
    const Object::CreateFlags   &a_flags,
    const double                &a_time_max_ms,
    const string                &a_msg,
    const string                &a_file,
    const int32_t               &a_line,
    const string                &a_function)
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    m_time_init     = tv.tv_sec + tv.tv_usec / 1e6;
    m_time_end      = 0.0f;
    m_time_delta    = 0.0f;
    m_time_max      = a_time_max_ms / 1000;
    m_msg           = a_msg;
    m_file          = a_file;
    m_line          = a_line;
    m_function      = a_function;
}

string ObjectTiming::getType()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return ObjectTiming::_getType();
}

void ObjectTiming::do_init_props()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    Object::do_init_props();
}

void ObjectTiming::do_init_api()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    Object::do_init_api();

    // register actions
}

void ObjectTiming::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    a_builder.append(a_field_name, m_time_delta);
}

void ObjectTiming::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    a_builder.append(m_time_delta);
}

SV * ObjectTiming::toPerl()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SV *ret = NULL;

    ret = newSVnv(m_time_delta);

    return ret;
}

string ObjectTiming::serialize(
    const string &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    char buffer[512] = { 0x00 };

    snprintf(buffer, sizeof(buffer),
        "%f",
        m_time_delta
    );

    return buffer;
}

void ObjectTiming::dumpToStderr()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    PRINT_TIMING(
        m_time_max,
        m_file.c_str(),
        m_line,
        m_function.c_str(),
        m_time_delta,
        "%s",
        m_msg.c_str()
    );

    if (    m_time_max
        &&  m_time_max <= m_time_delta)
    {
        PFATAL("very slow.. boost it!\n");
    }
}

// ---------------- module ----------------

string ObjectTiming::_getType()
{
    return "core.object.timing";
}

string ObjectTiming::_getTypePerl()
{
    return "core::object::timing::main";
}

int32_t ObjectTiming::_init()
{
    int32_t err = 0;

    if (1){
        PTIMING(0.0f, "calculating timing, deep 1 (outer)\n");
        PTIMING(0.0f, "calculating timing, deep 2\n");
        PTIMING(0.0f, "calculating timing, deep 3\n");
        PTIMING(0.0f, "calculating timing, deep 4\n");
        PTIMING(0.0f, "calculating timing, deep 5 (inner)\n");
    }

    return err;
}

void ObjectTiming::_destroy()
{
}

ObjectSptr ObjectTiming::_object_create()
{
    ObjectSptr object;
    _OBJECT_TIMING(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectTiming::_getType,
    .type_perl      = ObjectTiming::_getTypePerl,
    .init           = ObjectTiming::_init,
    .destroy        = ObjectTiming::_destroy,
    .object_create  = ObjectTiming::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

