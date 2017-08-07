#include "prop/int.hpp"

PropInt::PropInt()
    :   Prop(Prop::PROP_TYPE_INT)
{
    m_val = 0;
}

PropInt::PropInt(const int32_t &a_val)
    :   Prop(Prop::PROP_TYPE_INT)
{
    m_val = a_val;
    setDirty(1);
}

PropInt::~PropInt()
{
}

string PropInt::toString() const
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer),
        "%d",
        m_val
    );

    return buffer;
}

string PropInt::serialize(
    const string &) const
{
    return toString();
}

int32_t PropInt::getVal() const
{
    return m_val;
}

PropInt::operator PropSptr() const
{
    return PropSptr(new PropInt(*this));
}

PropInt::operator int() const
{
    return m_val;
}

