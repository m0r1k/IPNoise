#include "core/neuron/photo/main.hpp"

NeuronPhoto::NeuronPhoto()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronPhoto::~NeuronPhoto()
{
};

void NeuronPhoto::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.photo"));
}

int32_t NeuronPhoto::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronPhoto::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronPhoto::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronPhoto::apiObjectUpdate(
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

int32_t NeuronPhoto::init()
{
    int32_t err = 0;
    return err;
}

void NeuronPhoto::destroy()
{
}

NeuronSptr NeuronPhoto::object_create()
{
    NeuronSptr neuron(new NeuronPhoto);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.photo",
    .init           = NeuronPhoto::init,
    .destroy        = NeuronPhoto::destroy,
    .object_create  = NeuronPhoto::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

