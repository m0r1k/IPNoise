#include "class.hpp"
DEFINE_CLASS(NeuronParamGroup);

#ifndef NEURON_PARAM_GROUP_HPP
#define NEURON_PARAM_GROUP_HPP

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

#include "core/neuron/param/main.hpp"

using namespace std;

#define GROUP_NAME_ADMIN    "admin"

class NeuronParamGroup
    :   public  NeuronParam
{
    public:
        NeuronParamGroup();
        virtual ~NeuronParamGroup();

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
        bool            isAdmin();

        // static
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();
};

#endif

