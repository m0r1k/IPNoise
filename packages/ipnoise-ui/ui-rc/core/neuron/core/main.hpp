#include "class.hpp"
DEFINE_CLASS(NeuronCore);

#ifndef NEURON_CORE_HPP
#define NEURON_CORE_HPP

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

class NeuronCore
    :   public  SUPER_CLASS
{
    public:
        NeuronCore();
        virtual ~NeuronCore();

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);

        // generic
        virtual int32_t do_autorun();

        // static
        static int32_t     module_init();
        static void        module_destroy();
        static NeuronSptr  module_object_create();

    private:
        void do_init();
};

#endif

