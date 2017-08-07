#include "class.hpp"
DEFINE_CLASS(NeuronIPNoiseNeigh);

#ifndef NEURON_IPNOISE_NEIGH_HPP
#define NEURON_IPNOISE_NEIGH_HPP

#include <stdio.h>

#include <QThread>
#include <map>
#include <string>
#include <memory>

#include "prop/map.hpp"
#include "prop/int.hpp"
#include "prop/string.hpp"
#include "prop.hpp"
#include "prop/neuron/type.hpp"
#include "http_answer.hpp"

#include "utils.hpp"
#include "core/neuron/session/main.hpp"

#include "core/neuron/neuron/main.hpp"

#undef  SUPER_CLASS
#define SUPER_CLASS   Neuron

using namespace std;

class NeuronIPNoiseNeigh
    :   public  SUPER_CLASS
{
    public:
        NeuronIPNoiseNeigh();
        virtual ~NeuronIPNoiseNeigh();

        DEFINE_PROP(DstHuid, PropStringSptr);
        DEFINE_PROP(SrcHuid, PropStringSptr);
        DEFINE_PROP(LLAddr,  PropStringSptr);
        DEFINE_PROP(DevName, PropStringSptr);

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);
        virtual int32_t apiObjectUpdate(
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_props
        );

        // generic
        virtual int32_t do_autorun();
        string          getLinkAddress();
        string          getLinkName();
        void            timer();

        // static
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();
};

#endif

