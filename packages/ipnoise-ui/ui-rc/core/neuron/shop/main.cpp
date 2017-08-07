#include "network.hpp"
#include "thread/db.hpp"
#include "thread/http.hpp"

#include "core/neuron/user/main.hpp"
#include "core/neuron/god/main.hpp"
#include "core/neuron/session/main.hpp"
#include "core/neuron/param/main.hpp"

#include "core/neuron/shop/main.hpp"

#undef  SUPER_CLASS
#define SUPER_CLASS   Neuron

NeuronShop::NeuronShop()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronShop::~NeuronShop()
{
};

void NeuronShop::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.shop"));
    initId(PROP_NEURON_ID("core.shop"));
}

int32_t NeuronShop::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronShop::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronShop::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

int32_t NeuronShop::init()
{
    int32_t err = 0;
    return err;
}

void NeuronShop::destroy()
{
}

NeuronSptr NeuronShop::object_create()
{
    NeuronSptr neuron(new NeuronShop);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.shop",
    .init           = NeuronShop::init,
    .destroy        = NeuronShop::destroy,
    .object_create  = NeuronShop::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

