/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <ipnoise-common/rand.hpp>

#include "libxml2/domDocument.h"
#include "objects/ipnoiseObject.h"
#include "objects/serverObject.h"

#include "objects/packetObject.h"

PacketObject * PacketObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new PacketObject(a_node, a_doc, a_tagname);
}

PacketObject::PacketObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

PacketObject::~PacketObject()
{
};

void PacketObject::shutdown()
{
}

int32_t PacketObject::autorun()
{
    int32_t err         = 0;
    int32_t priveleges  = 0;
    string  packet_id;

    priveleges = getDocument()->getPriveleges();
    if (!(priveleges & DomDocument::USE_SYS_OBJECTS)){
        PERROR_OBJ(this, "autorun for '%s' element,"
            " document have not"
            " USE_SYS_OBJECTS priveleges, dump below:\n'%s'\n",
            getTagName().c_str(),
            dump().c_str()
        );
        goto fail;
    }

    // create elements
    getCreateReceiversObject();
    getCreateIPNoiseElement();
    getCreateEventsElement();
    getCreateCommandsElement();

    // setup timer
    setTimer("+1");

out:
    return err;

fail:
    if (err > 0){
        err = -1;
    }
    goto out;
}

void PacketObject::timer()
{
    // request remove packet
    if (isDelivered()){
        setExpired("+1");
        goto out;
    }

    xmit();

    setTimer("+1");

out:
    return;
}

ssize_t PacketObject::xmit()
{
    ssize_t wrote = -1;

    PacketsObject           *packets_obj            = NULL;
    PacketReceiversObject   *packet_receivers_obj   = NULL;

    vector<PacketReceiverObject *>            receivers;
    vector<PacketReceiverObject *>::iterator  receivers_it;

    vector<PacketObject *>              packets;
    vector<PacketObject *>::iterator    packets_it;

    // search packets
    packets_obj = (PacketsObject *)getParentNodeByTagName(
        "ipn_packets"
    );

    // check what all early packets delivered
    packets_obj->getPackets(packets);
    for (packets_it = packets.begin();
        packets_it != packets.end();
        packets_it++)
    {
        PacketObject *cur_packet = *packets_it;
        if ((void *)this == (void *)cur_packet){
            break;
        }
        if (not cur_packet->isDelivered()){
            // prev packet yet not delivered
            goto out;
        }
    }

    // search receivers
    receivers.clear();
    packet_receivers_obj = getCreateReceiversObject();
    packet_receivers_obj->getReceivers(receivers);

    for (receivers_it = receivers.begin();
        receivers_it != receivers.end();
        receivers_it++)
    {
        PacketReceiverObject *receiver = *receivers_it;
        ssize_t              cur_wrote = -1;

        if (receiver->isDelivered()){
            // skip if delivered
            continue;
        }
        // send packet via receiver
        cur_wrote = receiver->xmit();
        if (cur_wrote > ssize_t(0)){
            wrote = cur_wrote;
        }
    }

out:
    return wrote;
}

// ---------------- ipnoise ---------------

DomElement * PacketObject::getIPNoiseElement()
{
    DomElement *el = NULL;
    el = getElementByTagName("ipnoise");
    return el;
}

void PacketObject::setIPNoiseElement(DomElement *a_el)
{
    DomElement *ipnoise_el = NULL;

    if ("ipnoise" != a_el->getTagName()){
        PERROR("Invalid tag name: '%s' must be 'ipnoise'\n",
            a_el->getTagName().c_str()
        );
        goto fail;
    }

    ipnoise_el = getIPNoiseElement();
    if (ipnoise_el){
        // delete current
        DomDocument::deleteElementSafe(ipnoise_el);
    }

    appendChildSafe(a_el);

out:
    return;
fail:
    goto out;
}

DomElement * PacketObject::getCreateIPNoiseElement()
{
    DomElement *el = NULL;
    el = getIPNoiseElement();
    if (not el){
        el = getDocument()->createElement("ipnoise");
        appendChildSafe(el);
    }
    return el;
}

// ---------------- receivers -------------

PacketReceiversObject * PacketObject::getReceiversObject()
{
    PacketReceiversObject *el = NULL;
    el = (PacketReceiversObject *)getElementByTagName("ipn_packet_receivers");
    return el;
}

PacketReceiversObject * PacketObject::getCreateReceiversObject()
{
    PacketReceiversObject *el = NULL;

    el = getReceiversObject();
    if (not el){
        el = getDocument()->createElement<PacketReceiversObject>(
            "ipn_packet_receivers"
        );
        appendChildSafe(el);
    }
    return el;
}

PacketReceiverObject * PacketObject::createReceiver()
{
    PacketReceiverObject    *receiver   = NULL;
    PacketReceiversObject   *receivers  = NULL;

    receivers = getCreateReceiversObject();
    if (not receivers){
        PERROR_OBJ(this, "Cannot create 'ipn_packet_receivers' object\n");
        goto fail;
    }

    receiver = getDocument()->createElement<PacketReceiverObject>(
        "ipn_packet_receiver"
    );
    receivers->appendChildSafe(receiver);

out:
    return receiver;
fail:
    goto out;
}

