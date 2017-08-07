#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"
#include <openssl/sha.h>
#include "base64.hpp"

#include "core/object/openssl/main.hpp"

ObjectOpenSSL::ObjectOpenSSL(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectOpenSSL::~ObjectOpenSSL()
{
}

void ObjectOpenSSL::do_init(
    const Object::CreateFlags &a_flags)
{
    PROP_MAP(m_val);
}

string ObjectOpenSSL::getType()
{
    return ObjectOpenSSL::_getType();
}

void ObjectOpenSSL::do_init_props()
{
    Object::do_init_props();
}

void ObjectOpenSSL::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectOpenSSL::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    m_val->toBSON(a_builder, a_field_name);
}

void ObjectOpenSSL::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    m_val->toBSON(a_builder);
}

SV * ObjectOpenSSL::toPerl()
{
    return m_val->toPerl();
}

string ObjectOpenSSL::toString()
{
    return m_val->toString();
}

string ObjectOpenSSL::serialize(
    const string &a_delim)
{
    return m_val->serialize(a_delim);
}

ObjectMapSptr ObjectOpenSSL::getVal()
{
    return m_val;
}

// ---------------- static ----------------

string ObjectOpenSSL::sha1(
    const string    &a_data,
    const int32_t   &a_output_base64)
{
    char    buffer[1024] = { 0x00 };
    string  ret;

    SHA1(
        (const unsigned char *)a_data.c_str(),
        (size_t)a_data.size(),
        (unsigned char *)buffer
    );

    if (a_output_base64){
        ret = base64_encode(
            (const unsigned char*)buffer,
            strlen(buffer)
        );
    } else {
        ret = buffer;
    }

    return ret;
}

// ---------------- module ----------------

string ObjectOpenSSL::_getType()
{
    return "core.object.openssl";
}

string ObjectOpenSSL::_getTypePerl()
{
    return "core::object::openssl::main";
}

int32_t ObjectOpenSSL::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectOpenSSL::_destroy()
{
}

ObjectSptr ObjectOpenSSL::_object_create()
{
    ObjectSptr object;
    _OBJECT_OPENSSL(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectOpenSSL::_getType,
    .type_perl      = ObjectOpenSSL::_getTypePerl,
    .init           = ObjectOpenSSL::_init,
    .destroy        = ObjectOpenSSL::_destroy,
    .object_create  = ObjectOpenSSL::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

