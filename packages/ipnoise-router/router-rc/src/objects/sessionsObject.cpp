/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/sessionsObject.h"

SessionsObject * SessionsObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new SessionsObject(a_node, a_doc, a_tagname);
}

SessionsObject::SessionsObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
    // do remove us after exit
    // (will be called SessionsObject::expired)
    setExpired("0");
};

SessionsObject::~SessionsObject()
{
};

int32_t SessionsObject::autorun()
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

    getDocument()->registrySlot(
        this,
        "new_route_found",
        "ipn_neigh"
    );

out:
    return err;

fail:
    if (err > 0){
        err = -1;
    }
    goto out;
}

PacketsObject * SessionsObject::getCreatePacketsObject()
{
    PacketsObject *packets_obj = NULL;
    packets_obj = (PacketsObject *)getElementByTagName("ipn_packets");
    if (not packets_obj){
        packets_obj = getDocument()->createElement<PacketsObject>(
            "ipn_packets"
        );
        appendChildSafe(packets_obj);
    }
    return packets_obj;
}

void SessionsObject::ackPacket(
    const string    &a_sessid,
    const string    &a_packetid)
{
    PacketsObject                   *packets            = NULL;
    PacketObject                    *packet             = NULL;
    PacketReceiversObject           *packet_receivers   = NULL;
    PacketReceiverObject            *packet_receiver    = NULL;
    vector<PacketReceiverObject *>  receivers;

    packets = getCreatePacketsObject();
    packet  = packets->getPacket(a_packetid);

    if (not packet){
        // no such packet
        goto out;
    }

    // search packet receivers
    packet_receivers = packet->getCreateReceiversObject();

    // packet was delivered, delete receiver
    packet_receiver = packet_receivers->getReceiver(a_sessid);
    if (packet_receiver){
        DomDocument::deleteElement(packet_receiver);
        packet_receiver = NULL;
    }

    // get receivers
    packet_receivers->getReceivers(receivers);
    if (receivers.empty()){
        // no more receivers, delete packet
        DomDocument::deleteElement(packet);
        packet = NULL;

        // search and send next packet if available
        packet = packets->getFirstPacket();
        if (packet){
            xmit(packet);
        }
    }

out:
    return;
}

SessionObject * SessionsObject::getOutcomingSession()
{
    SessionObject *session = NULL;
    char buffer[1024] = { 0x00 };

    snprintf(buffer, sizeof(buffer),
        "//ipn_session_ipnoise[@type=\"outcoming\"]");
    session = (SessionObject *)getElementByXpath(buffer);

    return session;
}

SessionObject * SessionsObject::getSession(
    const string &a_sessid)
{
    return getSession(this, a_sessid);
}

SessionObject * SessionsObject::getSession(
    DomElement      *a_element,
    const string    &a_sessid)
{
    SessionObject *session = NULL;
    char buffer[1024] = { 0x00 };

    // session item
    if (not a_sessid.empty()){
        snprintf(buffer, sizeof(buffer),
            "ipn_session_item[@sessid=\"%s\"]",
            a_sessid.c_str()
        );
    } else {
        snprintf(buffer, sizeof(buffer),
            "ipn_session_item"
        );
    }
    session = (SessionObject *)a_element->getElementByXpath(
        buffer
    );
    if (session){
        goto out;
    }

    // session ipnoise
    if (not a_sessid.empty()){
        snprintf(buffer, sizeof(buffer),
            "ipn_session_ipnoise[@sessid=\"%s\"]",
            a_sessid.c_str()
        );
    } else {
        snprintf(buffer, sizeof(buffer),
            "ipn_session_ipnoise"
        );
    }
    session = (SessionObject *)a_element->getElementByXpath(
        buffer
    );
    if (session){
        goto out;
    }

    // session client
    if (not a_sessid.empty()){
        snprintf(buffer, sizeof(buffer),
            "ipn_session_client[@sessid=\"%s\"]",
            a_sessid.c_str()
        );
    } else {
        snprintf(buffer, sizeof(buffer),
            "ipn_session_client"
        );
    }
    session = (SessionObject *)a_element->getElementByXpath(
        buffer
    );
    if (session){
        goto out;
    }

    // session unknown
    if (not a_sessid.empty()){
        snprintf(buffer, sizeof(buffer),
            "ipn_session_unknown[@sessid=\"%s\"]",
            a_sessid.c_str()
        );
    } else {
        snprintf(buffer, sizeof(buffer),
            "ipn_session_unknown"
        );
    }
    session = (SessionObject *)a_element->getElementByXpath(
        buffer
    );
    if (session){
        goto out;
    }

    // session lo
    if (not a_sessid.empty()){
        snprintf(buffer, sizeof(buffer),
            "ipn_session_lo[@sessid=\"%s\"]",
            a_sessid.c_str()
        );
    } else {
        snprintf(buffer, sizeof(buffer),
            "ipn_session_lo"
        );
    }
    session = (SessionObject *)a_element->getElementByXpath(
        buffer
    );
    if (session){
        goto out;
    }

out:
    return session;
}

