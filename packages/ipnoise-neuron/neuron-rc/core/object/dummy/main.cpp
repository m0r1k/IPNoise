#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/double/main.hpp>
#include <core/object/int32/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>
#include <core/object/vector/main.hpp>

#include "main.hpp"

ObjectDummy::ObjectDummy(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectDummy::~ObjectDummy()
{
}

string ObjectDummy::getType()
{
    return ObjectDummy::s_getType();
}

int32_t ObjectDummy::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t err = 0;

    err = Object::do_init_as_prop(a_data, a_data_size);

    return err;
}

int32_t ObjectDummy::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = -1;

    res = Object::do_init_as_object(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    m_map       = OBJECT<ObjectMap>(CREATE_PROP);
    m_vector    = OBJECT<ObjectVector>(CREATE_PROP);
    m_string    = OBJECT<ObjectString>(CREATE_PROP);
    m_int32     = OBJECT<ObjectInt32>(CREATE_PROP);
    m_double    = OBJECT<ObjectDouble>(CREATE_PROP);

    // all ok
    err = 0;

out:
    return err;

fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

ObjectSptr ObjectDummy::copy()
{
    ObjectDummySptr ret;

    ret = OBJECT<ObjectDummy>();

    ret->m_map = dynamic_pointer_cast<ObjectMap>(
        m_map->copy()
    );

    return ret;
}

void ObjectDummy::getAllProps(
    ObjectMapSptr   a_out)
{
    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        a_out->add("dummy_map",       m_map);
        a_out->add("dummy_vector",    m_vector);
        a_out->add("dummy_string",    m_string);
        a_out->add("dummy_int32",     m_int32);
        a_out->add("dummy_double",    m_double);

        Object::getAllProps(a_out);
    }
}

int32_t ObjectDummy::saveAsProp(
    const char *a_path)
{
    string val;

    // m_map
    m_map->add("morik", "morik was here");
    m_map->add("tigra", "tigra was NOT here");

    // m_vector
    m_vector->add("pluto 0");
    m_vector->add("pluto 1");
    m_vector->add("pluto 2");

    // m_string
    val = "test";
    m_string->do_init_as_object(val.c_str(), val.size());

    // m_int32
    val = "1234567890";
    m_int32->do_init_as_object(val.c_str(), val.size());

    // m_double
    val = "0.987654321";
    m_double->do_init_as_object(val.c_str(), val.size());

    return 0;
}

// ---------------- static ----------------

// ---------------- module ----------------

string ObjectDummy::s_getType()
{
    return "core.object.dummy";
}

int32_t ObjectDummy::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectDummy::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectDummy::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectDummy(a_create_flags));

    return object;
}

void ObjectDummy::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_dummy = {
    .getType        = ObjectDummy::s_getType,
    .init           = ObjectDummy::s_init,
    .shutdown       = ObjectDummy::s_shutdown,
    .objectCreate   = ObjectDummy::s_objectCreate,
    .getTests       = ObjectDummy::s_getTests
};

