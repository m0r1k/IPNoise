#include "network.hpp"
#include "core/neuron/order/main.hpp"

#include "core/neuron/orders/main.hpp"

NeuronOrders::NeuronOrders()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronOrders::~NeuronOrders()
{
};

void NeuronOrders::do_init()
{
    NeuronParamSptr param;

    // props
    initType(PROP_NEURON_TYPE("core.neuron.orders"));

    // params
}

int32_t NeuronOrders::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronOrders::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronOrders::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

// ---------------- order ----------------

NeuronOrderSptr NeuronOrders::createOrder()
{
    NeuronOrderSptr    neuron_order;

    neuron_order = getNetwork()->createNeuron<NeuronOrder>();
    LINK_AND_SAVE(this, neuron_order);

    return neuron_order;
}

// ---------------- module ----------------

int32_t NeuronOrders::init()
{
    int32_t err = 0;
    return err;
}

void NeuronOrders::destroy()
{
}

NeuronSptr NeuronOrders::object_create()
{
    NeuronSptr neuron(new NeuronOrders);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.orders",
    .init           = NeuronOrders::init,
    .destroy        = NeuronOrders::destroy,
    .object_create  = NeuronOrders::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

