#include "core/neuron/photos/main.hpp"

NeuronPhotos::NeuronPhotos()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronPhotos::~NeuronPhotos()
{
};

void NeuronPhotos::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.photos"));
}

int32_t NeuronPhotos::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronPhotos::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronPhotos::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronPhotos::apiObjectUpdate(
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

int32_t NeuronPhotos::init()
{
    int32_t err = 0;
    return err;
}

void NeuronPhotos::destroy()
{
}

NeuronSptr NeuronPhotos::object_create()
{
    NeuronSptr neuron(new NeuronPhotos);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.photos",
    .init           = NeuronPhotos::init,
    .destroy        = NeuronPhotos::destroy,
    .object_create  = NeuronPhotos::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

