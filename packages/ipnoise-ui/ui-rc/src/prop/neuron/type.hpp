#include "class.hpp"
DEFINE_CLASS(PropNeuronType);

#ifndef PROP_NEURON_TYPE_HPP
#define PROP_NEURON_TYPE_HPP

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <map>

#include "prop.hpp"

using namespace std;

#define PROP_NEURON_TYPE(x) \
    PropNeuronTypeSptr(new PropNeuronType(x))

class PropNeuronType
    :   public Prop
{
    public:
        PropNeuronType(const string &);
        PropNeuronType(const char *);
        PropNeuronType();
        ~PropNeuronType();

        virtual string toString()   const;
        virtual string serialize(
            const string &a_delim = ": "
        )  const;

        operator PropSptr() const;
        operator string()   const;

        // generic
        void    add(const string   &);
        void    add(const char     &);
        string  serializeInfo(
            const string &a_delim
        ) const;

    private:
        string  m_val;
};

#endif

