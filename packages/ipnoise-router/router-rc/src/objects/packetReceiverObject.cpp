/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/serverObject.h"

#include "objects/packetReceiverObject.h"

PacketReceiverObject * PacketReceiverObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new PacketReceiverObject(a_node, a_doc, a_tagname);
}

PacketReceiverObject::PacketReceiverObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

PacketReceiverObject::~PacketReceiverObject()
{
};

void PacketReceiverObject::shutdown()
{
}

int32_t PacketReceiverObject::autorun()
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

void PacketReceiverObject::timer()
{
    // start xmit
    _xmit();

    // setup timer
    setTimer(PACKET_RETRANSMIT_TIME);
}

// ---------------- sess ID ---------------

string PacketReceiverObject::getSessId()
{
    return getAttribute("sessid");
}

void PacketReceiverObject::setSessId(const string &a_id)
{
    setAttributeSafe("sessid", a_id);
}

// ---------------- last send -------------

string PacketReceiverObject::getLastSendAttempt()
{
    return getAttribute("last_send_attempt");
}

void PacketReceiverObject::setLastSendAttempt(const string &a_last_send)
{
    setAttributeSafe("last_send_attempt", a_last_send);
}

// ----------------------------------------

SessionObject * PacketReceiverObject::getSession()
{
    SessionsObject  *sessions_obj   = NULL;
    SessionObject   *session        = NULL;
    string          sessid          = getSessId();


    // search sessions object
    sessions_obj = (SessionsObject *)getParentNodeByTagName("ipn_sessions");

    // search current session
    if (not sessid.empty()){
        session = sessions_obj->getSession(sessid);
    }

    return session;
}

// ---------------- xmit ------------------

ssize_t PacketReceiverObject::xmit()
{
    ssize_t wrote = -1;
    string  last_send_attempt;

    last_send_attempt = getLastSendAttempt();
    if (not last_send_attempt.empty()){
        // send already in progress
        goto out;
    }

    wrote = _xmit();

 out:
    return wrote;
}

ssize_t PacketReceiverObject::_xmit()
{
    ssize_t         wrote           = -1;
    SessionObject   *session        = NULL;
    PacketsObject   *packets_obj    = NULL;
    PacketObject    *packet         = NULL;
    ServerObject    *server         = NULL;
    DomElement      *el             = NULL;
    string          cur_time_str;
    string          sessid;

    vector<PacketObject *>              packets;
    vector<PacketObject *>::iterator    packets_it;

    if (isDelivered()){
        // skip if delivered
        goto out;
    }

    // search packet
    packet = (PacketObject *)getParentNodeByTagName(
        "ipn_packet"
    );

    // search packets
    packets_obj = (PacketsObject *)getParentNodeByTagName(
        "ipn_packets"
    );

    // get session
    session = getSession();
    if (not session){
        // no such session, request delete us
        setExpired("+1");
        goto out;
    }
    sessid = session->getSessId();

    // check what all early packets delivered
    packets_obj->getPackets(packets);
    for (packets_it = packets.begin();
        packets_it != packets.end();
        packets_it++)
    {
        PacketObject         *cur_packet   = *packets_it;
        PacketReceiverObject *cur_receiver = NULL;

        if ((void *)packet == (void *)cur_packet){
            break;
        }

        cur_receiver = cur_packet->getReceiver(sessid);
        if (    cur_receiver
            &&  not cur_receiver->isDelivered())
        {
            // prev packet(s) yet not delivered
            goto out;
        }
    }

    if (not session->isOnline()){
        // skip not online sessions
        goto out;
    }

    // get server
    server = (ServerObject *)getParentNodeByTagName(
        "ipn_server"
    );

    // get cur time
    cur_time_str = server->getCurTimeStr();

    // assign 'seq' if not exist
    if (getSeq().empty()){
        uint32_t sess_seq = session->getSeqUint();
        setSeq(sess_seq);
        session->setSeq(sess_seq + 1);
    }

    // store last send attempt
    setLastSendAttempt(cur_time_str);

    // send data
    el = packet->getIPNoiseElement()->clone();
    el->setAttributeSafe("src_sessid",  session->getSrcSessId());
    el->setAttributeSafe("dst_sessid",  session->getDstSessId());
    el->setAttributeHexSafe("seq",      getSeqUint());

    // element 'el' will be deleted after send
    wrote = session->xmit(el);

    // setup timer
    setTimer(PACKET_RETRANSMIT_TIME);

out:
    return wrote;
}

