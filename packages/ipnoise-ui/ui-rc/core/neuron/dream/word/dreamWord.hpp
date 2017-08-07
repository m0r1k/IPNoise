#include "class.hpp"
DEFINE_CLASS(NeuronDreamWord);

#ifndef NEURON_DREAM_WORD_HPP
#define NEURON_DREAM_WORD_HPP

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
#include "neuron/session.hpp"

#include "neuron/neuron.hpp"

#undef  SUPER_CLASS
#define SUPER_CLASS   Neuron

using namespace std;

class NeuronDreamWord
    :   public  SUPER_CLASS
{
    public:
        NeuronDreamWord();
        virtual ~NeuronDreamWord();

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);

        // generic
        virtual int32_t do_autorun();

        // static
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();
};

#endif

