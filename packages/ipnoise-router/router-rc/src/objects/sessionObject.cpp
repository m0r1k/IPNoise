/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <ipnoise-common/rand.hpp>

#include "libxml2/domDocument.h"
#include "objects/ipnoiseObject.h"
#include "objects/serverObject.h"
#include "objects/usersObject.h"
#include "objects/userObject.h"
#include "objects/netClientObject.h"
#include "objects/clientObject.h"

#include "objects/sessionObject.h"

SessionObject * SessionObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new SessionObject(a_node, a_doc, a_tagname);
}

SessionObject::SessionObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   IoObject(a_node, a_doc, a_tagname)
{
    createSessionId();

    // use "client" api as default
    setApi("client");
    setState("init");

    setAck(0);
};

SessionObject::~SessionObject()
{
};

void SessionObject::shutdown()
{
    setState("closed");
    // do remove us after exit
    // (will be called SessionObject::expired)
    setExpired("0");
}

int32_t SessionObject::autorun()
{
    int err         = 0;
    int priveleges  = 0;

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

    // create "commands" object
    getCreateCommandsObject();

    // create "events" object
    getCreateEventsObject();

    // setup timer
    setTimer("+10");

out:
    return err;

fail:
    if (err > 0){
        err = -1;
    }
    goto out;
}

void SessionObject::timer()
{
    vector<NeighObject *> neighs;

    if ("ipnoise" != getApi()){
        goto out;
    }

    getItem()->getOnlineNeighs(neighs);
    if (neighs.size()){
        // if we have 'up' neighs, request don't delete us
        setExpired("+10");
    }

    setTimer("+10");

out:
    return;
}

// ---------------- ack -----------------------------------

void SessionObject::setAck(const string &a_val)
{
    setAttributeSafe("ack", a_val);
}

void SessionObject::setAck(const uint32_t &a_val)
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer),
        "%x",
        a_val
    );
    setAck(buffer);
}

uint32_t SessionObject::getAckUint()
{
    return getAttributeFromHex("ack");
}

string SessionObject::getAck()
{
    return getAttribute("ack");
}

// ---------------- seq -----------------------------------

void SessionObject::setSeq(const string &a_val)
{
    setAttributeSafe("seq", a_val);
}

void SessionObject::setSeq(const uint32_t &a_val)
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer),
        "%x",
        a_val
    );
    setSeq(buffer);
}

uint32_t SessionObject::getSeqUint()
{
    return getAttributeFromHex("seq");
}

string SessionObject::getSeq()
{
    return getAttribute("seq");
}


// ---------------- commands ------------------------------

DomElement * SessionObject::getCreateCommandsObject()
{
    DomElement *commands = NULL;
    commands = getElementByTagName("ipn_commands");
    if (!commands){
        commands = getDocument()->createElement("ipn_commands");
        appendChildSafe(commands);
    }
    commands->setExpired("0");
    return commands;
}

DomElement * SessionObject::getCreateEventsObject()
{
    DomElement *events = NULL;
    events = getElementByTagName("ipn_events");
    if (!events){
        events = getDocument()->createElement("ipn_events");
        appendChildSafe(events);
    }
    events->setExpired("0");
    return events;
}

string SessionObject::createSessionId()
{
    string sessid = getRand(SESSION_ID_LENGTH_BYTES);

    // store sess ID
    setSessId(sessid);

    return sessid;
}

string SessionObject::getApi()
{
    return getAttribute("api");
}

void SessionObject::setApi(const string &api)
{
    assert(
            ("client"    == api)
        ||  ("ipnoise"   == api)
    );
    setAttributeSafe("api", api);
}

// ---------------- sess id -------------------------------

string SessionObject::getSessId()
{
    return getAttribute("sessid");
}

string SessionObject::getSrcSessId()
{
    return getSessId();
}

void SessionObject::setSrcSessId(const string &a_id)
{
    setSessId(a_id);
}

