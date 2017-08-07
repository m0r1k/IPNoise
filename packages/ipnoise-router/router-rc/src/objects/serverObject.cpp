/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include <ipnoise-common/rand.hpp>
#include <ipnoise-common/system.h>

#include "libxml2/domDocument.h"
#include "objects/ipnoiseObject.h"
#include "objects/netCommandObject.h"
#include "objects/neighObject.h"
#include "objects/neighsObject.h"
#include "objects/clientObject.h"
#include "objects/userObject.h"
#include "objects/sessionItemObject.h"
#include "inode.h"

#include "objects/serverObject.h"

static struct fuse_lowlevel_ops fuse_ll_oper;

static ServerObject     *server = NULL;
extern IPNoiseObject    *ipnoise;

ServerObject * ServerObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ServerObject(a_node, a_doc, a_tagname);
}

ServerObject::ServerObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
    command     = NULL;
    m_uptime    = 0;
    listener    = NULL;
    srand(time(NULL));

    // fuse
    _fuse_se            = NULL;
    _fuse_ch            = NULL;
    _fuse_args          = NULL;
    _fuse_ev            = NULL;
    _fuse_mountpoint    = NULL;

    // store server
    server = this;
};

ServerObject::~ServerObject()
{
};

Command *ServerObject::getCmd()
{
    return command;
}

UserObject * ServerObject::getUser(const string &huid)
{
    char tmp[1024] = { 0x00 };
    vector <DomElement *> res;
    UserObject *user = NULL;

    snprintf(tmp, sizeof(tmp),
        "//ipn_users/ipn_user[@huid=\"%s\"]",
        huid.c_str()
    );
    res.clear();
    getElementsByXpath(tmp, res);
    if (res.size()){
        user = (UserObject *)res[0];
    }

    return user;
}

void ServerObject::shutdown()
{
    vector <NeighObject *>           neighs;
    vector <NeighObject *>::iterator neighs_it;

    PINFO("Shutdown..\n");

    // stop timer
    removeTimer();

    // stop fuse
    stopFuse();

    // stop listener
    if (listener != NULL){
        evconnlistener_free(listener);
        listener = NULL;
    }

    // stop hostos listener
    if (listener_hostos != NULL){
        evconnlistener_free(listener_hostos);
        listener_hostos = NULL;
    }

    // update neighs {
    getCreateNeighsObject()->getNeighs(neighs);
    for (neighs_it = neighs.begin();
        neighs_it != neighs.end();
        neighs_it++)
    {
        NeighObject *neigh = *neighs_it;
        neigh->setStateDown();
    }
    // update neighs }

    if (command != NULL){
        delete command;
        command = NULL;
    }
}

LinksObject * ServerObject::getCreateLinksObject()
{
    LinksObject *links = NULL;

    links = (LinksObject *)getElementByTagName("ipn_links");
    if (!links){
        links = getDocument()->createElement<LinksObject>(
            "ipn_links"
        );
        // Don't save "ipn_link" object after save
        links->setExpired("0");
        appendChildSafe(links);
    }

    return links;
}

void ServerObject::CheckInterfaces()
{
    char buffer[1024];
    int i;

    time_t          cur_time       = getCurTime();
    string          cur_time_str   = getCurTimeStr();
    string          ifname         = "";
    struct ifreq    ifr;

    vector <InterfaceInfo>  ifs_info;
    vector <DomElement *>   items;
    vector <DomElement *>   users;
    vector <NeighObject *>  neighs;

    LinksObject    *links = NULL;
    LinkObject     *link  = NULL;

    // search 'links' element
    links = getCreateLinksObject();

    ifs_info = LinkObject::getInterfacesInfo();
    for (i = 0; i < (int)ifs_info.size(); i++){
        // EVERY TIME INITIALIZE STACK-VARIABLES!
        memset(&ifr, 0x00, sizeof(ifr));

        // get interface info
        InterfaceInfo &if_info = ifs_info[i];

        // get if name
        ifname = if_info.if_name;

        // search link
        link = links->getCreateLinkByName(
            ifname, if_info.if_index);

        // do 'touch' link
        link->setLastCheck(cur_time);
    }

again1:
    // remove 'untouched' links (only if ifindex exist)
    snprintf(buffer, sizeof(buffer),
        "//ipn_links/ipn_link"
    );
    items.clear();
    getDocument()->getElementsByXpath(buffer, items);
    for (i = 0; i < (int)items.size(); i++){
        link = (LinkObject *)items[i];
        assert(link != NULL);

        if (    link->getIfIndex()     >= 0
            &&  link->getLastCheck()   != cur_time)
        {
            DomDocument::deleteElement(link);
            goto again1;
        }
    }
}

void ServerObject::process_packet(SkBuffObject *a_skb)
{
    // receive command and events
    getCmd()->recvCommandSlot(a_skb);
}

