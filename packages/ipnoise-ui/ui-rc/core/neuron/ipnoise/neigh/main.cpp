#include "core/neuron/user/main.hpp"
#include "utils.hpp"

#include "core/neuron/ipnoise/neigh/main.hpp"

NeuronIPNoiseNeigh::NeuronIPNoiseNeigh()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronIPNoiseNeigh::~NeuronIPNoiseNeigh()
{
};

void NeuronIPNoiseNeigh::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.ipnoise.neigh"));
    initDstHuid(PROP_STRING());
    initSrcHuid(PROP_STRING());
    initLLAddr(PROP_STRING());
    initDevName(PROP_STRING());

    // register API
}

int32_t NeuronIPNoiseNeigh::do_autorun()
{
    int32_t err = 0;
    return err;
}

void NeuronIPNoiseNeigh::getAllProps(
    PropMapSptr a_props)
{
    SAVE_PROP("dst_huid",  getDstHuid);
    SAVE_PROP("src_huid",  getSrcHuid);
    SAVE_PROP("lladdr",    getLLAddr);
    SAVE_PROP("dev_name",  getDevName);

    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronIPNoiseNeigh::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    string  dst_huid;
    string  src_huid;
    string  lladdr;
    string  dev_name;

    // dst_huid
    if (a_obj.hasField("dst_huid")){
        dst_huid = a_obj.getStringField("dst_huid");
        initDstHuid(PROP_STRING(dst_huid));
    }

    // src_huid
    if (a_obj.hasField("src_huid")){
        src_huid = a_obj.getStringField("src_huid");
        initSrcHuid(PROP_STRING(src_huid));
    }

    // lladdr
    if (a_obj.hasField("lladdr")){
        lladdr = a_obj.getStringField("lladdr");
        initLLAddr(PROP_STRING(lladdr));
    }

    // dev_name
    if (a_obj.hasField("dev_name")){
        dev_name = a_obj.getStringField("dev_name");
        initDevName(PROP_STRING(dev_name));
    }

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

int32_t NeuronIPNoiseNeigh::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t     res, err = -1;

    PropSptr    prop_dst_huid;
    PropSptr    prop_src_huid;
    PropSptr    prop_lladdr;
    PropSptr    prop_dev_name;

    // search dst_huid
    prop_dst_huid = a_props->get("dst_huid");
    if (prop_dst_huid){
        setDstHuid(PROP_STRING(
            prop_dst_huid->toString()
        ));
    }

    // search src_huid
    prop_src_huid = a_props->get("src_huid");
    if (prop_src_huid){
        setSrcHuid(PROP_STRING(
            prop_src_huid->toString()
        ));
    }

    // search lladdr
    prop_lladdr = a_props->get("lladdr");
    if (prop_lladdr){
        setLLAddr(PROP_STRING(
            prop_lladdr->toString()
        ));
    }

    // search dev_name
    prop_dev_name = a_props->get("dev_name");
    if (prop_dev_name){
        setDevName(PROP_STRING(
            prop_dev_name->toString()
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

string NeuronIPNoiseNeigh::getLinkAddress()
{
    return getLLAddr()->toString();
}

string NeuronIPNoiseNeigh::getLinkName()
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

void NeuronIPNoiseNeigh::timer()
{
/*
    int32_t recheck_fail_max    = getRecheckFailMax();
    int32_t recheck_after       = getRecheckAfter();
    int32_t recheck_time        = getRecheckTime();
    int32_t down_time           = getDownTime();

    recheck_after--;
    if (recheck_after <= 0){
        vector<DomElement *> res;
        getElementsByTagName("ipn_neigh_checkings", res);
        vector<>
        if (recheck_fail_max <= int32_t(res.size())){
            // reset src huid (going to slow path)
            setSrcHuid("");
            // max fail count reached
            deleteChildsSafe();
            // update recheck_time
            recheck_time *= 2;
            setRecheckTime(recheck_time);
            // update recheck_after
            recheck_after = recheck_time;
            // update down time
            down_time += recheck_time * res.size();
            setDownTime(down_time);
            // set neigh down
            setStateDown();
        } else {
            // reset recheck_after
            recheck_after = recheck_time;
            // check neigh
            doCheck();
        }
    }

    // update recheck after
    setRecheckAfter(recheck_after);

    // setup timer
    setTimer("+1");
*/
}

/*
void NeuronIPNoiseNeigh::doCheck()
{
    vector<UserObject *>            users;
    vector<UserObject *>::iterator  users_it;
    map<const string, int>          src_huids;

    string src_huid = getSrcHuid();

    // maybe fast path?
    if (src_huid.size()){
        // yea! we known working source huid
        doCheck(src_huid);
        goto out;
    }

    // we have not source huid :(
    // so send searchRoute commands from all local users names
    server->getCreateUsersObject()->getUsers(users);
    for (users_it = users.begin();
        users_it != users.end();
        users_it++)
    {
        // start checking
        UserObject  *user   = *users_it;
        string      huid    = user->getHuid();

        // skip multicast user
        if (MULTICAST_USER_HUID == huid){
            continue;
        }

        // store huid
        src_huids[huid] = 1;
    }

    // check
    doCheck(src_huids);


out:
    return;
}

void NeuronIPNoiseNeigh::doCheck(
    const string &a_src_huid)
{
    map<const string, int> src_huids;
    src_huids[a_src_huid] = 1;
    doCheck(src_huids);
}

void NeuronIPNoiseNeigh::doCheck(
    map<const string, int> &a_src_huids)
{
    map<const string, int>::iterator src_huids_it;

    string dst_huid = getDstHuid();
    string lladdr   = getLLAddr();
    string dev      = getDev();

    DomElement *ipn_neigh_checkings = NULL;

    if (not a_src_huids.size()){
        goto out;
    }

    ipn_neigh_checkings = getDocument()->createElement(
        "ipn_neigh_checkings"
    );
    appendChildSafe(ipn_neigh_checkings);

    for (src_huids_it = a_src_huids.begin();
        src_huids_it != a_src_huids.end();
        src_huids_it++)
    {
        string          src_huid    = src_huids_it->first;
        SkBuffObject    *skb        = NULL;
        DomElement      *ipnoise    = NULL;
        DomElement      *command    = NULL;
        DomElement      *commands   = NULL;
        DomElement      *out_dom    = NULL;

        skb = getDocument()->createElement<SkBuffObject>(
            "ipn_skbuff"
        );
        skb->setFrom(src_huid);
        skb->setTo(dst_huid);
        skb->setToLL(lladdr);
        skb->setOutDevName(dev);

        // create command
        out_dom = skb->getOutDom(1);

        ipnoise = skb->getDocument()->createElement("ipnoise");
        out_dom->appendChildSafe(ipnoise);

        command = skb->getDocument()->createElement("command");
        command->setAttributeSafe("type", "searchRoute");
        commands = skb->getDocument()->createElement("commands");
        commands->appendChildSafe(command);
        ipnoise->appendChildSafe(commands);

        // append "up" neighs
        NetCommandObject::appendNeighsUp(command);

        // create "ipn_neigh_checking" item
        {
            string      checking_id  = getRand(20);
            DomElement  *checking    = NULL;

            checking = getDocument()->createElement(
                "ipn_neigh_checking"
            );

            // delete element after exit
            checking->setExpired("0");

            // set checking ID
            checking->setAttributeSafe("id",        checking_id);
            checking->setAttributeSafe("src_huid",  src_huid);
            command->setAttributeSafe("id",         checking_id);

            // add checking item
            ipn_neigh_checkings->appendChildSafe(checking);
        }

        // send packet,
        // NOTE: packet will be deleted after sendPacket call
        // tx packet (emitSignal will create clone of element)
        skb->prepare();
        getDocument()->emitSignalAndDelObj(
            "packet_outcoming",
            skb
        );
    }

out:
    return;
}
*/

// ---------------- module ----------------

int32_t NeuronIPNoiseNeigh::init()
{
    int32_t err = 0;
    return err;
}

void NeuronIPNoiseNeigh::destroy()
{
}

NeuronSptr NeuronIPNoiseNeigh::object_create()
{
    NeuronSptr neuron(new NeuronIPNoiseNeigh);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.ipnoise.neigh",
    .init           = NeuronIPNoiseNeigh::init,
    .destroy        = NeuronIPNoiseNeigh::destroy,
    .object_create  = NeuronIPNoiseNeigh::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

