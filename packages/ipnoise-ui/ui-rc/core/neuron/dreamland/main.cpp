#include "thread/db.hpp"
#include "thread/http.hpp"

#include "core/neuron/user/main.hpp"
#include "core/neuron/god/main.hpp"
#include "core/neuron/session/main.hpp"
#include "core/neuron/param/main.hpp"

#include "core/neuron/dreamland/main.hpp"

NeuronDreamland::NeuronDreamland()
    :   Neuron()
{
    do_init();
};

NeuronDreamland::~NeuronDreamland()
{
};

void NeuronDreamland::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.dreamland"));
    initId(PROP_NEURON_ID("dreamland"));
}

int32_t NeuronDreamland::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronDreamland::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronDreamland::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

void NeuronDreamland::beforeRequest(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    ContextSptr         context;
    NeuronUserSptr      cur_user;

    // get current user
    cur_user = Context::getCurUser();

    // create dreams neuron
    cur_user->getCreateDreamsNeuron();

    // process supper
    SUPER_CLASS::beforeRequest(
        a_answer,
        a_req_props
    );
}

// ---------------- module ----------------

int32_t NeuronDreamland::init()
{
    int32_t err = 0;
    return err;
}

void NeuronDreamland::destroy()
{
}

NeuronSptr NeuronDreamland::object_create()
{
    NeuronSptr neuron(new NeuronDreamland);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.dreamland",
    .init           = NeuronDreamland::init,
    .destroy        = NeuronDreamland::destroy,
    .object_create  = NeuronDreamland::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

