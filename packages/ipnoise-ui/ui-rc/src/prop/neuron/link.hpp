#include "class.hpp"
DEFINE_CLASS(PropNeuronLink);

#ifndef PROP_NEURON_LINK_HPP
#define PROP_NEURON_LINK_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <map>

#include "prop.hpp"
#include "prop/map.hpp"
#include "core/neuron/neuron/main.hpp"

using namespace std;

typedef map<PropSptr, PropSptr, PropCmp>        LinkInfo;
typedef LinkInfo::iterator                      LinkInfoIt;
typedef LinkInfo::const_iterator                LinkInfoConstIt;

#define PROP_NEURON_LINK(x) \
    PropNeuronLinkSptr(new PropNeuronLink(x))

class PropNeuronLink
    :   public Prop
{
    public:
        PropNeuronLink(
            const Prop::PropType &a_type
                = Prop::PROP_TYPE_NEURON_LINK
        );
        virtual ~PropNeuronLink();

        virtual string toString()   const;
        virtual string getType()    const;
        virtual string serialize(
            const string &a_delim = ": "
        )  const;

        // generic
        void    do_init();
        void    addNeigh(NeuronSptr);
        void    addNeigh(Neuron *);
        void    setLinkCreator(
            const string a_val
        );
        void    setWalkers(PropMapSptr a_walkers);

        operator PropSptr() const;

    protected:
        LinkInfo  m_link_info;
};

#endif

