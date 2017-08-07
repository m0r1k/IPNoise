#include "class.hpp"
DEFINE_CLASS(NeuronIPNoiseSkb);

#ifndef NEURON_IPNOISE_SKB_HPP
#define NEURON_IPNOISE_SKB_HPP

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

class NeuronIPNoiseSkb
    :   public  SUPER_CLASS
{
    public:
        NeuronIPNoiseSkb();
        virtual ~NeuronIPNoiseSkb();

        DEFINE_PROP(InLinkName,     PropStringSptr);
        DEFINE_PROP(InLinkIndex,    PropIntSptr);
        DEFINE_PROP(InContent,      PropStringSptr);
        DEFINE_PROP(From,           PropStringSptr);
        DEFINE_PROP(To,             PropStringSptr);

/*
        DEFINE_PROP(DstHuid, PropStringSptr);
        DEFINE_PROP(SrcHuid, PropStringSptr);
        DEFINE_PROP(LLAddr,  PropStringSptr);
        DEFINE_PROP(DevName, PropStringSptr);
*/

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
//        string  getLinkAddress();
//        string  getLinkName();

        // static
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();
};

#endif

