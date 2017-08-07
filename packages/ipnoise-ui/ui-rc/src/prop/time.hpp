#include "class.hpp"
DEFINE_CLASS(PropTime);

#ifndef PROP_TIME_HPP
#define PROP_TIME_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sys/time.h>

#include "prop.hpp"

using namespace std;

#define PROP_TIME(x)    \
    PropTimeSptr(new PropTime(x))

class PropTime
    :   public Prop
{
    public:
        PropTime(const double &);
        PropTime();
        ~PropTime();

        virtual string toString()   const;
        virtual string serialize(
            const string &a_delim = ": "
        )  const;

        static PropTimeSptr now();
        void                reset();
        double              getVal() const;

        operator PropSptr() const;
        operator double()   const;
        operator int64_t()  const;
        operator int32_t()  const;

        bool operator != (
            const PropTime &a_right
        ) const;

    private:
        double  m_val;
};

#endif

