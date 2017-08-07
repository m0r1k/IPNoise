#include "network.hpp"

#include "prop/neuron/type.hpp"

extern Network *g_network;

PropNeuronType::PropNeuronType()
    :   Prop(Prop::PROP_TYPE_NEURON_TYPE)
{
}

PropNeuronType::PropNeuronType(const char *a_val)
    :   Prop(Prop::PROP_TYPE_STRING)
{
    m_val = a_val;
    setDirty(1);
}

PropNeuronType::PropNeuronType(const string &a_val)
    :   Prop(Prop::PROP_TYPE_STRING)
{
    m_val = a_val;
    setDirty(1);
}

PropNeuronType::~PropNeuronType()
{
}

string PropNeuronType::toString() const
{
    return m_val;
}

string PropNeuronType::serialize(
    const string &) const
{
    string ret = "\"" + toString() + "\"";
    return ret;
}

PropNeuronType::operator PropSptr() const
{
    return PropSptr(new PropNeuronType(*this));
}

PropNeuronType::operator string() const
{
    return m_val;
}

void PropNeuronType::add(const string &a_val)
{
    m_val += a_val;
    setDirty(1);
}

void PropNeuronType::add(const char &a_val)
{
    m_val += a_val;
    setDirty(1);
}

string PropNeuronType::serializeInfo(
    const string &a_delim) const
{
//    char                buffer[512] = { 0x00 };
    string              ret, data;

    PropMapSptr         info = PROP_MAP();
    PropMap::iterator   info_it;

    g_network->getNeuronsInfo(info);

    ret = info->serialize(a_delim);

/*
    for (info_it = info->begin();
        info_it != info->end();
        info_it++)
    {
        if (data.size()){
            data += ", ";
        }
        snprintf(buffer, sizeof(buffer),
            "{ 'type_str'%s '%s' }",
            a_delim.c_str(),
            info_it->first->toString().c_str()
        );
        data += buffer;
    }

    ret += "[";
    if (data.size()){
        ret += " ";
    }
    ret += data;
    if (data.size()){
        ret += " ";
    }
    ret += "]";
*/

    return ret;
}

