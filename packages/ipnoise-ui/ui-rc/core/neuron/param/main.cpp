#include "core/neuron/param/main.hpp"

NeuronParam::NeuronParam()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronParam::~NeuronParam()
{
};

void NeuronParam::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.param"));
    initValue(PROP_STRING());

    // register API
}

int32_t NeuronParam::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronParam::getAllProps(
    PropMapSptr a_props)
{
    SAVE_PROP("value",  getValue);

    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronParam::save()
{
    PropMapSptr   a_props(new PropMap);
    getAllProps(a_props);
    return saveProps(a_props);
}

int32_t NeuronParam::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    string  value;

    // value
    if (a_obj.hasField("value")){
        value = a_obj.getStringField("value");
        initValue(PropStringSptr(
            new PropString(value)
        ));
    }

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronParam::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t     res, err = -1;
    PropSptr    prop_value;

    // search value
    prop_value = a_props->get("value");
    if (prop_value){
        setValue(PROP_STRING(
            prop_value->toString()
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

int32_t NeuronParam::init()
{
    int32_t err = 0;
    return err;
}

void NeuronParam::destroy()
{
}

NeuronSptr NeuronParam::object_create()
{
    NeuronSptr neuron(new NeuronParam);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.param",
    .init           = NeuronParam::init,
    .destroy        = NeuronParam::destroy,
    .object_create  = NeuronParam::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

