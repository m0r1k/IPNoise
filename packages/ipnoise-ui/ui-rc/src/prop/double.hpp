#include "class.hpp"
DEFINE_CLASS(PropDouble);

#ifndef PROP_DOUBLE_HPP
#define PROP_DOUBLE_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>

#include <string>

#include "prop.hpp"

using namespace std;

#define PROP_DOUBLE(x)  \
    PropDoubleSptr(new PropDouble(x))

class PropDouble
    :   public Prop
{
    public:
        PropDouble(const double &);
        PropDouble();
        ~PropDouble();

        virtual string toString()   const;
        virtual string serialize(
            const string &a_delim = ": "
        )  const;

        // generic
        double      getVal()    const;

        operator    PropSptr()  const;
        operator    double()    const;

    private:
        double   m_val;
};

#endif

