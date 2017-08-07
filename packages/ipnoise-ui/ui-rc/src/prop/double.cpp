#include <math.h>
#include "log.hpp"

#include "prop/double.hpp"

PropDouble::PropDouble()
    :   Prop(Prop::PROP_TYPE_DOUBLE)
{
    m_val = 0.f;
}

PropDouble::PropDouble(const double &a_val)
    :   Prop(Prop::PROP_TYPE_DOUBLE)
{
    m_val = a_val;
    setDirty(1);
}

PropDouble::~PropDouble()
{
}

string PropDouble::toString() const
{
    char    buffer[512]     = { 0x00 };
    double  integer_part    = 0.0f;
    double  fractional_part = modf(m_val, &integer_part);

    if (!fractional_part){
        // every time add '.00' to integers numbers
        // to show that it is float numbers
        snprintf(buffer, sizeof(buffer),
            "%.2f",
            m_val
        );
    } else {
        // print as generary
        snprintf(buffer, sizeof(buffer),
            "%f",
            m_val
        );
    }

    return buffer;
}

double PropDouble::getVal() const
{
    return m_val;
}

string PropDouble::serialize(
    const string &) const
{
    return toString();
}

PropDouble::operator PropSptr() const
{
    return PropSptr(new PropDouble(*this));
}

PropDouble::operator double() const
{
    return m_val;
}

