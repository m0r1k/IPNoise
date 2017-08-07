#include "class.hpp"
DEFINE_CLASS(PropNeuron);

#ifndef PROP_NEURON_HPP
#define PROP_NEURON_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>

#include <string>

#include "prop.hpp"

DEFINE_CLASS(Neuron);

using namespace std;

#define PROP_NEURON(x)   \
    PropNeuronSptr(new PropNeuron(x))

class PropNeuron
    :   public Prop
{
    public:
        PropNeuron(NeuronSptr &);
        PropNeuron();
        ~PropNeuron();

        virtual string  toString()   const;
        virtual string  serialize(
            const string &a_delim = ": "
        )  const;

        NeuronSptr  getVal()    const;

        operator    PropSptr()  const;

    private:
        NeuronSptr  m_val;
};

#endif

