#include "core/neuron/products/main.hpp"

NeuronProducts::NeuronProducts()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronProducts::~NeuronProducts()
{
};

void NeuronProducts::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.products"));
}

int32_t NeuronProducts::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronProducts::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronProducts::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronProducts::apiObjectUpdate(
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

// ---------------- module ----------------

int32_t NeuronProducts::init()
{
    int32_t err = 0;
    return err;
}

void NeuronProducts::destroy()
{
}

NeuronSptr NeuronProducts::object_create()
{
    NeuronSptr neuron(new NeuronProducts);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.products",
    .init           = NeuronProducts::init,
    .destroy        = NeuronProducts::destroy,
    .object_create  = NeuronProducts::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

