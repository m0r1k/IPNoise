#include "core/neuron/playlist/main.hpp"

NeuronPlaylist::NeuronPlaylist()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronPlaylist::~NeuronPlaylist()
{
};

void NeuronPlaylist::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.playlist"));
    initStatus(PROP_STRING("stop"));
    initPlayOrder(PROP_STRING("listen_time"));

    // register API
    registerApiMethod(
        "setStatus",
        NeuronPlaylist::apiSetStatus
    );
    registerApiMethod(
        "setPlayOrder",
        NeuronPlaylist::apiSetPlayOrder
    );
}

int32_t NeuronPlaylist::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronPlaylist::getAllProps(
    PropMapSptr a_props)
{
    SAVE_PROP("status",         getStatus);
    SAVE_PROP("play_order",     getPlayOrder);

    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronPlaylist::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    string  status;
    string  play_order;

    // status
    if (a_obj.hasField("status")){
        status = a_obj.getStringField("status");
        initStatus(PROP_STRING(
            status
        ));
    }

    // play_order
    if (a_obj.hasField("play_order")){
        play_order = a_obj.getStringField("play_order");
        initPlayOrder(PROP_STRING(
            play_order
        ));
    }

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronPlaylist::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t     res, err = -1;

    PropSptr    prop_status;
    PropSptr    prop_play_order;

    // TODO XXX validate params before set!
    // 20140708 rchechnev@

    // search status
    prop_status = a_props->get("status");
    if (prop_status){
        setStatus(PROP_STRING(
            prop_status->toString()
        ));
    }

    // search play order
    prop_play_order = a_props->get("play_order");
    if (prop_play_order){
        setPlayOrder(PROP_STRING(
            prop_play_order->toString()
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

void NeuronPlaylist::apiSetStatus(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronPlaylist  *neuron = NULL;
    PropSptr        prop_status;
    string          status;
    PropMapSptr     params;
    PropMapSptr     answer_params;

    neuron = dynamic_cast<NeuronPlaylist *>(a_neuron);

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

void NeuronPlaylist::apiSetPlayOrder(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronPlaylist  *neuron = NULL;
    PropSptr        prop_play_order;
    string          play_order;
    PropMapSptr     params;
    PropMapSptr     answer_params;

    neuron = dynamic_cast<NeuronPlaylist *>(a_neuron);

    // TODO check perms here

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    {
        PropSptr tmp = a_req_props->get("params");
        params = dynamic_pointer_cast<PropMap>(tmp);
    }

    if (params){
        prop_play_order = params->get("play_order");
    }
    if (prop_play_order){
        play_order = prop_play_order->toString();
    }
    if ("listen_time" == play_order){
        neuron->setPlayOrder(PROP_STRING(play_order));
        neuron->save();
    } else if (play_order.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                "\"descr\": \"unsupported value"
                    " for param 'play_order'\""
            "}"
        );
        goto out;
    } else {
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"play_order\": \"failed\","
                "\"descr\": \"missing or empty param 'play_order'\""
            "}"
        );
        goto out;
    }

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "play_order was updated"
    );

out:
    return;
}



// ---------------- module ----------------

int32_t NeuronPlaylist::init()
{
    int32_t err = 0;
    return err;
}

void NeuronPlaylist::destroy()
{
}

NeuronSptr NeuronPlaylist::object_create()
{
    NeuronSptr neuron(new NeuronPlaylist);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.playlist",
    .init           = NeuronPlaylist::init,
    .destroy        = NeuronPlaylist::destroy,
    .object_create  = NeuronPlaylist::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

