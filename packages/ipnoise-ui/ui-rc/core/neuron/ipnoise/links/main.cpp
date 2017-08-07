#include "network.hpp"

#include "core/neuron/ipnoise/links/main.hpp"

static QTimer  *s_timer = NULL;
extern Network *g_network;

NeuronIPNoiseLinks::NeuronIPNoiseLinks()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronIPNoiseLinks::~NeuronIPNoiseLinks()
{
};

void NeuronIPNoiseLinks::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.ipnoise.links"));
    initId(PROP_NEURON_ID("core.ipnoise.links"));
    initHuid(PROP_STRING());
}

int32_t NeuronIPNoiseLinks::do_autorun()
{
    int32_t err = 0;

    // init timer
    if (!s_timer){
        s_timer = new QTimer(0);
        connect(
            s_timer,    SIGNAL(timeout()),
            this,       SLOT(timerSlot())
        );
        s_timer->start(1000);
    }

    // QTimer::singleShot(1, this, SLOT(do_autorun()));

    return err;
}

void NeuronIPNoiseLinks::getAllProps(
    PropMapSptr a_props)
{
    SAVE_PROP("huid",  getHuid);

    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronIPNoiseLinks::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    string  huid;

    // huid
    if (a_obj.hasField("huid")){
        huid = a_obj.getStringField("huid");
        initHuid(PROP_STRING(
            huid
        ));
    }

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

NeuronIPNoiseLinkSptr NeuronIPNoiseLinks::createIPNoiseLink()
{
    NeuronIPNoiseLinkSptr    neuron_link;

    neuron_link = getNetwork()
        ->createNeuron<NeuronIPNoiseLink>();
    LINK_AND_SAVE(this, neuron_link);

    return neuron_link;
}

void NeuronIPNoiseLinks::apiAddNewLink(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronIPNoiseLinks      *links = (NeuronIPNoiseLinks *)a_neuron;
    PropMapSptr             answer_params;
    NeuronIPNoiseLinkSptr   neuron_link;
    PropSptr                link_id;

    // TODO check perms here

    // create link
    neuron_link = links->createIPNoiseLink();
    link_id     = neuron_link->getId();

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();
    answer_params->add("object_id", link_id);

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "link was created successfully"
    );
}

int32_t NeuronIPNoiseLinks::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t     res, err = -1;
    PropSptr    prop_huid;

    // search huid
    prop_huid = a_props->get("huid");
    if (prop_huid){
        setHuid(PROP_STRING(
            prop_huid->toString()
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

void NeuronIPNoiseLinks::timerSlot()
{
    vector<InterfaceInfoSptr>               ifs;
    vector<InterfaceInfoSptr>::iterator     ifs_it;

    vector<NeuronIPNoiseLinkSptr>           links;
    vector<NeuronIPNoiseLinkSptr>::iterator links_it;

    PropTimeSptr cur_time = PropTime::now();

    // get interfaces info
    NeuronIPNoiseLink::getInterfacesInfo(ifs);

    // add new
    for (ifs_it = ifs.begin();
        ifs_it != ifs.end();
        ifs_it++)
    {
        InterfaceInfoSptr       info = *ifs_it;
        NeuronIPNoiseLinkSptr   dev;
        string                  dev_name  = info->getName();
        int32_t                 dev_index = info->getIndex();

        PDEBUG(50, "check interface:"
            " dev_name: '%s',"
            " dev_index: '%d'\n",
            dev_name.c_str(),
            dev_index
        );

        dev = getNeigh<NeuronIPNoiseLink>(dev_name);
        if (!dev){
            dev = createNeigh<NeuronIPNoiseLink>(dev_name);
            dev->setupLink(dev_name, dev_index);
            PDEBUG(10, "device: '%s'"
                " (index: '%d') was added"
                " for watching\n",
                dev_name.c_str(),
                dev_index
            );
        }
        dev->setCheckTime(cur_time);
        dev->save();

        //dev->send(NeuronIPNoiseSkbSptr());
    }

    // remove old
    getNeighs<NeuronIPNoiseLink>(links);
    for (links_it = links.begin();
        links_it != links.end();
        links_it++)
    {
        NeuronIPNoiseLinkSptr cur_link = *links_it;
        PropTimeSptr          cur_link_time;
        PropStringSptr        cur_link_name;
        PropIntSptr           cur_link_index;

        cur_link_time  = cur_link->getCheckTime();
        cur_link_name  = cur_link->getName();
        cur_link_index = cur_link->getIfIndex();

        PWARN("cur link:\n"
            "  dev name:   '%s'\n"
            "  dev index:  '%d'\n"
            "  check time: '%s'\n"
            "  cur time:   '%s'\n",
            cur_link_name->toString().c_str(),
            cur_link_index->toInt(),
            cur_link_time->toString().c_str(),
            cur_time->toString().c_str()
        );

        if (*cur_link_time != *cur_time){
            PDEBUG(10, "device: '%s' was removed"
                " from watching\n",
                cur_link_name->toString().c_str()
            );
            cur_link->remove();
        }
    }
}

// ---------------- module ----------------

static ModuleInfo info = {
    .type           = "core.neuron.ipnoise.links",
    .init           = NeuronIPNoiseLinks::init,
    .destroy        = NeuronIPNoiseLinks::destroy,
    .object_create  = NeuronIPNoiseLinks::object_create,
    .flags          = 0
};

int32_t NeuronIPNoiseLinks::init()
{
    int32_t err = 0;

    // should never happen, but sometimes shit happens
    if (info.neuron){
        PWARN("info.neuron not empty\n");
        info.neuron = NeuronIPNoiseLinksSptr();
    }

    // create static neuron
    info.neuron = g_network
        ->getDbThread()
        ->getCreateNeuronById(
            PROP_NEURON_ID("core.ipnoise.links"),
            PROP_NEURON_TYPE("core.neuron.ipnoise.links")
        );

    return err;
}

void NeuronIPNoiseLinks::destroy()
{
}

NeuronSptr NeuronIPNoiseLinks::object_create()
{
    NeuronSptr neuron(new NeuronIPNoiseLinks);
    return neuron;
}

REGISTER_MODULE(info);

