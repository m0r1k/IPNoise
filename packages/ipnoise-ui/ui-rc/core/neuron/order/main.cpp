#include "core/neuron/order/main.hpp"

NeuronOrder::NeuronOrder()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronOrder::~NeuronOrder()
{
};

void NeuronOrder::do_init()
{
    NeuronParamSptr param;

    // props
    initType(PROP_NEURON_TYPE("core.neuron.order"));

    // params
}

int32_t NeuronOrder::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronOrder::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronOrder::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

int32_t NeuronOrder::init()
{
    int32_t err = 0;
    return err;
}

void NeuronOrder::destroy()
{
}

NeuronSptr NeuronOrder::object_create()
{
    NeuronSptr neuron(new NeuronOrder);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.order",
    .init           = NeuronOrder::init,
    .destroy        = NeuronOrder::destroy,
    .object_create  = NeuronOrder::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

