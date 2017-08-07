/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/ipnoiseObject.h"
#include "objects/serverObject.h"

#include "objects/packetsObject.h"

PacketsObject * PacketsObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new PacketsObject(a_node, a_doc, a_tagname);
}

PacketsObject::PacketsObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

PacketsObject::~PacketsObject()
{
};

void PacketsObject::shutdown()
{
}

int32_t PacketsObject::autorun()
{
    int32_t err         = 0;
    int32_t priveleges  = 0;

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

out:
    return err;

fail:
    if (err > 0){
        err = -1;
    }
    goto out;
}

void PacketsObject::timer()
{
}

void PacketsObject::getPackets(
    vector<PacketObject *>  &a_out)
{
    vector<DomElement *>            res;
    vector<DomElement *>::iterator  res_it;

    getElementsByTagName("ipn_packet", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        PacketObject *packet = NULL;
        packet = (PacketObject *)*res_it;
        a_out.push_back(packet);
    }
}

PacketObject * PacketsObject::getFirstPacket()
{
    PacketObject *packet = NULL;
    packet = (PacketObject *)getElementByTagName("ipn_packet");
    return packet;
}

PacketObject * PacketsObject::getPacket(
    const string &a_packetid)
{
    char            buffer[512] = { 0x00 };
    PacketObject    *packet     = NULL;

    snprintf(buffer, sizeof(buffer),
        "ipn_packet[@packet_id=\"%s\"]",
        a_packetid.c_str()
    );
    packet = (PacketObject *)getElementByXpath(buffer);
    return packet;
}

PacketObject * PacketsObject::getPacket(
    const uint32_t &a_packetid)
{
    char            buffer[512] = { 0x00 };
    PacketObject    *packet     = NULL;

    snprintf(buffer, sizeof(buffer),
        "%x",
        a_packetid
    );
    packet = getPacket(buffer);
    return packet;
}

PacketObject * PacketsObject::createPacket(
    DomElement *a_packet_data)
{
    vector <DomElement *>           commands;
    vector <DomElement *>           events;
    vector <DomElement *>           res;
    vector <DomElement *>::iterator it;

    PacketObject    *packet             = NULL;
    DomElement      *events_el          = NULL;
    DomElement      *commands_el        = NULL;

    // create new packet
    packet = getDocument()->createElement<PacketObject>(
        "ipn_packet"
    );
    appendChildSafe(packet);

    if ("ipnoise" == a_packet_data->getTagName()){
        // fast path
        packet->setIPNoiseElement(a_packet_data->clone());
        goto out;
    }

    // create packet elements
    events_el   = packet->getCreateEventsElement();
    commands_el = packet->getCreateCommandsElement();

    // search commands
    if ("command" == a_packet_data->getTagName()){
        commands.push_back(a_packet_data);
    }
    res.clear();
    a_packet_data->getElementsByXpath("//command", res);
    for (it = res.begin();
        it != res.end();
        it++)
    {
        commands.push_back(*it);
    }

    // search events
    if ("event" == a_packet_data->getTagName()){
        events.push_back(a_packet_data);
    }
    res.clear();
    a_packet_data->getElementsByXpath("//event", res);
    for (it = res.begin();
        it != res.end();
        it++)
    {
        events.push_back(*it);
    }

    // add commands
    for (it = commands.begin();
        it != commands.end();
        it++)
    {
        DomElement *command = (*it)->clone();
        commands_el->appendChildSafe(command);
    }

    // add events
    for (it = events.begin();
        it != events.end();
        it++)
    {
        DomElement *event = (*it)->clone();
        events_el->appendChildSafe(event);
    }

out:
    return packet;
}

PacketObject * PacketsObject::queue(
    DomElement *a_packet_data)
{
    ItemObject      *item           = NULL;
    PacketObject    *packet         = NULL;
    SessionsObject  *sessions_obj   = NULL;

    vector<SessionObject *>             sessions;
    vector<SessionObject *>::iterator   sessions_it;

    packet = createPacket(a_packet_data);
    if (not packet){
        PERROR_OBJ(this, "Cannot create packet for: '%s'\n",
            a_packet_data->serialize().c_str()
        );
        goto fail;
    }

    // search item
    item = (ItemObject *)getParentNodeByTagName("ipn_item");
    if (not item){
        PERROR("Cannot get 'ipn_item' element\n");
        goto fail;
    }

    // search sessions object
    sessions_obj = (SessionsObject *)getParentNodeByTagName(
        "ipn_sessions"
    );
    if (not sessions_obj){
        PERROR("Cannot get 'sessions' object\n");
        goto fail;
    }

    // search sessions
    sessions_obj->getSessions(sessions);

    // create receivers
    for (sessions_it = sessions.begin();
        sessions_it != sessions.end();
        sessions_it++)
    {
        SessionObject        *cur_session  = *sessions_it;
        PacketReceiverObject *cur_receiver = NULL;

        if ("ipnoise" == cur_session->getApi()){
            // for ipnoise, send only in outcoming session
            if ("outcoming" != cur_session->getType()){
                continue;
            }
        }

        cur_receiver = packet->createReceiver();
        cur_receiver->setSessId(cur_session->getSessId());
        cur_receiver->xmit();
    }

out:
    return packet;
fail:
    goto out;
}

void PacketsObject::clear()
{
    deleteChildsSafe();
}

