#include "class.hpp"
DEFINE_CLASS(NeuronParamAddress);

#ifndef NEURON_PARAM_ADDRESS_HPP
#define NEURON_PARAM_ADDRESS_HPP

#include <stdio.h>

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <mutex>

#include "prop/map.hpp"
#include "prop/int.hpp"
#include "prop/string.hpp"
#include "prop.hpp"
#include "prop/neuron/type.hpp"
#include "http_answer.hpp"

#include "core/neuron/param/main.hpp"

#undef  SUPER_CLASS
#define SUPER_CLASS   NeuronParam

using namespace std;

class NeuronParamAddress
    :   public  SUPER_CLASS
{
    public:
        NeuronParamAddress();
        virtual ~NeuronParamAddress();

        DEFINE_PROP(Levels, PropVectorSptr);

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);
        virtual int32_t apiObjectUpdate(
            HttpAnswerSptr,
            const PropMapSptr
        );

        // api
        static void apiGetLevel(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );
        static void apiGetRegionTypes(
            Neuron              *a_neuron,
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );

        // generic
        virtual int32_t do_autorun();

        // static
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();

        static  int32_t  kladrSqlCb(
            void    *a_kladr_req_info,
            int32_t a_argc,
            char    **a_argv,
            char    **a_col_name
        );
};

#endif