string SessionObject::getDstSessId()
{
    return getAttribute("dst_sessid");
}

void SessionObject::setDstSessId(const string &a_id)
{
    setAttributeSafe("dst_sessid", a_id);
}

void SessionObject::setSessId(const string &sessid)
{
    setAttributeSafe("sessid", sessid);
}

// ---------------- huid ----------------------------------

void SessionObject::setHuid(string &huid)
{
    setAttributeSafe("huid", huid);
}

string SessionObject::getHuid()
{
    return getAttribute("huid");
}

void SessionObject::setType(const string &type)
{
    assert (
        ("outcoming" == type) || ("incoming" == type)
    );
    setAttributeSafe("type", type);
}

string SessionObject::getType()
{
    return getAttribute("type");
}

void SessionObject::registrySignals()
{
    getDocument()->registrySignal(
        "session_opened",
        "ipn_session"
    );
    getDocument()->registrySignal(
        "session_closed",
        "ipn_session"
    );
    getDocument()->registrySignal(
        "session_opened",
        "ipn_session_client"
    );
    getDocument()->registrySignal(
        "session_closed",
        "ipn_session_client"
    );
    getDocument()->registrySignal(
        "session_opened",
        "ipn_session_item"
    );
    getDocument()->registrySignal(
        "session_closed",
        "ipn_session_item"
    );
    getDocument()->registrySignal(
        "session_opened",
        "ipn_session_lo"
    );
    getDocument()->registrySignal(
        "session_closed",
        "ipn_session_lo"
    );
    getDocument()->registrySignal(
        "session_opened",
        "ipn_session_unknown"
    );
    getDocument()->registrySignal(
        "session_closed",
        "ipn_session_unknown"
    );
}

void SessionObject::setState(const string &a_state)
{
    string cur_state = getState();

    PDEBUG(20, "SessionObject::setState,"
        " sessid: '%s', state: '%s'\n",
        getSessId().c_str(),
        a_state.c_str()
    );

    if (cur_state == a_state){
        // nothing changed
        goto out;
    }

    setAttributeSafe("state", a_state);

    if ("init" == a_state){
    } else if ("closed" == a_state){
        // remove us from packets
        vector<PacketObject *>           packets;
        vector<PacketObject *>::iterator packets_it;

        PacketsObject           *packets_obj    = NULL;
        PacketReceiverObject    *next_receiver  = NULL;
        string                  sessid          = getSessId();

        // get/create packets object
        packets_obj = getSessionsObject()->getCreatePacketsObject();

        packets_obj->getPackets(packets);
        for (packets_it = packets.begin();
            packets_it != packets.end();
            packets_it++)
        {
            PacketObject         *cur_packet = *packets_it;
            PacketReceiverObject *receiver   = NULL;

            receiver = cur_packet->getReceiver(sessid);
            if (receiver){
                // remove receiver
                DomDocument::deleteElement(receiver);
            }

            if (cur_packet->isDelivered()){
                DomDocument::deleteElement(cur_packet);
            }
        }

        // search receiver for send
        next_receiver = getNextReceiver();
        if (next_receiver){
            next_receiver->xmit();
        }

        getDocument()->emitSignal(
            "session_closed",
            this
        );
    } else if ("opening"   == a_state){
    } else if ("opened"    == a_state){
        getDocument()->emitSignal(
            "session_opened",
            this
        );
    } else if ("logging"   == a_state){
    } else if ("logged"    == a_state){
    } else {
        PFATAL("unsupported session state: '%s'\n",
            a_state.c_str()
        );
    }

out:
    return;
}

bool SessionObject::isOnline()
{
    bool    ret     = false;
    string  state   = getState();
    if (    "opened"    == state
        ||  "logged"    == state)
    {
        ret = true;
    }
    return ret;
}

string SessionObject::getState()
{
    return getAttribute("state");
}

