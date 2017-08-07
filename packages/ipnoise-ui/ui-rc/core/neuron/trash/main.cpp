#include "thread/db.hpp"
#include "thread/http.hpp"

//#include "neuron/user.hpp"
//#include "neuron/god.hpp"
//#include "neuron/session.hpp"
//#include "neuron/param.hpp"

#include "core/neuron/trash/main.hpp"

NeuronTrash::NeuronTrash()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronTrash::~NeuronTrash()
{
};

void NeuronTrash::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.trash"));
    initId(PROP_NEURON_ID("trash"));
}

int32_t NeuronTrash::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronTrash::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronTrash::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

int32_t NeuronTrash::init()
{
    int32_t err = 0;
    return err;
}

void NeuronTrash::destroy()
{
}

NeuronSptr NeuronTrash::object_create()
{
    NeuronSptr neuron(new NeuronTrash);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.trash",
    .init           = NeuronTrash::init,
    .destroy        = NeuronTrash::destroy,
    .object_create  = NeuronTrash::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

