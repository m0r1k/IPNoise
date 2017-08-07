#include "network.hpp"

#include "thread/db.hpp"
#include "thread/http.hpp"

#include "core/neuron/user/main.hpp"
#include "core/neuron/god/main.hpp"
#include "core/neuron/session/main.hpp"
#include "core/neuron/param/main.hpp"

#include "core/neuron/dreams/main.hpp"

NeuronDreams::NeuronDreams()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronDreams::~NeuronDreams()
{
};

void NeuronDreams::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.dreams"));

    // register API
    registerApiMethod(
        "add_new_dream",
        NeuronDreams::apiAddNewDream
    );
}

int32_t NeuronDreams::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronDreams::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronDreams::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

NeuronDreamSptr NeuronDreams::createDream()
{
    NeuronDreamSptr    neuron_dream;

    neuron_dream = getNetwork()->createNeuron<NeuronDream>();
    LINK_AND_SAVE(this, neuron_dream);

    return neuron_dream;
}

void NeuronDreams::apiAddNewDream(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronDreams        *dreams = (NeuronDreams *)a_neuron;
    PropMapSptr         answer_params;
    NeuronDreamSptr     neuron_dream;
    PropSptr            dream_id;

    // TODO check perms here

    // create dream
    neuron_dream = dreams->createDream();
    dream_id     = neuron_dream->getId();

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", dream_id);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "dream was created successfully"
    );
}

// ---------------- module ----------------

int32_t NeuronDreams::init()
{
    int32_t err = 0;
    return err;
}

void NeuronDreams::destroy()
{
}

NeuronSptr NeuronDreams::object_create()
{
    NeuronSptr neuron(new NeuronDreams);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.dreams",
    .init           = NeuronDreams::init,
    .destroy        = NeuronDreams::destroy,
    .object_create  = NeuronDreams::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

