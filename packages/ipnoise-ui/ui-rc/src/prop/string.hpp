#include "class.hpp"
DEFINE_CLASS(PropString);

#ifndef PROP_STRING_HPP
#define PROP_STRING_HPP

#include <stdio.h>

#include <string>

#include "prop.hpp"

using namespace std;

#define PROP_STRING(x) \
    PropStringSptr(new PropString(x))

class PropString
    :   public Prop
{
    public:
        PropString(const string &);
        PropString(const char);
        PropString(const char *);
        PropString();
        ~PropString();

        virtual string toString()   const;
        virtual string serialize(
            const string &a_delim = ": "
        )  const;

        operator PropSptr() const;
        operator string()   const;

        // generic
        void    add(const string   &);
        void    add(const char     &);
        string  getVal() const;

    private:
        string  m_val;
};

#endif

