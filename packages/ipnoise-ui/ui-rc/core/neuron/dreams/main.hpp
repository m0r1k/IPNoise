#include "class.hpp"
DEFINE_CLASS(NeuronDreams);

#ifndef NEURON_DREAMS_HPP
#define NEURON_DREAMS_HPP

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
#include "core/neuron/dream/main.hpp"

#include "core/neuron/neuron/main.hpp"

#undef  SUPER_CLASS
#define SUPER_CLASS   Neuron

using namespace std;

class NeuronDreams
    :   public  SUPER_CLASS
{
    public:
        NeuronDreams();
        virtual ~NeuronDreams();

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);

        // api
        static void apiAddNewDream(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );

        // generic
        virtual int32_t do_autorun();
        NeuronDreamSptr createDream();

        // static
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();
};

#endif

