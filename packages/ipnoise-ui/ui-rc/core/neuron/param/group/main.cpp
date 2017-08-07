#include "core/neuron/param/group/main.hpp"

NeuronParamGroup::NeuronParamGroup()
    :   NeuronParam()
{
    do_init();
};

NeuronParamGroup::~NeuronParamGroup()
{
};

void NeuronParamGroup::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.param.group"));
    initName(PROP_STRING("group"));
}

int32_t NeuronParamGroup::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronParamGroup::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronParamGroup::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronParamGroup::apiObjectUpdate(
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

// --------------------------------------------------------------------------80

bool NeuronParamGroup::isAdmin()
{
    bool        ret = false;
    PropSptr    prop_value;

    prop_value = getValue();
    if (    prop_value
        &&  GROUP_NAME_ADMIN == prop_value->toString())
    {
        ret = true;
    }
    return ret;
}

// ---------------- module ----------------

int32_t NeuronParamGroup::init()
{
    int32_t err = 0;
    return err;
}

void NeuronParamGroup::destroy()
{
}

NeuronSptr NeuronParamGroup::object_create()
{
    NeuronSptr neuron(new NeuronParamGroup);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.param.group",
    .init           = NeuronParamGroup::init,
    .destroy        = NeuronParamGroup::destroy,
    .object_create  = NeuronParamGroup::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

