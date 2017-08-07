#include "thread/http.hpp"
#include "core/neuron/user/main.hpp"

#include "prop/neuron/link/event.hpp"

PropNeuronLinkEvent::PropNeuronLinkEvent(
    const Prop::PropType &a_type)
    :   PropNeuronLink(a_type)
{
    do_init();
}

PropNeuronLinkEvent::PropNeuronLinkEvent(
    const PropNeuronLinkEvent   &a_link,
    const Prop::PropType        &a_type)
    :   PropNeuronLink(a_type)
{
    m_link_info = a_link.m_link_info;
}

PropNeuronLinkEvent::~PropNeuronLinkEvent()
{
}

string PropNeuronLinkEvent::getType() const
{
    string ret = "event";
    return ret;
}

void PropNeuronLinkEvent::do_init()
{
    // add link type
    m_link_info[PropString("type")] = PropString(getType());
}

PropNeuronLinkEvent::operator PropSptr() const
{
    return PropSptr(new PropNeuronLinkEvent(*this));
}

