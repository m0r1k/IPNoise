#include "class.hpp"
DEFINE_CLASS(NeuronMedia);

#ifndef NEURON_MEDIA_HPP
#define NEURON_MEDIA_HPP

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

class NeuronMedia
    :   public  SUPER_CLASS
{
    public:
        NeuronMedia();
        virtual ~NeuronMedia();

        DEFINE_PROP(Title,  PropStringSptr);
        DEFINE_PROP(Status, PropStringSptr);

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);
        virtual int32_t apiObjectUpdate(
            HttpAnswerSptr,
            const PropMapSptr
        );

        // generic
        virtual int32_t do_autorun();

        // api
        static void apiSetStatus(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );

        // module
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();
};

#endif

