#include "prop/neuron/group.hpp"

PropNeuronGroup::PropNeuronGroup()
    :   Prop(Prop::PROP_TYPE_NEURON_GROUP)
{
}

PropNeuronGroup::PropNeuronGroup(const string &a_val)
    :   Prop(Prop::PROP_TYPE_NEURON_GROUP)
{
    m_val = a_val;
    setDirty(1);

    // force to be lower
    transform(
        m_val.begin(),
        m_val.end(),
        m_val.begin(),
        ::tolower
    );
}

PropNeuronGroup::~PropNeuronGroup()
{
}

string PropNeuronGroup::toString() const
{
    return m_val;
}

string PropNeuronGroup::serialize(
    const string &) const
{
    string ret;
    ret = "\"" + m_val + "\"";
    return ret;
}

PropNeuronGroup::operator PropSptr() const
{
    return PropSptr(new PropNeuronGroup(*this));
}