void ServerObject::netif_rx(SkBuffObject *a_skb)
{
    vector <DomElement *>   res;
    NeighsObject            *neighs         = NULL;
    UserObject              *user           = NULL;
    ItemObject              *item           = NULL;
    SessionsObject          *sessions_obj   = NULL;
    DomElement              *ipnoise_el     = NULL;
    DomElement              *in_dom         = NULL;
    DomElement              *in_element     = NULL;

    string  indev_name;
    string  to;
    string  from;
    string  src_sessid;
    string  dst_sessid;

    // as default use server's sessions object
    sessions_obj = getCreateSessionsObject();

    // get source and dest info
    from = a_skb->getAttribute("from");
    to   = a_skb->getAttribute("to");

    // search in_dom
    in_dom = a_skb->getInDom();
    if (not in_dom){
        goto drop;
    }

    // search in element
    in_element = in_dom->getFirstChild();
    if (not in_element){
        goto drop;
    }

    // search ipnoise element
    if ("ipnoise" != in_element->getTagName()){
        goto drop;
    }
    ipnoise_el = in_element;

    // get in dev
    // LO_DEV_NAME      - virtual loopback
    // CLIENTS_DEV_NAME - tcp 2210 hostos
    // IPNOISE_DEV_NAME - udp/tcp 2210 ipv6 ipnoise
    indev_name = a_skb->getAttribute("indev_name");
    if (LO_DEV_NAME == indev_name){
        // process loopback packets directly
        goto process_command;
    }

    // check to
    if (not to.empty()){
        if (    CLIENTS_DEV_NAME    != indev_name
            &&  MULTICAST_USER_HUID != to)
        {
            // check what destination user exist
            user = getUser(to);
            if (!user){
                // no such user
                goto drop;
            }
        }
    }

    // check from
    if (not from.empty()){
        if (MULTICAST_USER_HUID != from){
            // martian source
            UserObject *tmp_user = NULL;
            tmp_user = getUser(from);
            if (tmp_user){
                // martian source
                // TODO send "IP duplicate event"
                // to packet sender
                goto drop;
            }

            // create item
            if (user){
                item         = user->getCreateItem(from);
                sessions_obj = item->getCreateSessionsObject();
            }
        }

        // search neighs
        neighs = getCreateNeighsObject();

        // add neighs from skb
        neighs->addNeigh(
            a_skb, // Element for search "ipnoise"
            from,                           // Item
            a_skb->getAttribute("from_ll"), // Link address
            indev_name,                     // Link device
            "1"                             // Metric
        );
    }

    // get sessid attributes
    src_sessid = ipnoise_el->getAttribute("src_sessid");
    dst_sessid = ipnoise_el->getAttribute("dst_sessid");

    // process sessions
    if (    not src_sessid.empty()
        ||  not dst_sessid.empty())
    {
        SessionObject *sess = NULL;

        // search opened session
        if (not dst_sessid.empty()){
            sess = server->getSession(dst_sessid);
            if (sess){
                sess->netif_rx(a_skb);
            } else {
                // session not found, send reset
                DomDocument *doc = getDocument();
                DomElement  *el  = NULL;
                el = doc->createElement("ipnoise");
                el->setAttributeSafe("flags",      "reset");
                el->setAttributeSafe("src_sessid", dst_sessid);
                el->setAttributeSafe("dst_sessid", src_sessid);

                if (CLIENTS_DEV_NAME == indev_name){
                    ClientObject *client = NULL;
                    client = a_skb->getClient();
                    if (client){
                        client->write(
                            el->serializeForPublic()
                        );
                    }
                } else {
                    if (    not from.empty()
                        &&  not to.empty())
                    {
                        ServerObject::sendMsg(
                            el,
                            from,   // dst_huid
                            to,     // src_huid
                            0       // don't delete 'el'
                        );
                    }
                }
                DomDocument::deleteElementSafe(el);
            }
            goto del;
        }

        // if we here, then src_sessid is not empty
        if ("init" == ipnoise_el->getAttribute("flags")){
            uint32_t        seq     = 0;
            ClientObject    *client = NULL;

            // get seq
            seq = ipnoise_el->getAttributeFromHex(
                "seq"
            );

            // it is request open session
            if (CLIENTS_DEV_NAME == indev_name){
                sess = sessions_obj->createSession(
                    "ipn_session_client"
                );
            } else {
                sess = sessions_obj->createSession(
                    "ipn_session_ipnoise"
                );
                PWARN("was received 'init' packet"
                    " from dev: '%s'\n",
                    indev_name.c_str()
                );
            }

            // if client exist, store him sessid
            client = a_skb->getClient();
            if (client){
                client->setSessId(sess->getSessId());
            }

            sess->setType("incoming");
            sess->setState("opening");

            // setup timeout for open session
            sess->setExpired(
                SESSION_INIT_TIMEOUT
            );

            // store dst sess ID
            sess->setDstSessId(src_sessid);

            // wait next packet
            sess->setAck(seq + 1);

            // send answer
            {
                uint32_t    seq  = sess->getSeqUint();
                DomDocument *doc = getDocument();
                DomElement  *el  = NULL;
                el = doc->createElement("ipnoise");
                el->setAttributeSafe("flags",       "init");
                el->setAttributeSafe("src_sessid",  sess->getSrcSessId());
                el->setAttributeSafe("dst_sessid",  sess->getDstSessId());
                el->setAttributeHexSafe("seq",      seq);
                el->setAttributeHexSafe("ack",      sess->getAckUint());
                // element 'el' will be deleted after xmit
                sess->xmit(el);

                // update seq
                sess->setSeq(seq + 1);
            }
        }
        // delete packet anyway
        goto del;
    }

process_command:
    process_packet(a_skb);

out:
    return;
drop:
    PWARN("drop packet: '%s'\n",
        a_skb->serialize(1).c_str()
    )
    goto out;
del:
    goto out;
}

void ServerObject::sendPacket(SkBuffObject *skb)
{
    int i;
    vector <DomElement *> items;
    string outdev_name;
    string outdev_index_str;
    int outdev_index = -1;

    outdev_name         = skb->getAttribute("outdev_name");
    outdev_index_str    = skb->getAttribute("outdev_index");
    if (not outdev_index_str.empty()){
        outdev_index = atoi(outdev_index_str.c_str());
    }

    items.clear();
    getDocument()->getElementsByXpath(
        "//ipn_links/ipn_link",
        items
    );

    for (i = 0; i < (int)items.size(); i++){
        LinkObject *tr = (LinkObject *)items[i];
        assert(tr != NULL);

        if (not outdev_name.empty()
            && outdev_name != tr->getName())
        {
            continue;
        }

        if (outdev_index >= 0
            && outdev_index != tr->getIfIndex())
        {
            continue;
        }

        PDEBUG_OBJ(this, 9, "Send packet via '%s' {\n", tr->getName().c_str());
        tr->linkSendPacket(skb);
        PDEBUG_OBJ(this, 9, "Send packet } \n");
    }
}

void ServerObject::acceptSignalSocket(
    struct evconnlistener   *listener       __attribute__ ((unused)),
    evutil_socket_t         socket,
    struct sockaddr         *sa,
    int                     socklen,
    void                    *user_data      __attribute__ ((unused)))
{
    int res;
    ClientObject    *client     = NULL;
    DomElement      *ipnoise    = NULL;
    DomElement      *clients    = NULL;

    // get server
    assert (server != NULL);

    // get ipnoise
    ipnoise = server->getDocument()->getRootElement();
    assert(ipnoise);

    // make socket non-blocked
    res = evutil_make_socket_nonblocking(socket);
    if (res < 0){
        PERROR_OBJ(server, "Cannot make socket not blocking\n");
        goto fail;
    }

    // get create element 'clients'
    clients = server->getCreateClientsObject();

    // create new client
    client = server->getDocument()
        ->createElement<ClientObject>("ipn_client");

    // Don't save this element after exit
    clients->setExpired("0");

    if (AF_INET6 == sa->sa_family){
        client->setRxDevName(IPNOISE_DEV_NAME);
    } else {
        client->setRxDevName(CLIENTS_DEV_NAME);
    }

    // init sesion
    res = client->accept(
        socket,
        sa,
        socklen
    );

    clients->appendChildSafe(client);

out:
    return;
fail:
    goto out;
}

/*
 *  Read IPV6 UDP packets from 2210 signal port.
 *  Note:
 *  We should do read here for avoid ICMPv6 "dst port unreachable" message.
 *  All our works We do in LinkObject::read_udp_cb via RAW socket,
 *  because no way here to known destination IPV6 addr :(
 */
