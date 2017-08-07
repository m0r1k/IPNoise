#include "thread/db.hpp"
#include "thread/http.hpp"
#include "neuron/user.hpp"
#include "neuron/god.hpp"
#include "neuron/session.hpp"
#include "neuron/param.hpp"

#include "neuron/dreamWord.hpp"

NeuronDreamWord::NeuronDreamWord()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronDreamWord::~NeuronDreamWord()
{
};

void NeuronDreamWord::do_init()
{
    setType(PROP_NEURON_TYPE("core.neuron.dream.word"));
}

int32_t NeuronDreamWord::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronDreamWord::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronDreamWord::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

int32_t NeuronDreamWord::init()
{
    int32_t err = 0;
    return err;
}

void NeuronDreamWord::destroy()
{
}

NeuronSptr NeuronDreamWord::object_create()
{
    NeuronSptr neuron(new NeuronDreamWord);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.dream.word",
    .init           = NeuronDreamWord::init,
    .destroy        = NeuronDreamWord::destroy,
    .object_create  = NeuronDreamWord::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

