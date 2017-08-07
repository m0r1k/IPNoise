#include "utils.hpp"
#include "module.hpp"
//#include "network.hpp"

#include "core/neuron/orders/main.hpp"
#include "core/neuron/order/main.hpp"
#include "core/neuron/product/main.hpp"

#include "core/neuron/basket/main.hpp"

NeuronBasket::NeuronBasket()
    :   Neuron()
{
    do_init();
};

NeuronBasket::~NeuronBasket()
{
};

void NeuronBasket::do_init()
{
    // props
    initType(PROP_NEURON_TYPE("core.neuron.basket"));

    // register API
    registerApiMethod(
        "createOrder",
        NeuronBasket::createOrder
    );
}

int32_t NeuronBasket::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronBasket::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronBasket::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

void NeuronBasket::createOrder(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronBasket        *basket = (NeuronBasket *)a_neuron;
    PropMapSptr         answer_params;
    NeuronOrdersSptr    neuron_orders;
    NeuronOrderSptr     neuron_order;
    NeuronUserSptr      cur_user;
    string              order_id;

    vector< NeuronProductSptr >             products;
    vector< NeuronProductSptr >::iterator   products_it;

    // get cur user
    cur_user = Context::getCurUser();

    // TODO check perms here

    // create order
    neuron_orders   = cur_user->getCreateOrdersNeuron();
    neuron_order    = neuron_orders->createOrder();
    order_id        = neuron_order->getId()->toString();

    basket->getNeighs<NeuronProduct>(products);
    for (products_it = products.begin();
        products_it != products.end();
        products_it++)
    {
        NeuronProductSptr product = *products_it;
        NeuronSptr        neuron;

        // link product to order
        neuron = dynamic_pointer_cast<Neuron>(product);
        neuron_order->addNeigh(neuron);

        // unlink product from basket
        product->neighUnlink(basket->getId());
        basket->neighUnlink(product->getId());

        // save product
        product->save();
    }

    // save order
    neuron_order->save();

    // save basket
    basket->save();

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", order_id);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "order was created successfully"
    );
}

static ModuleInfo info = {
    .type           = "core.neuron.basket",
    .init           = NeuronBasket::init,
    .destroy        = NeuronBasket::destroy,
    .object_create  = NeuronBasket::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

int32_t NeuronBasket::init()
{
    int32_t err = 0;
    return err;
}

void NeuronBasket::destroy()
{
}

NeuronSptr NeuronBasket::object_create()
{
    NeuronSptr neuron(new NeuronBasket);
    return neuron;
}

