#include "thread/db.hpp"
#include "thread/http.hpp"

#include "core/neuron/user/main.hpp"
#include "core/neuron/god/main.hpp"
#include "core/neuron/session/main.hpp"
#include "core/neuron/param/main.hpp"

#include "core/neuron/dream/main.hpp"

NeuronDream::NeuronDream()
    :   Neuron()
{
    do_init();
};

NeuronDream::~NeuronDream()
{
};

void NeuronDream::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.dream"));
}

int32_t NeuronDream::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronDream::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronDream::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

int32_t NeuronDream::init()
{
    int32_t err = 0;
    return err;
}

void NeuronDream::destroy()
{
}

NeuronSptr NeuronDream::object_create()
{
    NeuronSptr neuron(new NeuronDream);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.dream",
    .init           = NeuronDream::init,
    .destroy        = NeuronDream::destroy,
    .object_create  = NeuronDream::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

