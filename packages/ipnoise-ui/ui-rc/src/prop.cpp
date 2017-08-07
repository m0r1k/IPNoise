#include <ipnoise-common/log_common.h>

#include "prop.hpp"

Prop::Prop(const PropType &a_prop_type)
{
    m_is_dirty  = 0;
    m_prop_type = a_prop_type;
};

Prop::~Prop()
{
};

Prop::PropType Prop::getType() const
{
    return m_prop_type;
}

int32_t Prop::isDirty() const
{
    return m_is_dirty;
}

void Prop::setDirty(
    const int32_t &a_val)
{
    m_is_dirty = a_val;
}

int32_t Prop::toInt() const
{
    int32_t ret = 0;
    string  res = toString();

    if (res.size()){
        ret = atoi(res.c_str());
    }
    return ret;
}