ItemObject * SessionObject::getItem()
{
    ItemObject *item = NULL;

    item = (ItemObject *)getParentNodeByTagName(
        "ipn_item"
    );

    return item;
}

SessionsObject * SessionObject::getSessionsObject()
{
    SessionsObject *sessions = NULL;

    sessions = (SessionsObject *)getParentNode();
    assert("ipn_sessions" == sessions->getTagName());
    return sessions;
}

int SessionObject::connect()
{
    int err = 0;

    PFATAL_OBJ(this, "Cannot connect, method not implemented\n");

    return err;
};

PacketReceiverObject * SessionObject::queue(
    const string &a_packet_data)
{
    PacketReceiverObject    *receiver   = NULL;
    DomElement              *element    = NULL;

    if (not a_packet_data.empty()){
        element = getDocument()->createElementFromContent(
            a_packet_data
        );
        // TODO XXX
        // temporary add us to DomModel
        // else getElementsByXpath will not works
        // 20120107 morik@
        appendChildSafe(element);
    }

    if (element){
        receiver = queue(element);
        DomDocument::deleteElement(element);
    }

    return receiver;
}

PacketReceiverObject * SessionObject::queue(
    DomElement  *a_packet_data)
{
    PacketObject            *packet         = NULL;
    PacketsObject           *packets_obj    = NULL;
    PacketReceiverObject    *receiver       = NULL;
    PacketReceiverObject    *next_receiver  = NULL;

    // get packets obj
    packets_obj = getSessionsObject()->getCreatePacketsObject();

    // create packet
    packet = packets_obj->createPacket(a_packet_data);
    if (not packet){
        PERROR_OBJ(this, "Cannot create packet for: '%s'\n",
            a_packet_data->serialize().c_str()
        );
        goto fail;
    }

    receiver = packet->createReceiver();
    receiver->setSessId(getSessId());

    // search receiver for send
    next_receiver = getNextReceiver();
    if (next_receiver){
        next_receiver->xmit();
    }

out:
    return receiver;

fail:
    goto out;
}

void SessionObject::clearQueue()
{
    DomElement  *commands_object    = NULL;
    DomElement  *events_object      = NULL;

    vector<DomElement *>              commands;
    vector<DomElement *>::iterator    commands_it;
    vector<DomElement *>              events;
    vector<DomElement *>::iterator    events_it;

    // search 'commands'
    commands_object = getCreateCommandsObject();
    if (commands_object){
        commands_object->getElementsByTagName(
            "command",
            commands
        );
    }

    // search 'events'
    events_object = getCreateEventsObject();
    if (events_object){
        events_object->getElementsByTagName(
            "event",
            events
        );
    }

    for (commands_it = commands.begin();
        commands_it != commands.end();
        commands_it++)
    {
        vector<DomElement *>::iterator  tmp_it;
        tmp_it = commands_it;
        DomDocument::deleteElement(*tmp_it);
    }

    for (events_it = events.begin();
        events_it != events.end();
        events_it++)
    {
        vector<DomElement *>::iterator  tmp_it;
        tmp_it = events_it;
        DomDocument::deleteElement(*tmp_it);
    }
}

void SessionObject::sendAck()
{
    uint32_t packet_id = getAckUint();
    sendAck(packet_id);
}

void SessionObject::sendAck(
    const uint32_t  &a_packet_id)
{
    DomDocument *doc    = getDocument();
    DomElement  *el     = NULL;

    PDEBUG(20, "SessionIPNoiseObject::sendAck: 0x'%x'\n",
        a_packet_id
    );

    if ("ipnoise" == getApi()){
        PWARN("SessionIPNoiseObject::sendAck: 0x'%x'\n",
            a_packet_id
        );

    }

    el = doc->createElement("ipnoise");
    el->setAttributeHexSafe("ack",      a_packet_id);
    el->setAttributeSafe("src_sessid",  getSrcSessId());
    el->setAttributeSafe("dst_sessid",  getDstSessId());

    // el will be deleted after send in sendMsg
    xmit(el);
}

