#include "core/neuron/basket/main.hpp"
#include "core/neuron/user/main.hpp"
#include "utils.hpp"

#include "core/neuron/product/main.hpp"

NeuronProduct::NeuronProduct()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronProduct::~NeuronProduct()
{
};

void NeuronProduct::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.product"));

    // register API
    registerApiMethod(
        "addToBasket",
        NeuronProduct::apiAddToBasket
    );
}

int32_t NeuronProduct::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronProduct::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronProduct::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronProduct::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t res, err = -1;

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

void NeuronProduct::apiAddToBasket(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronUserSptr      cur_user;
    NeuronBasketSptr    basket;

    cur_user = Context::getCurUser();

    // TODO check perms here

    // add to basket
    basket = cur_user->getCreateBasket();
    LINK_AND_SAVE(a_neuron, basket.get());

    // prepare event
    {
        PropMapSptr event(new PropMap);
        event->add("type",      "rerender");
        event->add("object_id", basket->getId());
        // send event to all sessions
        cur_user->addEvent(event);
    }

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "object was added to basket successfully"
    );
}

// ---------------- module ----------------

int32_t NeuronProduct::init()
{
    int32_t err = 0;
    return err;
}

void NeuronProduct::destroy()
{
}

NeuronSptr NeuronProduct::object_create()
{
    NeuronSptr neuron(new NeuronProduct);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.product",
    .init           = NeuronProduct::init,
    .destroy        = NeuronProduct::destroy,
    .object_create  = NeuronProduct::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

