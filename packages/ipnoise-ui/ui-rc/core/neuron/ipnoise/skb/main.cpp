#include "core/neuron/user/main.hpp"
#include "utils.hpp"

#include "core/neuron/ipnoise/skb/main.hpp"

NeuronIPNoiseSkb::NeuronIPNoiseSkb()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronIPNoiseSkb::~NeuronIPNoiseSkb()
{
};

void NeuronIPNoiseSkb::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.ipnoise.skb"));
    initInLinkName(PROP_STRING());
    initInLinkIndex(PROP_INT());
    initInContent(PROP_STRING());
    initFrom(PROP_STRING());
    initTo(PROP_STRING());

    // register API
}

int32_t NeuronIPNoiseSkb::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronIPNoiseSkb::getAllProps(
    PropMapSptr a_props)
{
/*
    SAVE_PROP("dst_huid",  getDstHuid);
    SAVE_PROP("src_huid",  getSrcHuid);
    SAVE_PROP("lladdr",    getLLAddr);
    SAVE_PROP("dev_name",  getDevName);
*/

    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronIPNoiseSkb::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
/*
    string  dst_huid;
    string  src_huid;
    string  lladdr;
    string  dev_name;

    // dst_huid
    if (a_obj.hasField("dst_huid")){
        dst_huid = a_obj.getStringField("dst_huid");
        setDstHuid(PROP_STRING(
            dst_huid
        ));
    }

    // src_huid
    if (a_obj.hasField("src_huid")){
        src_huid = a_obj.getStringField("src_huid");
        setSrcHuid(PROP_STRING(
            src_huid
        ));
    }

    // lladdr
    if (a_obj.hasField("lladdr")){
        lladdr = a_obj.getStringField("lladdr");
        setLLAddr(PROP_STRING(
            lladdr
        ));
    }

    // dev_name
    if (a_obj.hasField("dev_name")){
        dev_name = a_obj.getStringField("dev_name");
        setDevName(PROP_STRING(
            dev_name
        ));
    }
*/

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronIPNoiseSkb::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t res, err = -1;

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

/*
string NeuronIPNoiseSkb::getLinkAddress()
{
    return getLLAddr()->toString();
}

string NeuronIPNoiseSkb::getLinkName()
{
    string dev = getDevName()->toString();
    string ret = dev;

    if ("udp_v4_" == dev.substr(0, 7)){
        ret = "udp_v4";
    } else if ("tcp_v4_" == dev.substr(0, 7)){
        ret = "tcp_v4";
    } else if ("icmp_v4_" == dev.substr(0, 8)){
        ret = "icmp_v4";
    } else {
        ret = "internal_error";
    }

    return ret;
}
*/

// ---------------- module ----------------

int32_t NeuronIPNoiseSkb::init()
{
    int32_t err = 0;
    return err;
}

void NeuronIPNoiseSkb::destroy()
{
}

NeuronSptr NeuronIPNoiseSkb::object_create()
{
    NeuronSptr neuron(new NeuronIPNoiseSkb);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.ipnoise.skb",
    .init           = NeuronIPNoiseSkb::init,
    .destroy        = NeuronIPNoiseSkb::destroy,
    .object_create  = NeuronIPNoiseSkb::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

