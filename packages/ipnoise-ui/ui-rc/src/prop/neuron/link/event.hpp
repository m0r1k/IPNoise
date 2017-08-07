#include "class.hpp"
DEFINE_CLASS(PropNeuronLinkEvent);

#ifndef PROP_NEURON_LINK_EVENT_HPP
#define PROP_NEURON_LINK_EVENT_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <map>

#include "prop.hpp"
#include "prop/map.hpp"
#include "prop/neuron/link.hpp"
#include "core/neuron/neuron/main.hpp"

using namespace std;

#define PROP_NEURON_LINK_EVENT(x) \
    PropNeuronLinkEventSptr(new PropNeuronLinkEvent(x))

class PropNeuronLinkEvent
    :   public PropNeuronLink
{
    public:
        PropNeuronLinkEvent(
            const Prop::PropType &a_type
                = Prop::PROP_TYPE_NEURON_LINK_EVENT
        );
        PropNeuronLinkEvent(
            const PropNeuronLinkEvent   &a_link,
            const Prop::PropType        &a_type
                = Prop::PROP_TYPE_NEURON_LINK_EVENT
        );

        virtual ~PropNeuronLinkEvent();

        virtual string getType() const;

        // generic
        void do_init();

        operator PropSptr() const;
};

#endif

