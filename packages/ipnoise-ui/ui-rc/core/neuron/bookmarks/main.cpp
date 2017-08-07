//#include "module.hpp"
#include "core/neuron/param/main.hpp"

#include "core/neuron/bookmarks/main.hpp"

NeuronBookmarks::NeuronBookmarks()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronBookmarks::~NeuronBookmarks()
{
};

void NeuronBookmarks::do_init()
{
    // init props
    initType(PROP_NEURON_TYPE("core.neuron.bookmarks"));
}

int32_t NeuronBookmarks::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronBookmarks::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronBookmarks::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = -1;

    err = SUPER_CLASS::parseBSON(a_obj);
    if (err){
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

int32_t NeuronBookmarks::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t         res, err = -1;

    PropSptr        prop_password;
    NeuronParamSptr param_password;

    // get password
    prop_password = a_props->get("password");
    if (prop_password){
        param_password = getCreateParam("password");
        param_password->setValue(
            dynamic_pointer_cast<PropString>(prop_password)
        );
        param_password->save();
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

int32_t NeuronBookmarks::init()
{
    int32_t err = 0;
    return err;
}

void NeuronBookmarks::destroy()
{
}

NeuronSptr NeuronBookmarks::object_create()
{
    NeuronSptr neuron(new NeuronBookmarks);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.bookmarks",
    .init           = NeuronBookmarks::init,
    .destroy        = NeuronBookmarks::destroy,
    .object_create  = NeuronBookmarks::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

