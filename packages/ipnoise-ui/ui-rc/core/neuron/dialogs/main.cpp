#include "network.hpp"
#include "core/neuron/dialog/main.hpp"

#include "core/neuron/dialogs/main.hpp"

NeuronDialogs::NeuronDialogs()
    :   Neuron()
{
    do_init();
};

NeuronDialogs::~NeuronDialogs()
{
};

void NeuronDialogs::do_init()
{
    NeuronParamSptr param;

    // props
    initType(PROP_NEURON_TYPE("core.neuron.dialogs"));

    // params

    // register API
    registerApiMethod(
        "add_new_dialog",
        NeuronDialogs::apiAddNewDialog
    );
}

int32_t NeuronDialogs::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronDialogs::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronDialogs::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

void NeuronDialogs::apiAddNewDialog(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronDialogs        *dialogs = (NeuronDialogs *)a_neuron;
    PropMapSptr         answer_params;
    NeuronDialogSptr     neuron_dialog;
    PropSptr            dialog_id;

    // TODO check perms here

    // create dialog
    neuron_dialog = dialogs->createDialog();
    dialog_id     = neuron_dialog->getId();

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", dialog_id);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "dialog was created successfully"
    );
}

// ---------------- dialog ---------------

NeuronDialogSptr NeuronDialogs::createDialog()
{
    NeuronDialogSptr    neuron_dialog;

    neuron_dialog = getNetwork()->createNeuron<NeuronDialog>();
    LINK_AND_SAVE(this, neuron_dialog);

    return neuron_dialog;
}

// ---------------- module ----------------

int32_t NeuronDialogs::init()
{
    int32_t err = 0;
    return err;
}

void NeuronDialogs::destroy()
{
}

NeuronSptr NeuronDialogs::object_create()
{
    NeuronSptr neuron(new NeuronDialogs);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.dialogs",
    .init           = NeuronDialogs::init,
    .destroy        = NeuronDialogs::destroy,
    .object_create  = NeuronDialogs::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

