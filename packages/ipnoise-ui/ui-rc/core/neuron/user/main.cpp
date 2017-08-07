#include "network.hpp"
#include "module.hpp"
#include "thread/worker/http.hpp"

#include "core/neuron/param/group/main.hpp"
#include "core/neuron/param/address/main.hpp"
#include "core/neuron/session/main.hpp"
#include "core/neuron/orders/main.hpp"
#include "core/neuron/order/main.hpp"
#include "core/neuron/dreams/main.hpp"
#include "core/neuron/dream/main.hpp"
#include "core/neuron/dialogs/main.hpp"
#include "core/neuron/dialog/main.hpp"

#include "core/neuron/user/main.hpp"

NeuronUser::NeuronUser()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronUser::~NeuronUser()
{
};

void NeuronUser::do_init()
{
    // init props
    initType(PROP_NEURON_TYPE("core.neuron.user"));

    // register API
    registerApiMethod(
        "getCreateAddress",
        NeuronUser::apiGetCreateAddress
    );
}

int32_t NeuronUser::do_autorun()
{
    int32_t             err = -1;
    NeuronDialogsSptr   dialogs;
    NeuronBasketSptr    basket;

    dialogs = getCreateNeigh<NeuronDialogs>();
    if (!dialogs){
        PWARN("cannot get/create neuron NeuronDialogs\n");
        goto fail;
    }

    basket = getCreateNeigh<NeuronBasket>();
    if (!basket){
        PWARN("cannot get/create neuron NeuronBasket\n");
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

void NeuronUser::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronUser::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = -1;

    err = SUPER_CLASS::parseBSON(a_obj);
    if (err){
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void NeuronUser::beforeRequest(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    // create dialogs neuron
    getCreateDialogsNeuron();

    // process supper
    SUPER_CLASS::beforeRequest(
        a_answer,
        a_req_props
    );
}

int32_t NeuronUser::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t         res, err = -1;

    PropSptr        prop_password;
    NeuronParamSptr param_password;

    // get password
    prop_password = a_props->get("password");
    if (prop_password){
        param_password = getCreateParam("password");
        param_password->setValue(
            dynamic_pointer_cast<PropString>(prop_password)
        );
        param_password->save();
    }

    // process upper props
    res = SUPER_CLASS::apiObjectUpdate(
        a_answer,
        a_props
    );
    if (res){
        PERROR("Cannot update props for object with ID: '%s'\n",
            getId()->toString().c_str()
        );
        err = res;
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// ---------------- address ----------------

NeuronParamAddressSptr NeuronUser::getAddress(
    PropNeuronIdSptr    a_neuron_id)
{
    NeuronSptr              tmp;
    NeuronParamAddressSptr  neuron_address;

    // TODO check perms here
    // 20130724 rchechnev

    tmp             = getNetwork()->getNeuronById(a_neuron_id);
    neuron_address  = dynamic_pointer_cast<NeuronParamAddress>(tmp);

    return neuron_address;
}

NeuronParamAddressSptr NeuronUser::getCreateAddress(
    PropNeuronIdSptr    a_neuron_id)
{
    NeuronParamAddressSptr  neuron_address;

    // TODO check perms here
    // 20130724 rchechnev

    neuron_address = getAddress(a_neuron_id);
    if (!neuron_address){
        // create new
        neuron_address = getNetwork()
            ->createNeuron<NeuronParamAddress>();
        LINK_AND_SAVE(this, neuron_address);
    }

    return neuron_address;
}

void NeuronUser::apiGetCreateAddress(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronUser              *user = (NeuronUser *)a_neuron;
    NeuronParamAddressSptr  neuron_address;
    PropMapSptr             answer_params;
    PropMapSptr             req_params;
    PropSptr                prop_address_id;
    string                  address_id;

    // get request params
    req_params = ThreadWorkerHttp::getParams(a_answer, a_req_props);
    if (req_params){
        prop_address_id = req_params->get("address_id");
    }
    if (prop_address_id){
        address_id = prop_address_id->toString();
    }

    // get or create address
    neuron_address = user->getCreateAddress(
        PROP_NEURON_ID(address_id)
    );

    // create answer
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("address_id", neuron_address->getId());

    {
        string      answer;
        string      status = "success";
        string      descr  = "address was created successfully";

        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus(status);
        a_answer->setAnswerApiDescr(descr);
    }
}

// ---------------- orders ----------------

NeuronOrdersSptr NeuronUser::getOrdersNeuron()
{
    NeuronOrdersSptr    neuron_orders;

    // TODO check perms here
    // 20130902 rchechnev

    neuron_orders = getNeigh<NeuronOrders>();
    return neuron_orders;
}

NeuronOrdersSptr NeuronUser::getCreateOrdersNeuron()
{
    NeuronOrdersSptr    neuron_orders;

    neuron_orders = getOrdersNeuron();
    if (!neuron_orders){
        neuron_orders = getNetwork()->createNeuron<NeuronOrders>();
        LINK_AND_SAVE(this, neuron_orders.get());
    }

    return neuron_orders;
}

// ---------------- dialogs ---------------

NeuronDialogsSptr NeuronUser::getDialogsNeuron()
{
    NeuronDialogsSptr   neuron_dialogs;

    // TODO check perms here
    // 20131120 rchechnev

    neuron_dialogs = getNeigh<NeuronDialogs>();
    return neuron_dialogs;
}

NeuronDialogsSptr NeuronUser::getCreateDialogsNeuron()
{
    NeuronDialogsSptr    neuron_dialogs;

    neuron_dialogs = getDialogsNeuron();
    if (!neuron_dialogs){
        neuron_dialogs = getNetwork()->createNeuron<NeuronDialogs>();
        LINK_AND_SAVE(this, neuron_dialogs.get());
    }

    return neuron_dialogs;
}

// -------------------------- generic ---------------------------------------80

bool NeuronUser::isAdmin()
{
    bool                   ret = false;
    NeuronParamGroupSptr   param_group;

    vector <NeuronParamGroupSptr>           groups;
    vector <NeuronParamGroupSptr>::iterator groups_it;

    if ("god" == getId()->toString()){
        ret = true;
        goto out;
    }

    getParams<NeuronParamGroup>("group", groups);
    for (groups_it = groups.begin();
        groups_it != groups.end();
        groups_it++)
    {
        NeuronParamGroupSptr cur_group = *groups_it;
        ret = cur_group->isAdmin();
        if (ret){
            break;
        }
    }

out:
    return ret;
}

NeuronBookmarksSptr NeuronUser::getBookmarks()
{
    NeuronBookmarksSptr neuron;
    neuron = getNeigh<NeuronBookmarks>();
    return neuron;
}

NeuronBookmarksSptr NeuronUser::getCreateBookmarks()
{
    NeuronBookmarksSptr neuron;
    neuron = getBookmarks();
    if (!neuron){
        neuron = getNetwork()->createNeuron<NeuronBookmarks>();
        LINK_AND_SAVE(this, neuron.get());
    }
    return neuron;
}

void NeuronUser::addToBookmarks(
    Neuron  *a_neuron)
{
    NeuronSptr      neuron;
    NeuronBookmarksSptr bookmarks;

    bookmarks  = getCreateBookmarks();
    bookmarks->addNeigh(a_neuron, 0);
    bookmarks->save();
}

void NeuronUser::delFromBookmarks(
    Neuron  *a_neuron)
{
    NeuronSptr      neuron;
    NeuronBookmarksSptr bookmarks;

    bookmarks  = getCreateBookmarks();
    bookmarks->neighUnlink(a_neuron->getId(), 0);
    bookmarks->save();
}

// basket
NeuronBasketSptr NeuronUser::getBasket()
{
    NeuronBasketSptr neuron;
    neuron = getNeigh<NeuronBasket>();
    return neuron;
}

NeuronBasketSptr NeuronUser::getCreateBasket()
{
    NeuronBasketSptr neuron;
    neuron = getBasket();
    if (!neuron){
        neuron = createNeigh<NeuronBasket>();
    }
    return neuron;
}

// dreams
NeuronDreamsSptr NeuronUser::getDreamsNeuron()
{
    NeuronDreamsSptr neuron;
    neuron = getNeigh<NeuronDreams>();
    return neuron;
}

NeuronDreamsSptr NeuronUser::getCreateDreamsNeuron()
{
    NeuronDreamsSptr neuron;
    neuron = getDreamsNeuron();
    if (!neuron){
        neuron = getNetwork()
            ->createNeuron<NeuronDreams>();
        LINK_AND_SAVE(this, neuron.get());
    }

    return neuron;
}

// shop
NeuronShopSptr NeuronUser::getShopNeuron()
{
    NeuronShopSptr neuron;

    neuron = getNeigh<NeuronShop>();

    return neuron;
}

NeuronShopSptr NeuronUser::getCreateShopNeuron()
{
    NeuronShopSptr neuron;

    neuron = getShopNeuron();
    if (!neuron){
        neuron = getNetwork()->createNeuron<NeuronShop>();
        LINK_AND_SAVE(this, neuron.get());
    }

    return neuron;
}

// sessions
void NeuronUser::getSessions(
    vector<NeuronSessionSptr> &a_out)
{
    getNeighs<NeuronSession>(a_out);
}

void NeuronUser::addEvent(PropMapSptr a_event)
{
    vector<NeuronSessionSptr>               sessions;
    vector<NeuronSessionSptr>::iterator     sessions_it;

    getSessions(sessions);

    for (sessions_it = sessions.begin();
        sessions_it != sessions.end();
        sessions_it++)
    {
        NeuronSessionSptr session = (*sessions_it);
        session->addEvent(a_event);
    }
}

// ---------------- module ----------------

int32_t NeuronUser::init()
{
    int32_t err = 0;
    return err;
}

void NeuronUser::destroy()
{
}

NeuronSptr NeuronUser::object_create()
{
    NeuronSptr neuron(new NeuronUser);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.user",
    .init           = NeuronUser::init,
    .destroy        = NeuronUser::destroy,
    .object_create  = NeuronUser::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