void SessionObject::process_packet(
    SkBuffObject *a_skb)
{
    ServerObject *server = NULL;

    // search server
    server = (ServerObject *)getParentNodeByTagName("ipn_server");

    // do process
    server->process_packet(a_skb);
}

PacketReceiverObject * SessionObject::getNextReceiver()
{
    PacketsObject           *packets_obj    = NULL;
    PacketReceiverObject    *receiver       = NULL;
    string                  sessid          = getSessId();

    vector<PacketObject *>           packets;
    vector<PacketObject *>::iterator packets_it;

    // get/create packets object
    packets_obj = getSessionsObject()->getCreatePacketsObject();

    packets_obj->getPackets(packets);
    for (packets_it = packets.begin();
        packets_it != packets.end();
        packets_it++)
    {
        PacketObject         *cur_packet    = *packets_it;
        PacketReceiverObject *cur_receiver  = NULL;

        cur_receiver = cur_packet->getReceiver(sessid);
        if (    cur_receiver
            &&  cur_receiver->getSeq().empty())
        {
            // we have found packet for send
            receiver = cur_receiver;
            break;
        }
    }

    return receiver;
}

void SessionObject::netif_rx(
    SkBuffObject *a_skb)
{
    PDEBUG(20, "SessionIPNoiseObject::netif_rx:\n%s\n",
        a_skb->getInDom()->serialize(1).c_str()
    );

    PacketsObject           *packets_obj    = NULL;
    PacketReceiverObject    *send_receiver  = NULL;
    DomElement              *ipnoise_el     = NULL;
    DomElement              *in_dom         = NULL;
    DomElement              *in_element     = NULL;

    string      packet_src_sessid;
    string      packet_dst_sessid;
    string      packet_seq_str;
    string      packet_ack_str;
    uint32_t    packet_seq  = 0;
    uint32_t    packet_ack  = 0;

    // get/create packets object
    packets_obj = getSessionsObject()->getCreatePacketsObject();

    // search in_dom
    in_dom = a_skb->getInDom();
    if (not in_dom){
        goto fail;
    }

    // search in element
    in_element = in_dom->getFirstChild();
    if (not in_element){
        goto fail;
    }

    // search ipnoise element
    if ("ipnoise" != in_element->getTagName()){
        goto fail;
    }
    ipnoise_el = in_element;

    if ("ipnoise" == getApi()){
        PWARN("[ recv ] session: '%s', seq: '%s', ack: '%s'\n"
            "%s\n",
            getSessId().c_str(),
            ipnoise_el->getAttribute("seq").c_str(),
            ipnoise_el->getAttribute("ack").c_str(),
            ipnoise_el->serialize(1).c_str()
        );
    }

    // get packet attributes
    packet_src_sessid   = ipnoise_el->getAttribute("src_sessid");
    packet_dst_sessid   = ipnoise_el->getAttribute("dst_sessid");
    packet_seq_str      = ipnoise_el->getAttribute("seq");
    packet_seq          = ipnoise_el->getAttributeFromHex("seq");
    packet_ack_str      = ipnoise_el->getAttribute("ack");
    packet_ack          = ipnoise_el->getAttributeFromHex("ack");

    // check 'reset' answer
    if ("reset" == ipnoise_el->getAttribute("flags")){
        // remove session :(
        setExpired("+1");
        goto out;
    }

    // check 'init' answer
    if ("init" == ipnoise_el->getAttribute("flags")){
        // check what seq exist
        if (packet_seq_str.empty()){
            PERROR("Empty 'seq' attribute in:\n'%s'\n",
                in_element->serialize(1).c_str()
            );
            goto fail;
        }
        // it is 'connect' answer, session was created
        // store destination sess ID
        setDstSessId(packet_src_sessid);
        // don't delete us
        unsetExpired();
        // set out state us 'connected'
        setState("opened");
        // wait next packet
        setAck(packet_seq + 1);
        sendAck();

        // add pending packets
        if (    "client" == getApi()
            ||  ("ipnoise" == getApi()
                    && "outcoming" == getType()))
        {
            vector<PacketObject *>           packets;
            vector<PacketObject *>::iterator packets_it;

            packets_obj->getPackets(packets);
            for (packets_it = packets.begin();
                packets_it != packets.end();
                packets_it++)
            {
                PacketObject *cur_packet = *packets_it;
                PacketReceiversObject *cur_receivers_obj = NULL;

                cur_receivers_obj = cur_packet->getCreateReceiversObject();
                cur_receivers_obj->getCreateReceiver(
                    packet_dst_sessid
                );
            }
        }

        goto out;
    }

    // mark us delivered and remove delivered packets
    if (not packet_ack_str.empty()){
        vector<PacketObject *>           packets;
        vector<PacketObject *>::iterator packets_it;

        if (    "incoming" == getType()
            &&  "opening"  == getState())
        {
            // it is first session rcv, so remove expiration timer
            unsetExpired();
            setState("opened");
        }

        packets_obj->getPackets(packets);
        for (packets_it = packets.begin();
            packets_it != packets.end();
            packets_it++)
        {
            PacketObject         *cur_packet   = *packets_it;
            PacketReceiverObject *cur_receiver = NULL;
            uint32_t             receiver_seq  = 0;
            string               receiver_seq_str;

            cur_receiver = cur_packet->getReceiver(
                packet_dst_sessid
            );
            if (not cur_receiver){
                // probably this packet was created
                // before client was connected,
                // search next packet
                continue;
            }

            receiver_seq_str = cur_receiver->getSeq();
            if (receiver_seq_str.empty()){
                // we have packet what yet not delivered
                break;
            }

            receiver_seq = cur_receiver->getSeqUint();
            if (receiver_seq < packet_ack){
                // mark packet as delivered to this session
                cur_receiver->markAsDelivered();
            }

            // remove delivered packets
            if (cur_packet->isDelivered()){
                PDEBUG(20, "delete packet: '%s'\n",
                    cur_packet->serialize(1).c_str()
                );
                DomDocument::deleteElement(cur_packet);
            }
        }
    }

    // if we have next packet for send, do it
    send_receiver = getNextReceiver();
    if (send_receiver){
        send_receiver->xmit();
    }

    // if seq not exist, goto out
    if (packet_seq_str.empty()){
        goto out;
    }

    // check seq number
    if (packet_seq < getAckUint()){
        // we already have received this packet
        // send ack again
        PERROR("was received packet"
            " with already accepted ID: '0x%x',"
            " packet:\n%s\n",
            packet_seq,
            a_skb->getInDom()->serialize(1).c_str()
        );
        // request next packet
        sendAck(packet_seq + 1);
        goto fail;
    } else if (packet_seq > getAckUint()){
        // we still not ready receive this packet
        PERROR("was received packet"
            " from future, ID: '0x%x',"
            " packet:\n%s\n",
            packet_seq,
            a_skb->getInDom()->serialize(1).c_str()
        );
        goto fail;
    }

    // wait next packet
    setAck(packet_seq + 1);
    sendAck();

    // do receive packet if packet have data
    do {
        DomElement *commands_el = NULL;
        DomElement *events_el   = NULL;

        commands_el = ipnoise_el->getElementByTagName(
            "commands"
        );
        events_el = ipnoise_el->getElementByTagName(
            "events"
        );
        if (not commands_el && not events_el){
            // packet have not data
            break;
        }

        // accept packet
        process_packet(a_skb);
    } while (0);

out:
    return;
fail:
    goto out;
}

