#include "prop/string.hpp"

PropString::PropString()
    :   Prop(Prop::PROP_TYPE_STRING)
{
}

PropString::PropString(const char *a_val)
    :   Prop(Prop::PROP_TYPE_STRING)
{
    m_val = a_val;
    setDirty(1);
}

PropString::PropString(const char a_val)
    :   Prop(Prop::PROP_TYPE_STRING)
{
    m_val = a_val;
    setDirty(1);
}

PropString::PropString(const string &a_val)
    :   Prop(Prop::PROP_TYPE_STRING)
{
    m_val = a_val;
    setDirty(1);
}

PropString::~PropString()
{
}

string PropString::toString() const
{
    return m_val;
}

string PropString::serialize(
    const string &) const
{
    string ret = "\"" + toString() + "\"";
    return ret;
}

void PropString::add(const string &a_val)
{
    m_val += a_val;
    setDirty(1);
}

void PropString::add(const char &a_val)
{
    m_val += a_val;
    setDirty(1);
}

string PropString::getVal() const
{
    return m_val;
}

PropString::operator PropSptr() const
{
    return PropSptr(new PropString(*this));
}

PropString::operator string() const
{
    return m_val;
}


