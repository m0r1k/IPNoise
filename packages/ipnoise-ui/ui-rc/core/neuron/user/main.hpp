#include "class.hpp"
DEFINE_CLASS(NeuronUser);
DEFINE_CLASS(NeuronOrders);
DEFINE_CLASS(NeuronOrder);
DEFINE_CLASS(NeuronDialogs);
DEFINE_CLASS(NeuronDialog);

#ifndef NEURON_USER_HPP
#define NEURON_USER_HPP

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
#include "core/neuron/basket/main.hpp"
#include "core/neuron/bookmarks/main.hpp"
#include "core/neuron/shop/main.hpp"

#include "core/neuron/neuron/main.hpp"

#undef  SUPER_CLASS
#define SUPER_CLASS   Neuron

using namespace std;

class NeuronUser
    :   public  SUPER_CLASS
{
//    Q_OBJECT

    public:
        NeuronUser();
        virtual ~NeuronUser();

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual void    beforeRequest(
            HttpAnswerSptr      a_answer,
            const PropMapSptr   a_req_props
        );
        virtual int32_t parseBSON(mongo::BSONObj);
        virtual int32_t apiObjectUpdate(
            HttpAnswerSptr,
            const PropMapSptr
        );

        // api
        static void apiGetCreateAddress(
            Neuron *,
            HttpAnswerSptr,
            const PropMapSptr
        );

        // generic
        virtual int32_t         do_autorun();
        bool                    isAdmin();
        void                    addToBookmarks(Neuron *);
        void                    delFromBookmarks(Neuron *);
        NeuronBookmarksSptr     getCreateBookmarks();
        NeuronBookmarksSptr     getBookmarks();

        NeuronParamAddressSptr  getCreateAddress(PropNeuronIdSptr);
        NeuronParamAddressSptr  getAddress(PropNeuronIdSptr);

        NeuronBasketSptr        getBasket();
        NeuronBasketSptr        getCreateBasket();
        NeuronDreamsSptr        getDreamsNeuron();
        NeuronDreamsSptr        getCreateDreamsNeuron();
        NeuronDialogsSptr       getDialogsNeuron();
        NeuronDialogsSptr       getCreateDialogsNeuron();
        NeuronOrdersSptr        getOrdersNeuron();
        NeuronOrdersSptr        getCreateOrdersNeuron();
        NeuronShopSptr          getShopNeuron();
        NeuronShopSptr          getCreateShopNeuron();
        void                    addEvent(PropMapSptr);

        void    getSessions(vector<NeuronSessionSptr> &);

        // static
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();
};

#endif

