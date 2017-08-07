#include <math.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/double/main.hpp"

ObjectDouble::ObjectDouble(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectDouble::ObjectDouble(
    const Object::CreateFlags   &a_flags,
    const double                &a_val)
    :   Object(a_flags)
{
    do_init(a_flags, a_val);
    setDirty(1);
}

ObjectDouble::~ObjectDouble()
{
}

void ObjectDouble::do_init(
    const Object::CreateFlags   &a_flags,
    const double                &a_val)
{
    m_val = a_val;
}

string ObjectDouble::getType()
{
    return ObjectDouble::_getType();
}

void ObjectDouble::do_init_props()
{
    Object::do_init_props();
}

void ObjectDouble::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectDouble::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    a_builder.append(a_field_name, m_val);
}

void ObjectDouble::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    a_builder.append(m_val);
}

SV * ObjectDouble::toPerl()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SV *ret = NULL;
    ret = newSVnv(m_val);
    return ret;
}

double ObjectDouble::getVal()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val;
}

string ObjectDouble::serialize(
    const string &)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer),
        "%g",
        m_val
    );
    return buffer;
}

ObjectDouble::operator double()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val;
}

/*
ObjectDouble & ObjectDouble::operator -= (
    const ObjectDouble &a_right)
{
    this->m_val -= a_right.m_val;
    return *this;
}

const ObjectDouble ObjectDouble::operator - (
    const ObjectDouble &a_right) const
{
    return ObjectDouble(*this) -= a_right;
}

bool ObjectDouble::operator != (
    const ObjectDouble &a_right) const
{
    bool ret = false;

    if (this->m_val != a_right.m_val){
        ret = true;
    }

    return ret;
}
*/

// ---------------- module ----------------

string ObjectDouble::_getType()
{
    return "core.object.double";
}

string ObjectDouble::_getTypePerl()
{
    return "core::object::dialog::main";
}

int32_t ObjectDouble::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectDouble::_destroy()
{
}

ObjectSptr ObjectDouble::_object_create()
{
    ObjectSptr object;
    _OBJECT_DOUBLE(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectDouble::_getType,
    .type_perl      = ObjectDouble::_getTypePerl,
    .init           = ObjectDouble::_init,
    .destroy        = ObjectDouble::_destroy,
    .object_create  = ObjectDouble::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

