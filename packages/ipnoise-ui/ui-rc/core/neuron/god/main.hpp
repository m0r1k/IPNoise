#include "class.hpp"
DEFINE_CLASS(NeuronGod);

#ifndef NEURON_GOD_HPP
#define NEURON_GOD_HPP

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

#include "core/neuron/user/main.hpp"

#undef  SUPER_CLASS
#define SUPER_CLASS   NeuronUser

#define GOD_LOGIN       "god"
#define GOD_PASSWORD    "god"

using namespace std;

class NeuronGod
    :   public  SUPER_CLASS
{
    public:
        NeuronGod();
        virtual ~NeuronGod();

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);
        virtual int32_t do_autorun();

        // generic

        // static
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();
};

#endif

