#include <ipnoise-common/log_common.h>

#include "object.hpp"

Object::Object(const ObjectType &a_object_type)
{
    m_is_dirty  = 0;
    m_object_type = a_object_type;
};

Object::~Object()
{
};

Object::ObjectType Object::getType() const
{
    return m_object_type;
}

int32_t Object::isDirty() const
{
    return m_is_dirty;
}

void Object::setDirty(
    const int32_t &a_val)
{
    m_is_dirty = a_val;
}

string Object::toString()
{
    lock_guard<recursive_mutex> guard(m_mutex);
    return serialize();
}

int32_t Object::toInt()
{
    int32_t ret = 0;
    string  res = toString();

    if (res.size()){
        ret = atoi(res.c_str());
    }
    return ret;
}

double Object::toDouble()
{
    string  res = toString();
    double  ret = 0.0f;

    ret = std::stod(res);

    return ret;
}

