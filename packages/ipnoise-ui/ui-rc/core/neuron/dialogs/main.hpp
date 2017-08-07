#include "class.hpp"
DEFINE_CLASS(NeuronDialogs);
DEFINE_CLASS(NeuronDialog);

#ifndef NEURON_DIALOGS_HPP
#define NEURON_DIALOGS_HPP

#include <stdio.h>

#include <map>
#include <string>
#include <memory>

#include "prop/map.hpp"
#include "prop/int.hpp"
#include "prop/string.hpp"
#include "prop.hpp"
#include "prop/neuron/type.hpp"
#include "http_answer.hpp"

#include "core/neuron/neuron/main.hpp"

#undef  SUPER_CLASS
#define SUPER_CLASS   Neuron

using namespace std;

class NeuronDialogs
    :   public  SUPER_CLASS
{
    public:
        NeuronDialogs();
        virtual ~NeuronDialogs();

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);

        // api
        static void apiAddNewDialog(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );

        // generic
        virtual int32_t     do_autorun();
        NeuronDialogSptr    createDialog();

        // static
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();
};

#endif

