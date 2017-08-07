#include "class.hpp"
DEFINE_CLASS(PropInt);

#ifndef PROP_INT_HPP
#define PROP_INT_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>

#include <string>

#include "prop.hpp"

using namespace std;

#define PROP_INT(x) \
    PropIntSptr(new PropInt(x))

class PropInt
    :   public Prop
{
    public:
        PropInt(const int32_t &);
        PropInt();
        ~PropInt();

        virtual string toString()   const;
        virtual string serialize(
            const string &a_delim = ": "
        )  const;

        int32_t getVal() const;

        operator PropSptr() const;
        operator int()      const;

    private:
        int32_t     m_val;
};

#endif