ssize_t SessionObject::xmit(
    DomElement      *a_packet_data,
    const int32_t   &a_delete_after_send)
{
    ssize_t err = -1;
    PERROR_OBJ(this, "method not implemented,"
        " packet will be broken:\n'%s'\n",
        a_packet_data->serialize(1).c_str()
    );

    if (a_delete_after_send){
        DomDocument::deleteElement(a_packet_data);
        a_packet_data = NULL;
    }

    return err;
}

void SessionObject::xmit(const string &a_packet_data)
{
    DomElement *element = NULL;

    if (not a_packet_data.empty()){
        element = getDocument()->createElementFromContent(a_packet_data);
        // TODO XXX
        // temporary add us to DomModel
        // else getElementsByXpath will not works
        // 20120107 morik@
        appendChildSafe(element);
    }

    if (element){
        // element will be deleted after xmit
        xmit(element);
    }

    return;
}

ssize_t SessionObject::xmitLocal(DomElement *a_packet_data)
{
    ssize_t             wrote           = -1;
    char                buffer[1024]    = { 0x00 };
    IPNoiseObject       *ipnoise        = NULL;
    ServerObject        *server         = NULL;
    string              data;

    vector <DomElement *>               res;
    vector <DomElement *>::iterator     res_it;

    vector <ClientObject *>             clients;
    vector <ClientObject *>::iterator   clients_it;

    // get ipnoise
    ipnoise = (IPNoiseObject *)getDocument()->getRootElement();
    assert("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = (ServerObject *)ipnoise->getServerObject();
    assert("ipn_server" == server->getTagName());

    // search client
    snprintf(buffer, sizeof(buffer),
        "//ipn_clients/ipn_client[@sessid=\"%s\"]",
        getSessId().c_str()
    );

    clients.clear();
    server->getElementsByXpath(buffer, res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        ClientObject *client = (ClientObject *)(*res_it);
        if (not client->isConnected()){
            continue;
        }
        clients.push_back(client);
    }

    if (clients.empty()){
        // there are no connected clients
        goto out;
    }

    // serialize
    data = a_packet_data->serializeForPublic(0);

    // send
    wrote = 0;
    for (clients_it = clients.begin();
        clients_it != clients.end();
        clients_it++)
    {
        ClientObject    *client     = *clients_it;
        ssize_t         cur_wrote   = 0;
        // send data to incoming connection
        wrote = client->write(data);
        PDEBUG(20, "write '%s' to '%s', wrote: '%d'\n",
            data.c_str(),
            client->serialize(0).c_str(),
            cur_wrote
        );
        if (cur_wrote < 0){
            wrote = cur_wrote;
        }
    }

out:
    return wrote;
}

#if 0
ssize_t SessionObject::xmit()
{
    ssize_t         ret = -1;
    string          data;
    char            buffer[1024]    = { 0x00 };
    IPNoiseObject   *ipnoise        = NULL;
    ServerObject    *server         = NULL;
    SessionsObject  *sessions       = NULL;
    NetClientObject *net_client     = NULL;

    vector <ClientObject *>             clients;
    vector <ClientObject *>::iterator   clients_it;

    vector <DomElement *>               res;
    vector <DomElement *>::iterator     res_it;

    string sess_state = getState();

    // get sessions
    sessions = (SessionsObject *)getParentNode();
    assert("ipn_sessions" == sessions->getTagName());

    // get ipnoise
    ipnoise = (IPNoiseObject *)getDocument()->getRootElement();
    assert("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = (ServerObject *)ipnoise->getServerObject();
    assert("ipn_server" == server->getTagName());

    // queue
/*
    {
        // commands
        res.clear();
        sessions->getCreateCommandsObject()->getChilds(res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            DomElement *cur_el = *res_it;
            queue(cur_el);
        }
        // events
        res.clear();
        sessions->getCreateEventsObject()->getChilds(res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            DomElement *cur_el = *res_it;
            queue(cur_el);
        }
    }
*/

    // search outcoming client
    net_client = (NetClientObject *)getElementByTagName(
        "ipn_net_client"
    );
    if (!net_client || !net_client->isConnected()){
        net_client = NULL;
    }

    // search incoming client
    snprintf(buffer, sizeof(buffer),
        "//ipn_clients/ipn_client[@sessid=\"%s\"]",
        getSessId().c_str()
    );
    res.clear();
    server->getElementsByXpath(buffer, res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        ClientObject *client = (ClientObject *)(*res_it);
        if (!client->isConnected()){
            continue;
        }
        clients.push_back(client);
    }

    if (!net_client && clients.empty()){
        // there are no connected clients
        goto out;
    }

    // prepare pending commands and events
    {
        int commands_exist  = 0;
        int events_exist    = 0;

        DomElement *commands_object        = NULL;
        DomElement *events_object          = NULL;
        DomElement *out_ipnoise            = NULL;
        DomElement *out_ipnoise_commands   = NULL;
        DomElement *out_ipnoise_events     = NULL;

        vector<DomElement *>              commands;
        vector<DomElement *>::iterator    commands_it;
        vector<DomElement *>              events;
        vector<DomElement *>::iterator    events_it;

        // search 'commands' object
        commands_object = getCreateCommandsObject();
        if (commands_object){
            commands_object->getElementsByTagName(
                "command",
                commands
            );
        }

        // search 'events' object
        events_object = getCreateEventsObject();
        if (events_object){
            events_object->getElementsByTagName(
                "event",
                events
            );
        }

        if (commands.empty() && events.empty()){
            goto out;
        }

        // prepare packet
        out_ipnoise = getDocument()->createElement("ipnoise");
        out_ipnoise->setAttributeSafe("sessid", getSessId());

        out_ipnoise_commands = getDocument()->createElement(
            "commands"
        );
        out_ipnoise->appendChildSafe(out_ipnoise_commands);

        out_ipnoise_events = getDocument()->createElement(
            "events"
        );
        out_ipnoise->appendChildSafe(out_ipnoise_events);

        for (commands_it = commands.begin();
            commands_it != commands.end();
            commands_it++)
        {
            int skip = 1;

            // check our state
            if (    ("opened"   == sess_state)
                ||  ("logged"   == sess_state))
            {
                // session is not opened
                skip = 0;
            }

            if (    ("opening"      == sess_state)
                &&  ("openSession"  == (*commands_it)->getAttribute("type")))
            {
                skip = 0;
            }

            if (skip){
                continue;
            }

            DomElement *command = (*commands_it)->clone();
            out_ipnoise_commands->appendChildSafe(command);
            commands_exist = 1;
        }

        for (events_it = events.begin();
            events_it != events.end();
            events_it++)
        {
            int skip = 1;

            // check our state
            if (    ("opened"   == sess_state)
                ||  ("logged"   == sess_state))
            {
                // session is not opened
                skip = 0;
            }

            if (skip){
                continue;
            }

            DomElement *event = (*events_it)->clone();
            out_ipnoise_events->appendChildSafe(event);
            events_exist = 1;
        }

        if (commands_exist || events_exist){
            // prepare content
            data = out_ipnoise->serializeForPublic() + "\n";
        }

        // delete temporary element
        DomDocument::deleteElement(out_ipnoise);
    }

    if (data.empty()){
        // there are no data for send
        goto out;
    }

    // OUTCOMING
    if (net_client){
        // send data to outcoming connection
        ret = net_client->write(data);
        goto out;
    }

    // INCOMING
    for (clients_it = clients.begin();
        clients_it != clients.end();
        clients_it++)
    {
        ClientObject *client = (ClientObject *)(*clients_it);
        // send data to incoming connection
        ret = client->write(data);
        PDEBUG(20, "write '%s' to '%s', ret: '%d'\n",
            data.c_str(),
            client->serialize(0).c_str(),
            ret
        );

        goto out;
    }

out:
    if (ret > 0){
        addTxBytes(ret);
        clearQueue();
    }
    return ret;
}
#endif

