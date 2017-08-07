#include "core/neuron/media/main.hpp"

NeuronMedia::NeuronMedia()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronMedia::~NeuronMedia()
{
};

void NeuronMedia::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.media"));
    initStatus(PROP_STRING("stop"));

    // register API
    registerApiMethod(
        "setStatus",
        NeuronMedia::apiSetStatus
    );
}

int32_t NeuronMedia::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronMedia::getAllProps(
    PropMapSptr a_props)
{
    SAVE_PROP("status", getStatus);

    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronMedia::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    string  status;

    // status
    if (a_obj.hasField("status")){
        status = a_obj.getStringField("status");
        initStatus(PROP_STRING(
            status
        ));
    }

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronMedia::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t     res, err = -1;
    PropSptr    prop_status;

    // search status
    prop_status = a_props->get("status");
    if (prop_status){
        setStatus(PROP_STRING(
            prop_status->toString()
        ));
    }

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

// ---------------- api -------------------

void NeuronMedia::apiSetStatus(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronMedia     *neuron = NULL;
    PropSptr        prop_status;
    string          status;
    PropMapSptr     params;
    PropMapSptr     answer_params;

    neuron = dynamic_cast<NeuronMedia *>(a_neuron);

    // TODO check perms here

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    {
        PropSptr tmp = a_req_props->get("params");
        params = dynamic_pointer_cast<PropMap>(tmp);
    }

    if (params){
        prop_status = params->get("status");
    }
    if (prop_status){
        status = prop_status->toString();
    }
    if (    "play"  == status
        ||  "pause" == status
        ||  "stop"  == status)
    {
        neuron->setStatus(PROP_STRING(status));
        neuron->save();
    } else if (status.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                "\"descr\": \"unsupported value"
                    " for param 'status'\""
            "}"
        );
        goto out;
    } else {
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                "\"descr\": \"missing or empty param 'status'\""
            "}"
        );
        goto out;
    }

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "status was updated"
    );

out:
    return;
}

// ---------------- module ----------------

int32_t NeuronMedia::init()
{
    int32_t err = 0;
    return err;
}

void NeuronMedia::destroy()
{
}

NeuronSptr NeuronMedia::object_create()
{
    NeuronSptr neuron(new NeuronMedia);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.media",
    .init           = NeuronMedia::init,
    .destroy        = NeuronMedia::destroy,
    .object_create  = NeuronMedia::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

