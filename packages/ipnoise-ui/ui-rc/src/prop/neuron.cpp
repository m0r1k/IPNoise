#include "core/neuron/neuron/main.hpp"

#include "prop/neuron.hpp"

PropNeuron::PropNeuron()
    :   Prop(Prop::PROP_TYPE_NEURON)
{
}

PropNeuron::PropNeuron(
    NeuronSptr  &a_val)
    :   Prop(Prop::PROP_TYPE_NEURON)
{
    m_val = a_val;
    setDirty(1);
}

PropNeuron::~PropNeuron()
{
}

string PropNeuron::toString() const
{
    string ret;
    ret = m_val->toString();
    return ret;
}

string PropNeuron::serialize(
    const string &) const
{
    string ret;
    ret = m_val->serialize();
    return ret;
}

NeuronSptr PropNeuron::getVal() const
{
    return m_val;
}

PropNeuron::operator PropSptr() const
{
    return PropSptr(new PropNeuron(*this));
}