void ServerObject::read_udp_cb(
    int32_t socket,
    short,          // event
    void *)         // a_ctx
{
    // ServerObject     *server = (ServerObject *)a_ctx;
    // char             ipstr[INET6_ADDRSTRLEN] = { 0x00 };
    char                buffer[MAX_PACKET_SIZE] = { 0x00 };
    struct sockaddr_in6 sin6;
    int32_t             sin6_len = sizeof(sin6);
    ssize_t             rd;

    // string  src_huid;
    // int32_t src_port = 0;

    vector<string>              cmds;
    vector<string>::iterator    cmds_it;

    memset(buffer, 0x00, sizeof(buffer));
    rd = recvfrom(
        socket,
        buffer,
        sizeof(buffer),
        0,
        (struct sockaddr *)&sin6,
        (socklen_t *)&sin6_len
    );
    if (rd <= ssize_t(0)){
        goto out;
    }

/*
    // get addr info
    inet_ntop(AF_INET6, &sin6.sin6_addr, ipstr, sizeof(ipstr));
    src_huid = ipstr;
    src_port = ntohs(sin6.sin6_port);

    // append to buffer
    server->m_udp_buffer += buffer;

    // search packets
    ServerObject::cutIPNoisePackets(
        server->m_udp_buffer,
        cmds
    );
    for (cmds_it = cmds.begin();
        cmds_it != cmds.end();
        cmds_it++)
    {
        const string &cmd = *cmds_it;
        SkBuffObject *skb = NULL;

        PWARN("MORIK was read from: '%s':'%d' packet: '%s'\n",
            src_huid.c_str(),
            src_port,
            cmd.c_str()
        );


        // process api command
        skb = sess->getDocument()->createElement<SkBuffObject>(
            "ipn_skbuff"
        );
        if (not skb){
            PERROR_OBJ(sess, "Cannot allocate new skb\n");
            goto out;
        }

        skb->setInContent(cmd);
        skb->setTo(item->getHuid());
        skb->setFrom(client->getDstAddr());
        skb->setInDevName(IPNOISE_DEV_NAME);
        skb->setInDevIndex(sess->getIno());

        sess->getDocument()->emitSignalAndDelObj(
            "packet_incoming",
            skb
        );
    }
*/

out:
    return;
}

