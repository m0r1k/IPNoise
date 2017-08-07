#include "class.hpp"
DEFINE_CLASS(NeuronBasket);

#ifndef NEURON_BASKET_HPP
#define NEURON_BASKET_HPP

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

class NeuronBasket
    :   public  SUPER_CLASS
{
    public:
        NeuronBasket();
        virtual ~NeuronBasket();

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);

        // generic
        virtual int32_t do_autorun();

        // static
        static void createOrder(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );

        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();
};

#endif