int32_t PacketReceiverObject::isDelivered()
{
    int32_t ret = 0;
    string  delivered;

    delivered = getAttribute("delivered");
    if (not delivered.empty()){
        ret = 1;
    }

    return ret;
}

void PacketReceiverObject::wasDeliveredToAllClients()
{
    PacketObject    *packet     = NULL;
    UserObject      *user       = NULL;
    DomElement      *ipnoise_el = NULL;

    vector<DomElement *>            res;
    vector<DomElement *>::iterator  res_it;

    // get packet
    packet = (PacketObject *)getParentNodeByTagName(
        "ipn_packet"
    );
    if (not packet){
        goto fail;
    }

    // get ipnoise element
    ipnoise_el = packet->getIPNoiseElement();
    if (not ipnoise_el){
        goto fail;
    }

    // search events
    ipnoise_el->getElementsByXpath(
        "//events/event[@type=\"conferenceMsg\"]",
        res
    );

    // get owner
    user = getOwner();

    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        DomElement  *cur_event = *res_it;
        DomElement  *msg       = NULL;
        ItemObject  *item      = NULL;

        string      src_conf_huid;
        string      src_conf_id;
        string      msg_id;
        string      conf_id;

        // search message
        msg = cur_event->getElementByTagName("message");
        if (not msg){
            continue;
        }

        // get message source conference info
        src_conf_huid   = msg->getAttribute("src_conf_huid");
        src_conf_id     = msg->getAttribute("src_conf_id");
        msg_id          = msg->getAttribute("id");

        // clients must know nothing about this attributes
        msg->delAttributeSafe("src_conf_huid");
        msg->delAttributeSafe("src_conf_id");

        // check source conference info
        if (    src_conf_huid.empty()
            ||  src_conf_id.empty()
            ||  msg_id.empty())
        {
            continue;
        }

        // search message source item
        item = user->getItem(src_conf_huid);
        if (not item){
            continue;
        }

        // prepare answer event
        {
            DomDocument *doc    = getDocument();
            DomElement  *el     = NULL;
            DomElement  *events = NULL;
            DomElement  *event  = NULL;

            el      = doc->createElement("ipnoise");
            events  = doc->createElement("events");
            event   = doc->createElement("event");

            event->setAttributeSafe("ver",      "0.01");
            event->setAttributeSafe("type",     "conferenceMsgDelivered");
            event->setAttributeSafe("conf_id",  src_conf_id);
            event->setAttributeSafe("msg_id",   msg_id);

            events->appendChildSafe(event);
            el->appendChildSafe(events);

            // element 'el' will be deleted after queue
            item->queue(el);
        }
    }

out:
    return;
fail:
    goto out;
}

void PacketReceiverObject::markAsDelivered()
{
    ServerObject    *server         = NULL;
    PacketObject    *packet         = NULL;
    ItemObject      *item           = NULL;
    string          sessid          = getSessId();
    string          delivered;
    string          cur_time_str;

    // maybe already delivered?
    delivered = getAttribute("delivered");
    if (not delivered.empty()){
        goto out;
    }

    // get server
    server = (ServerObject *)getParentNodeByTagName(
        "ipn_server"
    );

    // get packet
    packet = (PacketObject *)getParentNodeByTagName(
        "ipn_packet"
    );

    // getItem
    item = (ItemObject *)getParentNodeByTagName("ipn_item");

    // get cur time
    cur_time_str = server->getCurTimeStr();

    // mark as delivered
    setAttributeSafe("delivered", cur_time_str);

    PDEBUG(20, "marked us delivered:\n'%s'\n",
        this->serialize(1).c_str()
    );

    // remove timer
    removeTimer();

    // request remove packet
    if (packet->isDelivered()){
        if (item->isSelfItem()){
            // packet was delivered to all clients
            wasDeliveredToAllClients();
        }
        packet->setExpired("+1");
    }

out:
    return;
}

// ---------------- seq -----------------------------------

void PacketReceiverObject::setSeq(const string &a_val)
{
    setAttributeSafe("seq", a_val);
}

void PacketReceiverObject::setSeq(const uint32_t &a_val)
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer),
        "%x",
        a_val
    );
    setSeq(buffer);
}

uint32_t PacketReceiverObject::getSeqUint()
{
    return getAttributeFromHex("seq");
}

string PacketReceiverObject::getSeq()
{
    return getAttribute("seq");
}


