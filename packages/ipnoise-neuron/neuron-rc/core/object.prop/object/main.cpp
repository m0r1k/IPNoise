#include "core/object/object/main.hpp"

#include "object/object.hpp"

Object::Object()
    :   Object(Object::OBJECT_TYPE_OBJECT)
{
}

Object::Object(
    const Object &a_val)
    :   Object(Object::OBJECT_TYPE_OBJECT)
{
    m_val = a_val.m_val;
    setDirty(1);
}

Object::Object(
    ObjectSptr  &a_val)
    :   Object(Object::OBJECT_TYPE_OBJECT)
{
    m_val = a_val;
    setDirty(1);
}

Object::~Object()
{
}

string Object::toString()
{
    string ret;
    ret = m_val->toString();
    return ret;
}

void Object::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    m_val->toBSON(a_builder, a_field_name);
}

void Object::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    m_val->toBSON(a_builder);
}

string Object::serialize(
    const string &)
{
    string ret;
    ret = m_val->serialize();
    return ret;
}

ObjectSptr Object::getVal() const
{
    return m_val;
}

Object::operator ObjectSptr() const
{
    return ObjectSptr(new Object(*this));
}

