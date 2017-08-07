#include "core/neuron/ipnoise/links/main.hpp"
#include "core/neuron/user/main.hpp"
#include "utils.hpp"

#include "core/neuron/ipnoise/main.hpp"

NeuronIPNoise::NeuronIPNoise()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronIPNoise::~NeuronIPNoise()
{
};

void NeuronIPNoise::do_init()
{
    // props
    initType(PROP_NEURON_TYPE("core.neuron.ipnoise"));
    initId(PROP_NEURON_ID("core.ipnoise"));

    // params

    // register API
}

int32_t NeuronIPNoise::do_autorun()
{
    int32_t err = 0;

    // init neighs if need
    getCreateNeigh<NeuronIPNoiseLinks>();

    return err;
}

void NeuronIPNoise::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronIPNoise::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronIPNoise::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t     res, err = -1;

    PropSptr    prop_dst_huid;
    PropSptr    prop_src_huid;
    PropSptr    prop_lladdr;
    PropSptr    prop_dev_name;

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

// ---------------- module ----------------

int32_t NeuronIPNoise::init()
{
    int32_t err = 0;
    return err;
}

void NeuronIPNoise::destroy()
{
}

NeuronSptr NeuronIPNoise::object_create()
{
    NeuronSptr neuron(new NeuronIPNoise);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.ipnoise",
    .init           = NeuronIPNoise::init,
    .destroy        = NeuronIPNoise::destroy,
    .object_create  = NeuronIPNoise::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

