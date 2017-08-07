#include "network.hpp"

#include "core/neuron/ipnoise/neighs/main.hpp"

extern Network *g_network;

NeuronIPNoiseNeighs::NeuronIPNoiseNeighs()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronIPNoiseNeighs::~NeuronIPNoiseNeighs()
{
};

void NeuronIPNoiseNeighs::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.ipnoise.neighs"));
    initHuid(PROP_STRING());
}

int32_t NeuronIPNoiseNeighs::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronIPNoiseNeighs::getAllProps(
    PropMapSptr a_props)
{
    SAVE_PROP("huid",  getHuid);

    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronIPNoiseNeighs::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    string  huid;

    // huid
    if (a_obj.hasField("huid")){
        huid = a_obj.getStringField("huid");
        initHuid(PROP_STRING(huid));
    }

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

NeuronIPNoiseNeighSptr NeuronIPNoiseNeighs::createIPNoiseNeigh()
{
    NeuronIPNoiseNeighSptr    neuron_neigh;

    neuron_neigh = getNetwork()
        ->createNeuron<NeuronIPNoiseNeigh>();
    LINK_AND_SAVE(this, neuron_neigh);

    return neuron_neigh;
}

void NeuronIPNoiseNeighs::apiAddNewNeigh(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronIPNoiseNeighs     *neighs = (NeuronIPNoiseNeighs *)a_neuron;
    PropMapSptr             answer_params;
    NeuronIPNoiseNeighSptr  neuron_neigh;
    PropSptr                neigh_id;

    // TODO check perms here

    // create neigh
    neuron_neigh = neighs->createIPNoiseNeigh();
    neigh_id     = neuron_neigh->getId();

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", neigh_id);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "neigh was created successfully"
    );
}

int32_t NeuronIPNoiseNeighs::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t     res, err = -1;
    PropSptr    prop_huid;

    // search huid
    prop_huid = a_props->get("huid");
    if (prop_huid){
        setHuid(PROP_STRING(
            prop_huid->toString()
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

// ---------------- module ----------------

int32_t NeuronIPNoiseNeighs::init()
{
    int32_t err = 0;
    return err;
}

void NeuronIPNoiseNeighs::destroy()
{
}

NeuronSptr NeuronIPNoiseNeighs::object_create()
{
    NeuronSptr neuron(new NeuronIPNoiseNeighs);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.ipnoise.neighs",
    .init           = NeuronIPNoiseNeighs::init,
    .destroy        = NeuronIPNoiseNeighs::destroy,
    .object_create  = NeuronIPNoiseNeighs::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

