#include "class.hpp"
DEFINE_CLASS(PropNeuronId);

#ifndef PROP_NEURON_ID_HPP
#define PROP_NEURON_ID_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>

#include <string>

#include "prop.hpp"

using namespace std;

#define PROP_NEURON_ID(x)   \
    PropNeuronIdSptr(new PropNeuronId(x))

class PropNeuronId
    :   public Prop
{
    public:
        PropNeuronId(const string &);
        PropNeuronId();
        ~PropNeuronId();

        virtual string  toString()   const;
        virtual string  serialize(
            const string &a_delim = ": "
        )  const;

        operator PropSptr() const;

    private:
        string  m_val;
};

#endif

