#include "thread/http.hpp"
#include "core/neuron/user/main.hpp"

#include "prop/neuron/link.hpp"

PropNeuronLink::PropNeuronLink(
    const Prop::PropType &a_type)
    :   Prop(a_type)
{
    do_init();
}

PropNeuronLink::~PropNeuronLink()
{
}

string PropNeuronLink::getType() const
{
    string ret = "neuron";
    return ret;
}

void PropNeuronLink::do_init()
{
    // add link type
    m_link_info[PropString("type")] = PropString(getType());
}

void PropNeuronLink::setLinkCreator(
    const string a_val)
{
    m_link_info[PropString("link_creator")] = PropString(a_val);
    setDirty(1);
}

void PropNeuronLink::setWalkers(
    PropMapSptr a_val)
{
    m_link_info[PropString("walkers")] = a_val;
    setDirty(1);
}

void PropNeuronLink::addNeigh(
    NeuronSptr a_neuron)
{
    addNeigh(a_neuron.get());
}

void PropNeuronLink::addNeigh(
    Neuron *a_neuron)
{
    PropMapSptr     walkers;
    PropMapSptr     walker_info;
    NeuronUserSptr  user;
    ContextSptr     context;

    context = ThreadHttp::getContext();
    if (context){
        user = context->getUser();
    }

    // walk info
    walker_info = PROP_MAP(
        PropString("walk_count"), PropInt(12)
    );

    walkers = PROP_MAP(
        PropString("TODO__walker_object_id"),
        walker_info
    );
    m_link_info[PropString("walkers")] = walkers;

    // add link creator
    if (user){
        m_link_info[PropString("link_creator")] = PropString(
            user->getId()->toString()
        );
    }

    setDirty(1);
}

string PropNeuronLink::serialize(
    const string &a_delim) const
{
    LinkInfoConstIt     it;
    string              ret;
    string              data;

    for (it = m_link_info.begin();
        it != m_link_info.end();
        it++)
    {
        if (data.size()){
            data += ", ";
        }
        data += it->first->serialize(a_delim);
        data += a_delim;
        data += it->second->serialize(a_delim);
    }

    ret += "{";
    if (data.size()){
        ret += " " + data + " ";
    }
    ret += "}";

    return ret;
}

string PropNeuronLink::toString() const
{
    return serialize();
}

PropNeuronLink::operator PropSptr() const
{
    return PROP_NEURON_LINK(*this);
}