int32_t ServerObject::initSignalPortUDP()
{
    int res, err = 0;
    int                     sd      = -1;
    int32_t                 p_proto = -1;
    struct  protoent        *proto  = NULL;
    struct  event           *event  = NULL;
    struct  sockaddr_in6    sin6;

    proto = getprotobyname("udp");
    if (!proto){
        PERROR_OBJ(this, "Cannot get proto by name: 'udp'\n");
        goto fail;
    }
    p_proto = proto->p_proto;
    endprotoent();

    sd = socket(PF_INET6, SOCK_DGRAM, p_proto);
    if (sd < 0){
        PERROR_OBJ(this, "Cannot create:"
            " socket(PF_INET6, SOCK_DGRAM, %d);\n",
            proto->p_proto);
        goto fail;
    }

    memset((void *)&sin6, 0, sizeof(sin6));
    sin6.sin6_family    = AF_INET6;
    sin6.sin6_flowinfo  = 0;
    sin6.sin6_port      = htons(IPNOISE_UDP_PORT);
    sin6.sin6_addr      = in6addr_any;

    res = ::bind(sd, (struct sockaddr *)&sin6, sizeof(sin6));
    if (res < 0){
        PERROR_OBJ(this, "bind() failed\n");
        goto fail;
    }

    res = fcntl(sd, F_SETFL, O_NONBLOCK);
    if (res == -1){
       PERROR_OBJ(this, "%s\n", "fcntl()");
       goto fail;
    }

    event = event_new(
        getDocument()->getEvBase(),
        sd,
        EV_READ | EV_PERSIST,
        read_udp_cb,
        (void *)this
    );
    if (!event){
        PERROR_OBJ(this, "%s\n", "Cannot create new event");
        goto fail;
    }
    event_add(event, NULL);

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int32_t ServerObject::initSignalPortTCP()
{
    int32_t err = -1, sd = -1;

    struct sockaddr_in6 server;
    struct hostent *hp;

    const char *bind_addr   = "::0";
    int         bind_port   = 2210;

    memset((char *)&server, 0x00, sizeof(server));
    hp = gethostbyname2(bind_addr, AF_INET6);
    if (hp == NULL) {
        PERROR_OBJ(this, "Cannot resolv addr: '%s'\n", bind_addr);
        goto fail;
    }

    memcpy(
        (char *)&server.sin6_addr,
        hp->h_addr,
        hp->h_length
    );

    server.sin6_family      = hp->h_addrtype;
    server.sin6_port        = htons(bind_port);
    server.sin6_flowinfo    = 0;
    server.sin6_scope_id    = 0;

    listener = evconnlistener_new_bind(
        getDocument()->getEvBase(),                     // libevent instance
        ServerObject::acceptSignalSocket,               // accept callback
        (void *)this,                                   // user defined argument
	    LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,        // flags
        -1,                                             // backlog size
	    (struct sockaddr*)&server,                      // bind addr
	    sizeof(server)                                  // bind addr size
    );

	if (!listener) {
		PERROR_OBJ(this, "Could not create a listener!\n");
        goto fail;
	}

    // all ok
    err = 0;

ret:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    if (sd >= 0){
        close(sd);
        sd = -1;
    }
    goto ret;

}

int32_t ServerObject::initHostOsSignalPort()
{
    int32_t err = -1, sd = -1;

    struct sockaddr_in server;
    struct hostent *hp;

    const char *bind_addr   = "0.0.0.0";
    int         bind_port   = 2210;

    memset((char *)&server, 0x00, sizeof(server));
    hp = gethostbyname2(bind_addr, AF_INET);
    if (hp == NULL) {
        PERROR_OBJ(this, "Cannot resolv addr: '%s'\n", bind_addr);
        goto fail;
    }

    memcpy(
        (char *)&server.sin_addr,
        hp->h_addr,
        hp->h_length
    );

    server.sin_family   = AF_INET;
    server.sin_port     = htons(bind_port);

    listener_hostos = evconnlistener_hostos_new_bind(
        getDocument()->getEvBase(),                     // libevent instance
        ServerObject::acceptSignalSocket,               // accept callback
        (void *)this,                                   // user defined argument
	    LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,        // flags
        -1,                                             // backlog size
	    (struct sockaddr*)&server,                      // bind addr
	    sizeof(server)                                  // bind addr size
    );

	if (!listener_hostos) {
		PERROR_OBJ(this, "Could not create a hostos listener!\n");
        goto fail;
	}

    // all ok
    err = 0;

ret:
    return err;

fail:
    err = -1;
    if (sd >= 0){
        close(sd);
        sd = -1;
    }
    goto ret;

}

#if 0
void ServerObject::processPendingSessions()
{
    char buffer[1024] = { 0x00 };
    vector <DomElement *>           session_objects;
    vector <DomElement *>::iterator session_objects_it;

    // search all sessions for items
    snprintf(buffer, sizeof(buffer), "//ipn_session_item");
    session_objects.clear();
    server->getDocument()->getElementsByXpath(
        buffer,
        session_objects
    );
    for (session_objects_it = session_objects.begin();
        session_objects_it != session_objects.end();
        session_objects_it++)
    {
        SessionObject   *session = NULL;
        session = (SessionObject *)*session_objects_it;
        session->xmit();
    }

    // search all sessions for clients
    snprintf(buffer, sizeof(buffer), "//ipn_session_clients");
    session_objects.clear();
    server->getDocument()->getElementsByXpath(
        buffer,
        session_objects
    );
    for (session_objects_it = session_objects.begin();
        session_objects_it != session_objects.end();
        session_objects_it++)
    {
        SessionObject   *session = NULL;
        session = (SessionObject *)*session_objects_it;
        session->xmit();
    }

    // search all sessions for unknowns
    snprintf(buffer, sizeof(buffer), "//ipn_session_unknowns");
    session_objects.clear();
    server->getDocument()->getElementsByXpath(
        buffer,
        session_objects
    );
    for (session_objects_it = session_objects.begin();
        session_objects_it != session_objects.end();
        session_objects_it++)
    {
        SessionObject   *session = NULL;
        session = (SessionObject *)*session_objects_it;
        session->xmit();
    }
}
#endif

uint32_t ServerObject::getUpTime()
{
    return m_uptime;
}

string ServerObject::getUpTimeStr()
{
    char        buffer[512] = { 0x00 };
    uint32_t    time        = getUpTime();
    snprintf(buffer, sizeof(buffer), "%u", time);
    return buffer;
}

time_t ServerObject::getCurTime()
{
    return time(NULL);
}

string ServerObject::getCurTimeStr()
{
    char    buffer[512] = { 0x00 };
    time_t  time        = getCurTime();
    snprintf(buffer, sizeof(buffer), "%ld", time);
    return buffer;
}

void ServerObject::timer()
{
    string cur_time_str = getCurTimeStr();

    // update uptime
    m_uptime++;

    // update curtime
    setAttributeSafe("cur_time", cur_time_str);

    // TODO must be fully async, not every seconds
    // morik@ 20111220
    // server->processPendingSessions();

    // check interfaces state
    server->CheckInterfaces();

    // reschedule
    setTimer("+1");
}

void ServerObject::registrySignals()
{
}

void ServerObject::registrySlots()
{
    getDocument()->registrySlot(
        this,
        "packet_incoming",
        "ipn_skbuff"
    );
    getDocument()->registrySlot(
        this,
        "packet_outcoming",
        "ipn_skbuff"
    );
    getDocument()->registrySlot(
        this,
        "new_contact_was_created",
        "ipn_contact"
    );
    getDocument()->registrySlot(
        this,
        "new_item_was_created",
        "ipn_item"
    );
    getDocument()->registrySlot(
        this,
        "new_user_was_created",
        "ipn_user"
    );
    getDocument()->registrySlot(
        this,
        "new_route_found",
        "ipn_neigh"
    );
    getDocument()->registrySlot(
        this,
        "conferenceJoin",
        "ipn_conference_item"
    );
    getDocument()->registrySlot(
        this,
        "conferenceCreated",
        "ipn_conference"
    );

    // SessionItemObject
    getDocument()->registrySlot(
        this,
        "ipnoise_conn_up",
        "ipn_session_item"
    );
    getDocument()->registrySlot(
        this,
        "ipnoise_conn_down",
        "ipn_session_item"
    );
}

void ServerObject::SetupNewItem(ItemObject *item)
{
    string std_out, std_err;
    UserObject  *user       = NULL;
    string      item_huid;
    string      user_huid;

    // get item huid
    item_huid = item->getHuid();
    if (item_huid.empty()){
        goto out;
    }

    // get user
    user = item->getOwner();
    assert(user);

    // get user huid
    user_huid = user->getHuid();

    // ok, trying to setup this item
    {
        const char * const argv[] = {
            "/usr/sbin/ipnoise-setup-item",
            "add",
            user_huid.c_str(),
            item_huid.c_str(),
            NULL
        };
        my_system(argv, std_out, std_err);
    }

out:
    return;
}

void ServerObject::SetupNewUser(UserObject *user)
{
    string std_out, std_err;
    string user_huid;

    // get huid
    user_huid = user->getHuid();
    if (user_huid.empty()){
        goto out;
    }

    // ok, trying to setup this user
    {
        const char * const argv[] = {
            "/usr/sbin/ipnoise-setup-user",
            "add",
            user_huid.c_str(),
            NULL
        };
        my_system(argv, std_out, std_err);
    }

out:
    return;
}

void ServerObject::slot(
    const string    &signal_name,
    DomElement      *object)
{
    assert(object != NULL);
    string object_type = "";

    object_type = object->getTagName();

    if ("packet_incoming" == signal_name){
        if ("ipn_skbuff" == object_type){
            netif_rx((SkBuffObject *)object);
        }
    } else if ("packet_outcoming" == signal_name){
        if ("ipn_skbuff" == object_type){
            sendPacket((SkBuffObject *)object);
        }
    } else if ("new_item_was_created" == signal_name){
        // called every time when user have added new item in contact list
        ItemObject *item = NULL;
        item = (ItemObject *)object;
        if (!item){
            goto out;
        }
        SetupNewItem(item);
    } else if ("new_user_was_created" == signal_name){
        UserObject *user = NULL;
        user = (UserObject *)object;
        if (!user){
            goto out;
        }
        SetupNewUser(user);
    } else if ( "new_route_found" == signal_name
        &&      "ipn_neigh" == object_type)
    {
    } else if ("conferenceCreated" == signal_name){
        if ("ipn_conference" == object_type){}
    } else if ("ipnoise_conn_up" == signal_name){
    } else if ("ipnoise_conn_down" == signal_name){
    }

out:
    return;
}

// ---------------------- Fuse filesystem -----------------------------------

int ServerObject::fuse_stat(
    DomElement      *ipnoise,
    fuse_ino_t      ino,
    struct stat     *stbuf)
{
    char buffer[1024]   = { 0x00 };
    int err             = -1;
    int ino_subtype     = 0;

    vector<DomElement *> items;
    DomElement *element = NULL;

    stbuf->st_ino   = ino;
    ino_subtype     = (ino - (int(ino / INODE_NEXT_ELEMENT_OFFSET)
        * INODE_NEXT_ELEMENT_OFFSET));

    if (    (ino_subtype == INODE_SUBTYPE_ATTRIBUTES)
        ||  (ino_subtype == INODE_SUBTYPE_LOG)
        ||  (ino_subtype == INODE_SUBTYPE_XML))
    {
        stbuf->st_mode  = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size  = 100e6;
    } else {
        snprintf(buffer, sizeof(buffer),
            "//*[@_ino=\"%x\"]",
            (unsigned int)ino
        );
        items.clear();
        ipnoise->getElementsByXpath(buffer, items);
        if (items.size()){
            element = items[0];
        }

        if (!element){
            goto fail;
        }

        stbuf->st_mode  = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }

    // all ok;
    err = 0;

out:
    return err;

fail:
    err = -1;
    goto out;
}

void ServerObject::fuse_ll_getattr(
    fuse_req_t              req,
    fuse_ino_t              ino,
    struct fuse_file_info   *)      // fi
{
    int res;
    struct stat stbuf;
    IPNoiseObject *ipnoise = NULL;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = (IPNoiseObject *)server->getDocument()->getRootElement();
    assert(ipnoise);

    // clear buf
    memset(&stbuf, 0, sizeof(stbuf));

    res = fuse_stat(ipnoise, ino, &stbuf);
    if (res == -1){
        goto fail;
    }

    fuse_reply_attr(req, &stbuf, 1.0);

out:
    return;

fail:
    fuse_reply_err(req, ENOENT);
    goto out;
}

void ServerObject::fuse_ll_lookup(
    fuse_req_t      req,
    fuse_ino_t      parent,
    const char      *name)
{
    struct fuse_entry_param e;
    char buffer[1024] = { 0x00 };
    int i;

    DomElement *element         = NULL;
    DomElement *parent_element  = NULL;
    DomElement *ipnoise         = NULL;

    vector<DomElement *>            items;
    vector<DomElement *>::iterator  items_i;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = server->getDocument()->getRootElement();
    assert(ipnoise);

    if (!strncmp(name, ".attributes", 11)){
        memset(&e, 0, sizeof(e));
        e.ino               = (parent - 1) + INODE_SUBTYPE_ATTRIBUTES;
        e.attr_timeout      = 1.0;
        e.entry_timeout     = 1.0;
        fuse_stat(ipnoise, e.ino, &e.attr);
        fuse_reply_entry(req, &e);
        goto out;
    }

    if (!strncmp(name, ".log", 4)){
        memset(&e, 0, sizeof(e));
        e.ino               = (parent - 1) + INODE_SUBTYPE_LOG;
        e.attr_timeout      = 1.0;
        e.entry_timeout     = 1.0;
        fuse_stat(ipnoise, e.ino, &e.attr);
        fuse_reply_entry(req, &e);
        goto out;
    }
    if (!strncmp(name, ".xml", 4)){
        memset(&e, 0, sizeof(e));
        e.ino               = (parent - 1) + INODE_SUBTYPE_XML;
        e.attr_timeout      = 1.0;
        e.entry_timeout     = 1.0;
        fuse_stat(ipnoise, e.ino, &e.attr);
        fuse_reply_entry(req, &e);
        goto out;
    }

    snprintf(buffer, sizeof(buffer),
        "//*[@_ino=\"%x\"]",
        (unsigned int)parent
    );
    items.clear();
    ipnoise->getElementsByXpath(buffer, items);
    if (items.size()){
        parent_element = items[0];
    }

    // check what parent exist
    if (!parent_element){
        goto fail;
    }

    element = NULL;
    items.clear();
    parent_element->getChilds(items);
    for (i = 0; i < (int)items.size(); i++){
        DomElement *cur_element = items[i];
        string cur_fuse_name    = cur_element->getFuseFullName();
        if (cur_fuse_name == name){
            element = cur_element;
            break;
        }
    }
    if (!element){
        goto fail;
    }

    // fill data
    memset(&e, 0, sizeof(e));
    e.ino               = element->getInoInt();
    e.attr_timeout      = 1.0;
    e.entry_timeout     = 1.0;

    fuse_stat(ipnoise, e.ino, &e.attr);
    fuse_reply_entry(req, &e);

out:
    return;

fail:
    fuse_reply_err(req, ENOENT);
    goto out;
}

void ServerObject::dirbuf_add(
    fuse_req_t      req,
    struct dirbuf   *b,
    const char      *name,
    fuse_ino_t      ino)
{
    struct stat stbuf;
    size_t oldsize = b->size;

    b->size += fuse_add_direntry(req, NULL, 0, name, NULL, 0);
    b->p = (char *) realloc(b->p, b->size);
    memset(&stbuf, 0, sizeof(stbuf));
    stbuf.st_ino = ino;
    fuse_add_direntry(
        req,
        b->p + oldsize,
        b->size - oldsize,
        name,
        &stbuf,
        b->size
    );
}

void ServerObject::fuse_ll_readdir(
    fuse_req_t              req,
    fuse_ino_t              ino,
    size_t                  size    __attribute__ ((unused)),
    off_t                   off     __attribute__ ((unused)),
    struct fuse_file_info   *fi     __attribute__ ((unused)))
{
    int i;
    char buffer[1024] = { 0x00 };

    vector<DomElement *> items;
    DomElement *element         = NULL;
    DomElement *parent_element  = NULL;
    DomElement *ipnoise         = NULL;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = server->getDocument()->getRootElement();
    assert(ipnoise);

    // search element
    snprintf(buffer, sizeof(buffer),
        "//*[@_ino=\"%x\"]",
        (unsigned int)ino
    );
    element = ipnoise->getElementByXpath(buffer);

    if (element){
        struct dirbuf b;
        memset(&b, 0, sizeof(b));

        int element_id          = element->getInoInt();
        int parent_element_id   = element->getInoInt();

        parent_element = element->getParentNode();
        if (parent_element){
            parent_element_id = parent_element->getInoInt();
        }

        items.clear();
        element->getChilds(items);

        dirbuf_add(req, &b, ".",            element_id);
        dirbuf_add(req, &b, "..",           parent_element_id);
        dirbuf_add(req, &b, ".attributes",  element_id + INODE_SUBTYPE_ATTRIBUTES);
        dirbuf_add(req, &b, ".log",         element_id + INODE_SUBTYPE_LOG);
        dirbuf_add(req, &b, ".xml",         element_id + INODE_SUBTYPE_XML);

        for (i = 0; i < (int)items.size(); i++){
            DomElement *cur_element     = items[i];
            string fuse_name            = cur_element->getFuseFullName();

            dirbuf_add(
                req,
                &b,
                fuse_name.c_str(),
                cur_element->getInoInt()
            );
        }

        fuse_reply_buf_limited(req, b.p, b.size, off, size);
        free(b.p);
    } else {
        fuse_reply_err(req, ENOTDIR);
    }
}

void ServerObject::fuse_ll_open(
    fuse_req_t              req,
    fuse_ino_t              ino,
    struct fuse_file_info   *fi)
{
    char buffer[1024]       = { 0x00 };
    int ino_subtype         = 0;
    int ino_element         = 0;
    DomElement *element     = NULL;
    DomElement *ipnoise     = NULL;
    FuseClient *fuse_client = NULL;

    vector <DomElement *> items;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = server->getDocument()->getRootElement();
    assert(ipnoise);

    // calculate node subtype
    ino_subtype = (ino - (int(ino / INODE_NEXT_ELEMENT_OFFSET)
        * INODE_NEXT_ELEMENT_OFFSET));

    // calculate inode number for element
    ino_element = (ino + 1) - ino_subtype;

    if (    ino_subtype != INODE_SUBTYPE_ATTRIBUTES
        &&  ino_subtype != INODE_SUBTYPE_LOG
        &&  ino_subtype != INODE_SUBTYPE_XML)
    {
        // look like is not file..
        goto error_is_dir;
    }

    if ((fi->flags & 3) != O_RDONLY){
        // must be read only access
        goto error_eaccess;
    }

    // search element
    snprintf(buffer, sizeof(buffer),
        "//*[@_ino=\"%x\"]",
        ino_element
    );

    element = ipnoise->getElementByXpath(buffer);
    if (!element){
        goto error_eaccess;
    }

    // open file handle
    fuse_client = element->openFuseClient();
    if (!fuse_client){
        PERROR("Cannot create new file handle\n");
        goto error_eaccess;
    }

    // set client type (inode type)
    fuse_client->setType(ino_subtype);

    // store
    fi->fh = (uint64_t)fuse_client;

    // force direct io for avoid kernel mind
    fi->direct_io = 1;

    // all ok
    fuse_reply_open(req, fi);

out:
    return;

error_is_dir:
    fuse_reply_err(req, EISDIR);
    goto out;

error_eaccess:
    fuse_reply_err(req, EACCES);
    goto out;

}

void ServerObject::fuse_ll_read(
    fuse_req_t              req,
    fuse_ino_t              ino,
    size_t                  size    __attribute__ ((unused)),
    off_t                   off     __attribute__ ((unused)),
    struct fuse_file_info   *fi     __attribute__ ((unused)))
{
    char buffer[1024]       = { 0x00 };
    DomElement *element     = NULL;
    int ino_subtype         = 0;
    int ino_element         = 0;
    int count               = 0;
    int res;
    string tmp;

    DomElement *ipnoise     = NULL;
    FuseClient *fuse_client = NULL;

    vector <DomElement *>           items;
    map <string, string>            attributes;
    map <string, string>::iterator  attributes_i;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = server->getDocument()->getRootElement();
    assert(ipnoise);

    fuse_client = (FuseClient *)fi->fh;

    // calculate node subtype
    ino_subtype = (ino - (int(ino / INODE_NEXT_ELEMENT_OFFSET)
        * INODE_NEXT_ELEMENT_OFFSET));

    // calculate inode number for element
    ino_element = (ino + 1) - ino_subtype;

    // search element
    snprintf(buffer, sizeof(buffer),
        "//*[@_ino=\"%x\"]",
        ino_element
    );

    element = ipnoise->getElementByXpath(buffer);
    if (!element){
        goto fail;
    }

    if (!element->isFuseClientOpen(fuse_client)){
        goto fail;
    }

    switch (ino_subtype){
        case (INODE_SUBTYPE_ATTRIBUTES):
            // get all element's attributes
            attributes = element->getAllAttributes();

            // init buffer ptr
            count = 0;
            for (attributes_i = attributes.begin();
                attributes_i != attributes.end();
                attributes_i++)
            {
                res = snprintf(buffer + count, sizeof(buffer) - count,
                    "%s=%s\n",
                    attributes_i->first.c_str(),
                    attributes_i->second.c_str()
                );
                if (res > 0){
                    count += res;
                } else {
                    break;
                }
            }

            fuse_reply_buf(req, buffer, count);
            element->closeFuseClient(fuse_client);
            break;

        case (INODE_SUBTYPE_LOG):
            // read from log
            element->processLog(fuse_client->getFd(), req);
            break;

        case (INODE_SUBTYPE_XML):
            tmp = element->serialize(1);
            tmp += "\n";
            fuse_reply_buf(req, tmp.c_str(), tmp.size());
            element->closeFuseClient(fuse_client);
            break;

        default:
            goto fail;
            break;
    };

out:
    return;

fail:

    fuse_reply_buf(req, NULL, 0);
    goto out;
}

void ServerObject::fuse_ll_release(
    fuse_req_t              req     __attribute__ ((unused)),
    fuse_ino_t              ino,
    struct fuse_file_info   *fi)
{

    char buffer[1024]       = { 0x00 };
    DomElement *element     = NULL;
    int ino_subtype         = 0;
    int ino_element         = 0;
    DomElement *ipnoise     = NULL;
    FuseClient *fuse_client = NULL;

    vector <DomElement *>           items;
    map <string, string>            attributes;
    map <string, string>::iterator  attributes_i;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = server->getDocument()->getRootElement();
    assert(ipnoise);

    fuse_client = (FuseClient *)fi->fh;

    // calculate node subtype
    ino_subtype = (ino - (int(ino / INODE_NEXT_ELEMENT_OFFSET)
        * INODE_NEXT_ELEMENT_OFFSET));

    // calculate inode number for element
    ino_element = (ino + 1) - ino_subtype;

    // search element
    snprintf(buffer, sizeof(buffer),
        "//*[@_ino=\"%x\"]",
        ino_element
    );

    element = ipnoise->getElementByXpath(buffer);
    if (!element){
        goto fail;
    }

    if (!element->isFuseClientOpen(fuse_client)){
        goto fail;
    }

    // close client
    element->closeFuseClient(fuse_client);

out:
    return;

fail:
    goto out;

}

void ServerObject::fuseSessionLoop(
    int     socket  __attribute__ ((unused)),
    short   event   __attribute__ ((unused)),
    void    *arg    __attribute__ ((unused)))
{
    int res = 0;
    struct fuse_session *fuse_se    = NULL;
    struct fuse_chan    *fuse_ch    = NULL;
    struct fuse_chan    *tmpch      = NULL;
    size_t bufsize                  = 0;
    char                *buff       = NULL;
//    ServerObject        *server     = NULL;

//    server = (ServerObject *)arg;
    assert(server);

    fuse_se = server->_fuse_se;
    fuse_ch = fuse_session_next_chan(fuse_se, NULL);
    bufsize = fuse_chan_bufsize(fuse_ch);
    tmpch   = fuse_ch;

    buff = (char *) malloc(bufsize);
    if (!buff) {
        PERROR_OBJ(server, "%s\n", "fuse: failed to allocate read buffer");
        goto fail;
    }

    res = fuse_chan_recv(&tmpch, buff, bufsize);
    if (res > 0){
        fuse_session_process(fuse_se, buff, res, tmpch);
    }

    free(buff);
    fuse_session_reset(fuse_se);

out:
    return;

fail:
    goto out;
}

int ServerObject::stopFuse()
{
    int err = 0;

    if (_fuse_ev){
        event_free(_fuse_ev);
        _fuse_ev = NULL;
    }
    if (_fuse_se && _fuse_ch){
        fuse_session_remove_chan(_fuse_ch);
    }
    if (_fuse_se){
        fuse_remove_signal_handlers(_fuse_se);
        fuse_session_destroy(_fuse_se);
        _fuse_se = NULL;
    }
    if (_fuse_ch){
        fuse_unmount(_fuse_mountpoint, _fuse_ch);
        _fuse_ch = NULL;
    }
    if (_fuse_args){
        fuse_opt_free_args(_fuse_args);
        _fuse_args = NULL;
    }

    return err;
}

int ServerObject::initFuse()
{
    const char *argv[3]     = { "ipnoise-router", FUSE_DIR, NULL };
    int argc                = 2;
    int fd = -1, err = -1;

    int res;
    string std_out, std_err;

    {
        // do umount before
        const char * const argv[] = {
            "/bin/umount",
            FUSE_DIR,
            NULL
        };
        res = my_system(argv, std_out, std_err);
    }

    // going init fuse
    _fuse_args = (struct fuse_args *)malloc(sizeof(struct fuse_args));
    if (!_fuse_args){
        PERROR_OBJ(this, "Malloc failed for 'struct fuse_args',"
            " was needed: '%d' bute(s)\n",
            sizeof(struct fuse_args)
        );
        goto fail;
    }

    *_fuse_args = FUSE_ARGS_INIT(argc, (char **)argv);

    res = fuse_parse_cmdline(_fuse_args, &_fuse_mountpoint, NULL, NULL);
    if (res == -1){
        PERROR_OBJ(this, "%s\n", "Cannot parse fuse cmdline");
        goto fail;
    }

    fuse_ll_oper.lookup    = ServerObject::fuse_ll_lookup;
    fuse_ll_oper.getattr   = ServerObject::fuse_ll_getattr;
    fuse_ll_oper.readdir   = ServerObject::fuse_ll_readdir;
    fuse_ll_oper.open      = ServerObject::fuse_ll_open;
    fuse_ll_oper.read      = ServerObject::fuse_ll_read;
    fuse_ll_oper.release   = ServerObject::fuse_ll_release;

    _fuse_ch = fuse_mount(_fuse_mountpoint, _fuse_args);
    if (!_fuse_ch){
        PERROR_OBJ(this, "Cannot mount fuse to mount point: '%s'\n", _fuse_mountpoint);
        goto fail;
    }

    _fuse_se = fuse_lowlevel_new(
        _fuse_args,
        &fuse_ll_oper,
        sizeof(fuse_ll_oper),
        NULL
    );
    if (!_fuse_se){
        PERROR_OBJ(this, "%s\n", "Cannot init fuse lowlevel");
        goto fail;
    }

    res = fuse_set_signal_handlers(_fuse_se);
    if (res == -1){
        PERROR_OBJ(this, "%s\n", "Cannot set fuse signal handlers");
        goto fail;
    }

    fuse_session_add_chan(_fuse_se, _fuse_ch);

    // create event handler
    fd = fuse_chan_fd(_fuse_ch);
    if (fd < 0){
        PERROR_OBJ(this, "Was received invalid file descriptor: '%d'"
            " from fuse_chan_fd()\n", fd);
        goto fail;
    }

    res = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (res == -1){
       PERROR_OBJ(this, "%s\n", "Cannot setup flags for fuse descriptor (O_NONBLOCK)");
       goto fail;
    }

    _fuse_ev = event_new(
        getDocument()->getEvBase(),
        fd,
        EV_READ | EV_PERSIST,
        ServerObject::fuseSessionLoop,
        (void *)this
    );
    if (_fuse_ev == NULL){
        PERROR_OBJ(this, "%s\n", "Cannot create new event");
        goto fail;
    }

    event_add(_fuse_ev, NULL);

    // all ok
    err = 0;

out:
    return err;

fail:
    stopFuse();

    if (!err){
        err = -1;
    }
    goto out;
}

int32_t ServerObject::autorun()
{
    int             err         = 0;
    int             res         = 0;
    int             priveleges  = 0;
    UsersObject     *users      = NULL;

    priveleges = getDocument()->getPriveleges();
    if (!(priveleges & DomDocument::USE_SYS_OBJECTS)){
        PERROR_OBJ(this,
            "autorun for '%s' element, document have not"
            " USE_SYS_OBJECTS priveleges, dump below:\n'%s'\n",
            getTagName().c_str(),
            dump().c_str()
        );
        goto fail;
    }

    // create container if not exist
    if (not isContainer()){
        setContainerVersion(0);
    }

    // create "ipn_sessions" element if not exist
    getCreateSessionsObject();

    // create "ipn_clients" element if not exist
    getCreateClientsObject();

    // create "ipn_neighs" object
    getCreateNeighsObject();

    // create 'ipn_links' element
    getCreateLinksObject();

    // create 'ipn_users' element
    users = getCreateUsersObject();

    // add broadcast user
    users->createUserByHuid(
        MULTICAST_USER_HUID,
        MULTICAST_USER_REAL_NICKNAME
    );

    // init commands
    command = new Command(this);
    command->init();

    res = initSignalPortTCP();
    if (res){
        PERROR_OBJ(this, "%s\n", "Cannot init TCP signal port");
        goto fail;
    }

    res = initSignalPortUDP();
    if (res){
        PERROR_OBJ(this, "%s\n", "Cannot init UDP signal port");
        goto fail;
    }

    res = initHostOsSignalPort();
    if (res){
        PERROR_OBJ(this, "%s\n", "Cannot init host OS signal port");
        goto fail;
    }

    /*
        // temporary disabled
        // 20120114 rchechnev@
        res = initFuse();
        if (res){
            PERROR_OBJ(this, "%s\n", "Cannot init fuse FS");
            goto fail;
        }
    */

    // set timer
    setTimer("+1");

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

SessionObject * ServerObject::getSession(
    const string &a_sessid)
{
    SessionObject *session = NULL;

    vector <DomElement *>               res;
    vector <DomElement *>::iterator     res_it;

    getElementsByXpath("//ipn_sessions", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        SessionsObject *sessions = NULL;
        sessions = (SessionsObject *)*res_it;

        session = sessions->getSession(a_sessid);
        if (session){
            break;
        }
    }

    return session;
}

SessionsObject * ServerObject::getCreateSessionsObject()
{
    SessionsObject *sessions = NULL;

    sessions = (SessionsObject*)getElementByTagName(
        "ipn_sessions"
    );

    if (not sessions){
        sessions = getDocument()
            ->createElement<SessionsObject>("ipn_sessions");
        appendChildSafe(sessions);
    }

    return sessions;
}

DomElement * ServerObject::getCreateClientsObject()
{
    DomElement *clients_object = NULL;

    clients_object = getElementByTagName(
        "ipn_clients"
    );

    if (not clients_object){
        clients_object = getDocument()->createElement("ipn_clients");
        appendChildSafe(clients_object);
    }

    return clients_object;
}

UsersObject * ServerObject::getCreateUsersObject()
{
    UsersObject * users_object = NULL;

    users_object = (UsersObject *)getElementByTagName("ipn_users");
    if (not users_object){
        users_object = getDocument()->createElement<UsersObject>(
            "ipn_users"
        );
        appendChildSafe(users_object);
    }

    return users_object;
}

// ---------------------- neighs --------------------------------------------

NeighObject * ServerObject::addNeigh(
    const string    &a_dst_huid,
    const string    &a_dev,
    const string    &a_lladdr,
    const string    &a_metric)
{
    NeighObject     *neigh  = NULL;
    NeighsObject    *neighs = NULL;

    neighs = getCreateNeighsObject();
    assert(neighs);

    neigh = neighs->addNeigh(
        a_dst_huid,
        a_dev,
        a_lladdr,
        a_metric
    );
    return neigh;
};

/*
 * add information about links, like:
 *
 * <links>
 *   <link name="udp_v4" addreess="192.168.1.1"/>
 * </links>
 *
 */
void ServerObject::addLinks(
    const string    &a_huid,
    DomElement      *a_out)
{
    DomElement *links = NULL;

    vector<NeighObject *>              neighs;
    vector<NeighObject *>::iterator    neighs_it;

    getNeighsByDstHuid(a_huid, neighs);
    for (neighs_it = neighs.begin();
        neighs_it != neighs.end();
        neighs_it++)
    {
        DomElement  *link  = NULL;
        NeighObject *neigh      = *neighs_it;

        if (!links){
            links = getDocument()->createElement("links");
            a_out->appendChildSafe(links);
        }

        link = getDocument()->createElement("link");
        link->setAttributeSafe("name",     neigh->getLinkName());
        link->setAttributeSafe("address",  neigh->getLinkAddress());
        links->appendChildSafe(link);
    }
}

void ServerObject::getNeighsByDstHuid(
    const string            &a_dst_huid,
    vector<NeighObject *>   &a_out,
    const int               &a_only_up)
{
    vector<NeighObject *>               res;
    vector<NeighObject *>::iterator     res_it;

    getNeighs(res, a_only_up);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        NeighObject *cur_neigh = *res_it;
        if (a_dst_huid != cur_neigh->getDstHuid()){
            continue;
        }
        a_out.push_back(cur_neigh);
    }
}

void ServerObject::getNeighs(
    vector<NeighObject *>   &a_out,
    const int               &a_only_up)
{
    vector<DomElement *>            res;
    vector<DomElement *>::iterator  res_it;
    NeighsObject *neighs_object = NULL;

    neighs_object = getCreateNeighsObject();
    neighs_object->getElementsByTagName("ipn_neigh", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        NeighObject *cur_neigh = (NeighObject *)(*res_it);
        if (    a_only_up
            &&  not cur_neigh->isUp())
        {
            continue;
        }
        a_out.push_back(cur_neigh);
    }
}

NeighsObject * ServerObject::getNeighsObject()
{
    NeighsObject *neighs_object = NULL;

    neighs_object = (NeighsObject *)getElementByTagName(
        "ipn_neighs"
    );

    return neighs_object;
}

NeighsObject * ServerObject::getCreateNeighsObject()
{
    NeighsObject *neighs_object = NULL;

    neighs_object = getNeighsObject();
    if (not neighs_object){
        neighs_object = getDocument()
            ->createElement<NeighsObject>("ipn_neighs");
        appendChildSafe(neighs_object);
    }

    return neighs_object;
}

HUID ServerObject::generateLocalHuid()
{
    HUID huid;
    char buffer[512] = {0x00};

    vector <DomElement *>           els;
    vector <DomElement *>::iterator els_it;
    map <uint32_t, int>             local_ids;
    map <uint32_t, int>::iterator   local_ids_it;

    int res;
    uint32_t id, id_found = 0;
    string id_str, huid_str;

    els.clear();
    getElementsByXpath("//ipn_item", els);
    for (els_it = els.begin();
        els_it != els.end();
        els_it++)
    {
        ItemObject  *item   = (ItemObject *)*els_it;
        HUID        huid    = item->getHuid();
        if (huid.isLocal()){
            uint32_t id = huid.getLocalId();
            local_ids[id] = 1;
        }
    }

    for (id = 0; id < (uint32_t(0) - 1); id++){
        local_ids_it = local_ids.find(id);
        if (local_ids.end() == local_ids_it){
            // free ID found
            id_found = 1;
            break;
        }
    }

    if (not id_found){
        PERROR("Cannot generate local HUID ID\n");
        goto fail;
    }

    // generate ID
    snprintf(buffer, sizeof(buffer), "%8.8x", id);
    id_str = buffer;

    // generate huid "2210:0000:0000:0000:0000:0000:0000:0000";
    huid_str =       "2210:0000:0000:0000:0000:0000";
    huid_str += ":" + id_str.substr(0, 4);
    huid_str += ":" + id_str.substr(4, 4);

    res = huid.setHuid(huid_str);
    if (res){
        PERROR("Cannot setup local HUID\n");
        goto fail;
    }

    huid.setLocal(true);

out:
    return huid;
fail:
    goto out;
}

void ServerObject::getAllItems(
    map<string, ItemObject *> &a_items)
{
    vector <DomElement *>           res;
    vector <DomElement *>::iterator res_it;

    res.clear();
    getElementsByXpath("//ipn_item", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        ItemObject  *ipn_item = (ItemObject *)*res_it;
        string      huid      = ipn_item->getHuid();

        a_items[huid] = ipn_item;
    }
}

ssize_t ServerObject::sendMsg(
    DomElement      *a_msg,
    const string    &a_dst_huid,
    const string    &a_src_huid,
    const int32_t   &a_delete_after_send)
{
    ssize_t res = -1;
    string content = a_msg->serializeForPublic();
    res = sendMsg(content, a_dst_huid, a_src_huid);

    if (a_delete_after_send){
        DomDocument::deleteElement(a_msg);
        a_msg = NULL;
    }
    return res;
}

ssize_t ServerObject::sendMsg(
    const string    &a_msg,
    const string    &a_dst_huid,
    const string    &a_src_huid)
{
    int32_t res;
    ssize_t ret     = -1;
    int32_t fd      = -1;

    struct  sockaddr_in6    sa_src;
    struct  sockaddr_in6    sa_dst;
    struct  protoent        *proto  = NULL;
    int32_t                 p_proto = -1;

    // convert from
    if (not a_src_huid.empty()){
        memset(&sa_src, 0x00, sizeof(sa_src));
        res = inet_pton(
            AF_INET6,
            a_src_huid.c_str(),
            &(sa_src.sin6_addr)
        );
        if (1 != res){
            PERROR("inet_pton for: '%s' failed\n",
                a_src_huid.c_str()
            );
            goto fail;
        }
        sa_src.sin6_family = AF_INET6;
        sa_src.sin6_port   = htons(0);
        sa_src.sin6_port   = if_nametoindex("lo");
    }

    // convert to
    if (not a_dst_huid.empty()){
        memset(&sa_dst, 0x00, sizeof(sa_dst));
        res = inet_pton(
            AF_INET6,
            a_dst_huid.c_str(),
            &(sa_dst.sin6_addr)
        );
        if (1 != res){
            PERROR("inet_pton for: '%s' failed\n",
                a_dst_huid.c_str()
            );
            goto fail;
        }
        sa_dst.sin6_family = AF_INET6;
        sa_dst.sin6_port   = htons(2210);
    }

    // search protocol
    proto = getprotobyname("udp");
    if (not proto){
        PERROR(
            "Cannot open proto by name: 'udp'\n"
        );
        goto fail;
    }
    p_proto = proto->p_proto;
    endprotoent();

    // socket
    fd = socket(AF_INET6, SOCK_DGRAM, p_proto);
    if (fd < 0){
        PERROR(
            "Cannot open socket(AF_INET6, SOCK_DGRAM, %d);\n",
            p_proto
        );
        goto fail;
    }

    // bind
    if (not a_src_huid.empty()){
        res = ::bind(
            fd,
            (struct sockaddr *)&sa_src,
            (socklen_t)sizeof(sa_src)
        );
        if (res){
            PERROR(
                "Cannot bind fd: '%d' to '%s'\n",
                    fd, a_src_huid.c_str()
            );
            goto fail;
        }
    }

    // send msg
    ret = sendto(
        fd,                                 // socket
        a_msg.c_str(),                      // data ptr
        a_msg.size(),                       // data size
        0,                                  // flags
        (const struct sockaddr *)&sa_dst,   // dst
        (socklen_t)sizeof(sa_dst)           // dst len
    );

    if (0 >= ret){
        PERROR("Failed send message:\n"
            " fd:       '%d'\n"
            " src_huid: '%s'\n"
            " dst_huid: '%s'\n"
            " msg:      '%s'\n"
            " was sent: '%d' byte(s)\n",
            fd,
            a_src_huid.c_str(),
            a_dst_huid.c_str(),
            a_msg.c_str(),
            ret
        );
        goto fail;
    }

    PDEBUG(20, "SessionItemObject::sendMsg:\n"
        " fd:       '%d'\n"
        " src_huid: '%s'\n"
        " dst_huid: '%s'\n"
        " msg:      '%s'\n"
        " was sent: '%d' byte(s)\n",
        fd,
        a_src_huid.c_str(),
        a_dst_huid.c_str(),
        a_msg.c_str(),
        ret
    );

out:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    return ret;

fail:
    if (ret >= 0){
       ret = -1;
    }
    goto out;
}

