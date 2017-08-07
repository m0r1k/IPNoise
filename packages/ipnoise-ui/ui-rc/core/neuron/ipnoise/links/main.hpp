#include "class.hpp"
DEFINE_CLASS(NeuronIPNoiseLinks);

#ifndef NEURON_IPNOISE_LINKS_HPP
#define NEURON_IPNOISE_LINKS_HPP

#include <stdio.h>

#include <QThread>
#include <QTimer>

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
#include "core/neuron/ipnoise/link/main.hpp"

#include "core/neuron/neuron/main.hpp"

#undef  SUPER_CLASS
#define SUPER_CLASS   Neuron

using namespace std;

class NeuronIPNoiseLinks
    :   public  SUPER_CLASS
{
    Q_OBJECT

    public:
        NeuronIPNoiseLinks();
        virtual ~NeuronIPNoiseLinks();

        DEFINE_PROP(Huid, PropStringSptr);

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);
        virtual int32_t apiObjectUpdate(
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_props
        );

        // api
        static void apiAddNewLink(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );

        // generic
        virtual int32_t         do_autorun();
        NeuronIPNoiseLinkSptr   createIPNoiseLink();

        // static
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    public slots:
        void timerSlot();

    private:
        void    do_init();
};

#endif