SessionObject * SessionsObject::createSession(
    const string    &a_sess_type,
    const string    &a_sessid)
{
    string                              sessid      = a_sessid;
    SessionObject                       *session    = NULL;
    vector<PacketObject *>              packets;
    vector<PacketObject *>::iterator    packets_it;

    if (    "ipn_session_item"      != a_sess_type
        &&  "ipn_session_ipnoise"   != a_sess_type
        &&  "ipn_session_unknown"   != a_sess_type
        &&  "ipn_session_client"    != a_sess_type
        &&  "ipn_session_lo"        != a_sess_type)
    {
        PERROR_OBJ(this, "Unsupported session type: '%s'\n",
            a_sess_type.c_str()
        );
        assert(0);
    }

    session = getDocument()->createElement<SessionObject>(
        a_sess_type
    );

    if (not sessid.empty()){
        session->setSessId(sessid);
    } else {
        sessid = session->getSessId();
    }
    appendChildSafe(session);

    return session;
}

SessionObject * SessionsObject::getCreateSession(
    const string    &sess_type,
    const string    &sessid)
{
    SessionObject *session = NULL;

    session = getSession(sessid);
    if (session){
        goto out;
    }

    session = createSession(sess_type, sessid);

out:
    return session;
}

SessionObject * SessionsObject::createIPNoiseOutcomingSession()
{
    SessionObject *session = NULL;

    session = createSession("ipn_session_ipnoise");
    session->setType("outcoming");

    return session;
}

ItemObject * SessionsObject::getItem()
{
    ItemObject *item = NULL;

    item = (ItemObject *)getParentNodeByTagName(
        "ipn_item"
    );

    return item;
}

void SessionsObject::slot(
    const string    &signal_name,
    DomElement      *object)
{
    assert(object);

    string  object_type;
    object_type = object->getTagName();

    PDEBUG_OBJ(this, 15, "SessionsObject::slot called,"
        " signal_name: '%s',"
        " object_type: '%s'\n",
        signal_name.c_str(),
        object_type.c_str()
    );

    if (    "new_route_found" == signal_name
        &&  "ipn_neigh" == object_type)
    {
        ItemObject          *item              = NULL;
        SessionObject       *session_outcoming = NULL;
        string              item_huid;

        NeighObject         *neigh = NULL;
        string              huid;

        // check neigh
        neigh = (NeighObject *)object;
        if (!neigh){
            goto out;
        }

        // get item for this item's sessions
        // (if it "ipn_session_item" session)
        item = getItem();
        if (item){
            // get item's huid
            item_huid = item->getHuid();
            if (item_huid != neigh->getDstHuid()){
                goto out;
            }

            // yea! we have route to this item
            // open session now! (if it needed)
            session_outcoming = getOutcomingSession();
            if (not session_outcoming){
                PDEBUG_OBJ(this, 15,
                    "Trying open new ipnoise session\n"
                );
                session_outcoming = createIPNoiseOutcomingSession();
            }
            session_outcoming->connect();
        }
    } else {
        PERROR_OBJ(this, "Unsupported signal: '%s' object type: '%s'\n",
            signal_name.c_str(),
            object_type.c_str()
        );
    }

out:
    return;
}

void SessionsObject::getOnlineSessions(
    vector<SessionObject *>  &a_out)
{
    vector<SessionObject *>                res;
    vector<SessionObject *>::iterator      res_it;

    getSessions(res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        SessionObject *session = (SessionObject *)(*res_it);
        if (session->isOnline()){
            a_out.push_back(session);
        }
    }
}

void SessionsObject::getSessions(
    vector<SessionObject *>  &a_out)
{
    vector<DomElement *>                res;
    vector<DomElement *>::iterator      res_it;

    // collect sessions
    res.clear();
    getElementsByTagName("ipn_session_client", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        SessionObject *session = (SessionObject *)(*res_it);
        a_out.push_back(session);
    }
    res.clear();
    getElementsByTagName("ipn_session_ipnoise", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        SessionObject *session = (SessionObject *)(*res_it);
        a_out.push_back(session);
    }
    res.clear();
    getElementsByTagName("ipn_session_item", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        SessionObject *session = (SessionObject *)(*res_it);
        a_out.push_back(session);
    }
    res.clear();
    getElementsByTagName("ipn_session_unknown", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        SessionObject *session = (SessionObject *)(*res_it);
        a_out.push_back(session);
    }
}

PacketObject * SessionsObject::queue(const string &a_packet_data)
{
    DomElement      *element    = NULL;
    PacketObject    *packet     = NULL;

    if (not a_packet_data.empty()){
        element = getDocument()->createElementFromContent(a_packet_data);
        // TODO XXX
        // temporary add us to DomModel
        // else getElementsByXpath will not works
        // 20120107 morik@
        appendChildSafe(element);
    }

    if (element){
        packet = queue(element);
        DomDocument::deleteElement(element);
    }

    return packet;
}

PacketObject * SessionsObject::queue(DomElement *a_packet_data)
{
    PacketsObject *packets = getCreatePacketsObject();
    PacketObject  *packet  = NULL;

    packet = packets->queue(a_packet_data);
    return packet;
}

ssize_t SessionsObject::xmit(const string &a_packet_data)
{
    PWARN("SessionsObject::xmit(&a_packet_data)\n");

    ssize_t         err = -1;
    PacketObject    *packet = NULL;
    packet = queue(a_packet_data);
    err = xmit(packet);

    return err;
}

ssize_t SessionsObject::xmit(DomElement *a_packet_data)
{
    PWARN("SessionsObject::xmit(*a_packet_data)\n");
    ssize_t         err     = -1;
    PacketObject    *packet = NULL;

    packet  = queue(a_packet_data);
    err     = xmit(packet);

    return err;
}

ssize_t SessionsObject::xmit(PacketObject *a_packet)
{
    PWARN("SessionsObject::xmit(*a_packet)\n");
    ssize_t wrote = 0;

    wrote = a_packet->xmit();

    return wrote;
}

