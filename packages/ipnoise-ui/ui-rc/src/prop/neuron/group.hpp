#include "class.hpp"
DEFINE_CLASS(PropNeuronGroup);

#ifndef PROP_NEURON_GROUP_HPP
#define PROP_NEURON_GROUP_HPP

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <algorithm>

#include "prop.hpp"

using namespace std;

class PropNeuronGroup
    :   public Prop
{
    public:
        PropNeuronGroup(const string &);
        PropNeuronGroup();
        ~PropNeuronGroup();

        virtual string  toString()   const;
        virtual string  serialize(
            const string &a_delim = ": "
        )  const;

        operator PropSptr() const;

    private:
        string  m_val;
};

#endif

