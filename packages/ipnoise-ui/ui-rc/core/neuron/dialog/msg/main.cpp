#include "core/neuron/dialog/msg/main.hpp"

NeuronDialogMsg::NeuronDialogMsg()
    :   Neuron()
{
    do_init();
};

NeuronDialogMsg::~NeuronDialogMsg()
{
};

void NeuronDialogMsg::do_init()
{
    NeuronParamSptr param;

    // props
    initType(PROP_NEURON_TYPE("core.neuron.dialog.msg"));
    initText(PROP_STRING());

    // params
}

int32_t NeuronDialogMsg::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronDialogMsg::getAllProps(
    PropMapSptr a_props)
{
    SAVE_PROP("text", getText);

    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronDialogMsg::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    string  text;

    // text
    if (a_obj.hasField("text")){
        text = a_obj.getStringField("text");
        initText(PROP_STRING(text));
    }

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

int32_t NeuronDialogMsg::init()
{
    int32_t err = 0;
    return err;
}

void NeuronDialogMsg::destroy()
{
}

NeuronSptr NeuronDialogMsg::object_create()
{
    NeuronSptr neuron(new NeuronDialogMsg);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.dialog.msg",
    .init           = NeuronDialogMsg::init,
    .destroy        = NeuronDialogMsg::destroy,
    .object_create  = NeuronDialogMsg::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