// ---------------- events ----------------

DomElement * PacketObject::getEventsElement()
{
    DomElement *ipnoise_el  = NULL;
    DomElement *el          = NULL;

    ipnoise_el = getIPNoiseElement();
    if (not ipnoise_el){
        goto out;
    }

    el = ipnoise_el->getElementByTagName("events");

out:
    return el;
}

void PacketObject::setEventsElement(DomElement *a_el)
{
    DomElement *ipnoise_el  = NULL;
    DomElement *events_el   = NULL;

    if ("events" != a_el->getTagName()){
        PERROR("Invalid tag name: '%s' must be 'events'\n",
            a_el->getTagName().c_str()
        );
        goto fail;
    }

    ipnoise_el = getCreateIPNoiseElement();
    if (not ipnoise_el){
        PERROR("Cannot create 'ipnoise' element\n");
        goto fail;
    }

    events_el = getEventsElement();
    if (events_el){
        // delete current
        DomDocument::deleteElementSafe(events_el);
    }

    ipnoise_el->appendChildSafe(a_el);

out:
    return;
fail:
    goto out;
}

DomElement * PacketObject::getCreateEventsElement()
{
    DomElement *el = NULL;

    el = getEventsElement();
    if (not el){
        el = getDocument()->createElement("events");
        setEventsElement(el);
    }
    return el;
}

// ---------------- commands --------------

DomElement * PacketObject::getCommandsElement()
{
    DomElement *ipnoise_el  = NULL;
    DomElement *el          = NULL;

    ipnoise_el = getIPNoiseElement();
    if (not ipnoise_el){
        goto out;
    }

    el = ipnoise_el->getElementByTagName("commands");

out:
    return el;
}

void PacketObject::setCommandsElement(DomElement *a_el)
{
    DomElement *ipnoise_el  = NULL;
    DomElement *commands_el   = NULL;

    if ("commands" != a_el->getTagName()){
        PERROR("Invalid tag name: '%s' must be 'commands'\n",
            a_el->getTagName().c_str()
        );
        goto fail;
    }

    ipnoise_el = getCreateIPNoiseElement();
    if (not ipnoise_el){
        PERROR("Cannot create 'ipnoise' element\n");
        goto fail;
    }

    commands_el = getCommandsElement();
    if (commands_el){
        // delete current
        DomDocument::deleteElementSafe(commands_el);
    }

    ipnoise_el->appendChildSafe(a_el);

out:
    return;
fail:
    goto out;
}

DomElement * PacketObject::getCreateCommandsElement()
{
    DomElement *el = NULL;

    el = getCommandsElement();
    if (not el){
        el = getDocument()->createElement("commands");
        setCommandsElement(el);
    }
    return el;
}

PacketReceiverObject * PacketObject::getReceiver(
    const string &a_sessid)
{
    PacketReceiversObject *receivers_obj    = NULL;
    PacketReceiverObject  *receiver         = NULL;

    receivers_obj = getCreateReceiversObject();
    receiver      = receivers_obj->getReceiver(a_sessid);

    return receiver;
}

PacketReceiverObject * PacketObject::getCreateReceiver(
    const string &a_sessid)
{
    PacketReceiversObject *receivers_obj    = NULL;
    PacketReceiverObject  *receiver         = NULL;

    receivers_obj = getCreateReceiversObject();
    receiver      = receivers_obj->getCreateReceiver(a_sessid);

    return receiver;
}

void PacketObject::getReceivers(
    vector<PacketReceiverObject *> &a_out)
{
    PacketReceiversObject *receivers_obj = NULL;
    receivers_obj = getCreateReceiversObject();
    receivers_obj->getReceivers(a_out);
}

int32_t PacketObject::isDelivered(
    const string &a_sessid)
{
    int32_t     ret             = 0;
    int32_t     all_delivered   = 1;
    int32_t     have_delivered  = 0;
    ItemObject  *item           = NULL;
    int32_t     is_ipnoise_api  = 0;

    vector<PacketReceiverObject *>              receivers;
    vector<PacketReceiverObject *>::iterator    receivers_it;

    // get item
    item = (ItemObject *)getParentNodeByTagName("ipn_item");
    if (not item){
        goto out;
    }

    // detect api
    is_ipnoise_api = item->isSelfItem() ? 0 : 1;

    // get receivers
    getReceivers(receivers);

    for (receivers_it = receivers.begin();
        receivers_it != receivers.end();
        receivers_it++)
    {
        PacketReceiverObject *cur_receiver = *receivers_it;
        string cur_sessid = cur_receiver->getSessId();

        if (    not a_sessid.empty()
            &&  a_sessid != cur_sessid)
        {
            // skip not requested sessions
            continue;
        }
        if (not cur_receiver->isDelivered()){
            all_delivered = 0;
        } else {
            have_delivered = 1;
        }
        if (    not all_delivered
            &&  have_delivered)
        {
            break;
        }
    }

    if (is_ipnoise_api){
        // for ipnoise items, exactly one receiver must be reached
        ret = have_delivered;
    } else {
        // for client items, all receivers must be reached
        ret = all_delivered;
    }

out:
    return ret;
}

