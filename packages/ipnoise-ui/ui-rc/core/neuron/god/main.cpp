#include "core/neuron/god/main.hpp"

NeuronGod::NeuronGod()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronGod::~NeuronGod()
{
};

void NeuronGod::do_init()
{
    // props
    initType(PROP_NEURON_TYPE("core.neuron.god"));
    initId(PROP_NEURON_ID("core.users.god"));
}

int32_t NeuronGod::do_autorun()
{
    int32_t err = 0;

    NeuronParamSptr param_login;
    NeuronParamSptr param_passwd;

    // params
    param_login = getParam<NeuronParam>("login");
    if (!param_login){
        param_login = createParam<NeuronParam>("login");
        param_login->setValue(PROP_STRING(GOD_LOGIN));
        LINK_AND_SAVE(this, param_login);
    }
    param_passwd = getParam<NeuronParam>("password");
    if (!param_passwd){
        param_passwd = createParam<NeuronParam>("password");
        param_passwd->setValue(PROP_STRING(GOD_PASSWORD));
        LINK_AND_SAVE(this, param_passwd);
    }

    return  err;
}

void NeuronGod::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronGod::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

// ---------------- module ----------------

int32_t NeuronGod::init()
{
    int32_t err = 0;
    return err;
}

void NeuronGod::destroy()
{
}

NeuronSptr NeuronGod::object_create()
{
    NeuronSptr neuron(new NeuronGod);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.god",
    .init           = NeuronGod::init,
    .destroy        = NeuronGod::destroy,
    .object_create  = NeuronGod::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

