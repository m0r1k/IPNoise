#include "module.hpp"
#include "network.hpp"

#include "core/neuron/users/main.hpp"
#include "core/neuron/ipnoise/main.hpp"
#include "core/neuron/shop/main.hpp"

#include "core/neuron/core/main.hpp"

NeuronCore::NeuronCore()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronCore::~NeuronCore()
{
};

void NeuronCore::do_init()
{
    NeuronParamSptr param;

    // props
    initType(PROP_NEURON_TYPE("core.neuron.core"));
    initId(PROP_NEURON_ID("core"));
}

int32_t NeuronCore::do_autorun()
{
    int32_t             err = -1;
    NeuronUsersSptr     users;
    NeuronShopSptr      shop;
    NeuronIPNoiseSptr   ipnoise;

    users = getCreateNeigh<NeuronUsers>();
    if (!users){
        PWARN("cannot get/create neuron NeuronUsers\n");
        goto fail;
    }

    shop = getCreateNeigh<NeuronShop>();
    if (!shop){
        PWARN("cannot get/create neuron NeuronShop\n");
        goto fail;
    }

    ipnoise = getCreateNeigh<NeuronIPNoise>();
    if (!ipnoise){
        PWARN("cannot get/create neuron NeuronIPNoise\n");
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

void NeuronCore::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronCore::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronCore::module_init()
{
    int32_t err = 0;
    return err;
}

void NeuronCore::module_destroy()
{
}

NeuronSptr NeuronCore::module_object_create()
{
    NeuronCoreSptr neuron(new NeuronCore);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.core",
    .init           = NeuronCore::module_init,
    .destroy        = NeuronCore::module_destroy,
    .object_create  = NeuronCore::module_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

