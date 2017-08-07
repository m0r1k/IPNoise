#include "prop/neuron/id.hpp"

PropNeuronId::PropNeuronId()
    :   Prop(Prop::PROP_TYPE_NEURON_ID)
{
}

PropNeuronId::PropNeuronId(const string &a_val)
    :   Prop(Prop::PROP_TYPE_NEURON_ID)
{
    m_val = a_val;
    setDirty(1);
}

PropNeuronId::~PropNeuronId()
{
}

string PropNeuronId::toString() const
{
    return m_val;
}

string PropNeuronId::serialize(
    const string &) const
{
    string ret;
    ret = "\"" + m_val + "\"";
    return ret;
}

PropNeuronId::operator PropSptr() const
{
    return PropSptr(new PropNeuronId(*this));
}

