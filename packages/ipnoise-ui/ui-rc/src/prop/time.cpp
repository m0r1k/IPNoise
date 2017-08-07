#include "prop/time.hpp"

PropTime::PropTime()
    :   Prop(Prop::PROP_TYPE_TIME)
{
    m_val = 0.0f;
}

PropTime::PropTime(
    const double &a_val)
    :   Prop(Prop::PROP_TYPE_TIME)
{
    m_val = a_val;
    setDirty(1);
}

PropTime::~PropTime()
{
}

string PropTime::toString() const
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer),
        "%f",
        m_val
    );

    return buffer;
}

string PropTime::serialize(
    const string &) const
{
    return toString();
}

PropTimeSptr PropTime::now()
{
    PropTimeSptr ret(new PropTime);
    ret->reset();
    return ret;
}

void PropTime::reset()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    m_val = tv.tv_sec + tv.tv_usec / 1e6;
    setDirty(1);
}

double PropTime::getVal() const
{
    return m_val;
}

PropTime::operator PropSptr() const
{
    return PropSptr(new PropTime(*this));
}

PropTime::operator double() const
{
    return m_val;
}

PropTime::operator int32_t() const
{
    return (int32_t)m_val;
}

PropTime::operator int64_t() const
{
    return (int64_t)m_val;
}

bool PropTime::operator != (
    const PropTime &a_right
) const
{
    bool ret = false;
    ret = (m_val != a_right.m_val);
    return ret;
}

