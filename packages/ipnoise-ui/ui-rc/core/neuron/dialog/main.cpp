#include "network.hpp"
#include "base64.hpp"

#include "core/neuron/dialog/msg/main.hpp"
#include "core/neuron/user/main.hpp"

#include "core/neuron/dialog/main.hpp"

NeuronDialog::NeuronDialog()
    :   Neuron()
{
    do_init();
};

NeuronDialog::~NeuronDialog()
{
};

void NeuronDialog::do_init()
{
    NeuronParamSptr param;

    // props
    initType(PROP_NEURON_TYPE("core.neuron.dialog"));

    // params

    // register API
    registerApiMethod(
        "message_add",
        NeuronDialog::apiMessageAdd
    );
}

int32_t NeuronDialog::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronDialog::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronDialog::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

void NeuronDialog::apiMessageAdd(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronDialog        *dialog     = (NeuronDialog *)a_neuron;
    string              dialog_id   = dialog->getId()->toString();
    PropMapSptr         answer_params;
    NeuronDialogMsgSptr neuron_dialog_msg;
    PropMapSptr         params;
    PropSptr            msg_id;
    PropSptr            prop_text_base64;
    string              msg_text_base64;
    string              msg_text;
    NeuronUserSptr      cur_user;

    // get curent user
    cur_user = Context::getCurUser();

    // TODO check perms here

    {
        PropSptr tmp = a_req_props->get("params");
        params = dynamic_pointer_cast<PropMap>(tmp);
    }

    if (params){
        prop_text_base64 = params->get("text_base64");
    }
    if (prop_text_base64){
        msg_text_base64 = prop_text_base64->toString();
    }

    // decode base64
    msg_text = base64_decode(msg_text_base64);

    // search text_base64
    if (!msg_text.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerApiStatus("failed");
        a_answer->setAnswerApiDescr(
            "empty or missing argument 'text_base64'"
            " or it is not Base64 string"
            " or base64 string is empty"
        );
        goto fail;
    }

    // create message
    neuron_dialog_msg   = dialog->createMessage();
    msg_id              = neuron_dialog_msg->getId();

    // setup message
    neuron_dialog_msg->setText(PROP_STRING(msg_text));

    // save message
    neuron_dialog_msg->save();

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", msg_id);

    // prepare event
    {
        PropMapSptr event(PROP_MAP());
        PropMapSptr params(PROP_MAP());

        // prepare params
        params->add("msg_id",   msg_id);

        // prepare event
        event->add("object_id", dialog_id);
        event->add("type",      "message_added");
        event->add("params",    params);

        cur_user->addEvent(event);
    }


    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "message was created successfully"
    );


out:
    return;
fail:
    goto out;
}

// ---------------- message --------------

NeuronDialogMsgSptr NeuronDialog::createMessage()
{
    NeuronDialogMsgSptr    neuron_dialog_msg;

    neuron_dialog_msg = getNetwork()->createNeuron<NeuronDialogMsg>();
    LINK_AND_SAVE(this, neuron_dialog_msg);

    return neuron_dialog_msg;
}

// ---------------- module ----------------

int32_t NeuronDialog::init()
{
    int32_t err = 0;
    return err;
}

void NeuronDialog::destroy()
{
}

NeuronSptr NeuronDialog::object_create()
{
    NeuronSptr neuron(new NeuronDialog);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.dialog",
    .init           = NeuronDialog::init,
    .destroy        = NeuronDialog::destroy,
    .object_create  = NeuronDialog::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

