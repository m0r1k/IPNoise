/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2012 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/ipnoiseObject.h"
#include "objects/usersObject.h"
#include "objects/sessionLoObject.h"

#include "objects/netCommandAndEventObject.h"

NetCommandAndEventObject * NetCommandAndEventObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new NetCommandAndEventObject(a_node, a_doc, a_tagname);
}

NetCommandAndEventObject::NetCommandAndEventObject(
    xmlNodePtr      _node,
    DomDocument     *_doc,
    const string    &_tagname)
    :   Object(_node, _doc, _tagname)
{
};

NetCommandAndEventObject::~NetCommandAndEventObject()
{
};

void NetCommandAndEventObject::setType(const string &type)
{
    setAttributeSafe("type", type);
}

string NetCommandAndEventObject::getType()
{
    return getAttribute("type");
}

int NetCommandAndEventObject::sendImmediately(
    SkBuffObject    *skb,
    string          &answer)
{
    int err = -1;
    ClientObject    *cl         = NULL;
    string          handler_ino = "";
    string          devname     = skb->getInDevName();

    // client handler "_ino" stored in "indev_index"
    handler_ino = skb->getInDevIndex();

    // get client
    cl = (ClientObject *)getDocument()->getElementByIno(handler_ino);
    if (!cl){
        PERROR_OBJ(this, "Cannot sendImmediately data, client not found\n");
        goto out;
    }

    cl->write(answer);

    // all ok
    err = 0;

out:
    return err;
}

// ------------------- work with sessions -----------------------------------80

string NetCommandAndEventObject::getSessId(SkBuffObject *skb)
{
    DomElement  *element    = NULL;
    DomElement  *ipnoise_el = NULL;
    string      sessid      = "";

    element = skb->getInDom();
    if (!element){
        goto out;
    }

    ipnoise_el = element->getElementByTagName("ipnoise");
    if (!ipnoise_el){
        goto out;
    }

    sessid = ipnoise_el->getAttribute("dst_sessid");

out:
    return sessid;
}

SessionObject * NetCommandAndEventObject::getSession(
    SkBuffObject    *skb,
    const string    &a_sessid)
{
    IPNoiseObject   *ipnoise        = NULL;
    ServerObject    *server         = NULL;
    SessionObject   *session        = NULL;
    DomElement      *indev_object   = NULL;
    string          indev_index     = "";
    string          sessid          = "";

    ipnoise = (IPNoiseObject *)getDocument()->getRootElement();
    if (!ipnoise){
        PERROR_OBJ(this, "Cannot get 'ipnoise' element\n");
        goto fail;
    }

    server = ipnoise->getServerObject();
    if (!server){
        PERROR_OBJ(this, "Cannot get 'ipnoise.server' element\n");
        goto fail;
    }

    // loopback?
    if ("lo" == skb->getInDevName()){
        string          from        = skb->getFrom();
        string          to          = skb->getTo();
        UserObject      *user       = NULL;
        ItemObject      *item       = NULL;
        SessionsObject  *sessions   = NULL;

        user = server->getUser(to);
        if (user){
            item = user->getItem(from);
        }
        if (item){
            sessions = item->getCreateSessionsObject();
        }
        if (sessions){
            session = sessions->getCreateSession("ipn_session_lo");
        }
        goto out;
    }

    if (not a_sessid.empty()){
        sessid = a_sessid;
    } else {
        sessid = getSessId(skb);
    }

    if (!sessid.size()){
        goto fail;
    }

    // get session
    session = server->getSession(sessid);

    // set sessid in client el
    indev_index     = skb->getInDevIndex();
    indev_object    = getDocument()->getElementByIno(indev_index);

    if (    indev_object
        &&  "ipn_client" == indev_object->getTagName()){
        // if it is "client" device, then associate sessid
        // with input device object
        ClientObject *client = (ClientObject *)indev_object;
        client->setSessId(sessid);
    }

out:
    return session;

fail:
    session = NULL;
    goto out;
}

int NetCommandAndEventObject::appendNeighsUp(
    DomElement *a_element) // where must be added new neighs
{
    vector <DomElement *>           neighs;
    vector <DomElement *>::iterator neighs_it;

    DomElement *neighs_el = NULL;

    if ("neighs" == a_element->getTagName()){
        neighs_el = a_element;
    } else {
        neighs_el = a_element->getDocument()->createElement("neighs");
        a_element->appendChildSafe(neighs_el);
    }

    // search neighs in state up and inform about it
    neighs.clear();
    a_element->getDocument()->getElementsByXpath(
        "//ipn_neighs/ipn_neigh[@state=\"up\"]",
        neighs
    );

    for (neighs_it = neighs.begin();
        neighs_it != neighs.end();
        neighs_it++)
    {
        NeighObject *ipn_neigh  = (NeighObject *)*neighs_it;
        DomElement  *neigh      = NULL;

        neigh = neighs_el->getDocument()->createElement("neigh");
        neigh->setAttributeSafe("lladdr",   ipn_neigh->getLLAddr());
        neigh->setAttributeSafe("dev",      ipn_neigh->getDev());
        neighs_el->appendChildSafe(neigh);
    }

    return int(neighs.size());
}

// ------------------- errors -----------------------------------------------80

int NetCommandAndEventObject::errorUnsupportedCommand(SkBuffObject *skb)
{
    int             err         = 0;
    SessionObject   *session    = NULL;
    string          answer      = "";

    // get session
    session = getSession(skb);
    if (not session){
        goto out;
    }

    // unsupported command
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer +=   ">";
    answer +=   "<events>";
    answer +=       "<event";
    answer +=           " ver=\"0.01\"";
    answer +=           " type=\"processCommandFailed\"";
    answer +=       ">";
    answer +=           "<description msg_id=\"processCommandFailed.1\">";
    answer +=               "unsupported command type";
    answer +=           "</description>";
    answer +=       "</event>";
    answer +=   "</events>";
    answer += "</ipnoise>";

    // return events
    session->queue(answer);

out:
    return err;
}

int NetCommandAndEventObject::errorSyntaxError(SkBuffObject *skb)
{
    int             err         = 0;
    SessionObject   *session    = NULL;
    string          answer      = "";

    // get session
    session = getSession(skb);
    if (not session){
        goto out;
    }

    // syntax error
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=   "<events>";
    answer +=       "<event";
    answer +=           " ver=\"0.01\"";
    answer +=           " type=\"processCommandFailed\"";
    answer +=       ">";
    answer +=           "<description msg_id=\"processCommandFailed.4\">";
    answer +=               "syntax error";
    answer +=           "</description>";
    answer +=       "</event>";
    answer +=   "</events>";
    answer += "</ipnoise>";

    // return events
    session->queue(answer);

out:
    return err;
}

int NetCommandAndEventObject::errorNotLogged(SkBuffObject *skb)
{
    int             err         = -1;
    SessionObject   *session    = NULL;
    string          answer      = "";

    // get session
    session = getSession(skb);
    if (not session){
        goto out;
    }

    // not logged
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer +=   ">";
    answer +=   "<events>";
    answer +=       "<event";
    answer +=           " ver=\"0.01\"";
    answer +=           " type=\"processCommandFailed\"";
    answer +=       ">";
    answer +=           "<description msg_id=\"processCommandFailed.3\">";
    answer +=               "you are not logged";
    answer +=           "</description>";
    answer +=       "</event>";
    answer +=   "</events>";
    answer += "</ipnoise>";

    // return events
    session->queue(answer);

out:
    return err;
}

int NetCommandAndEventObject::errorSessionFailed(SkBuffObject *skb)
{
    int     err     = 0;
    string  answer  = "";

    // session is not open or expired
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=   "<events>";
    answer +=       "<event";
    answer +=           " ver=\"0.01\"";
    answer +=           " type=\"processCommandFailed\"";
    answer +=       ">";
    answer +=           "<description msg_id=\"processCommandFailed.2\">";
    answer +=               "session was not open or expired";
    answer +=           "</description>";
    answer +=       "</event>";
    answer +=   "</events>";
    answer += "</ipnoise>";

    // return events
    sendImmediately(skb, answer);
    return err;
}

int NetCommandAndEventObject::errorInternalError(SkBuffObject *skb)
{
    int             err         = 0;
    SessionObject   *session    = NULL;
    string          answer      = "";

    // get session
    session = getSession(skb);
    if (not session){
        goto out;
    }

    // internal error
    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=   "<events>";
    answer +=       "<event";
    answer +=           " ver=\"0.01\"";
    answer +=           " type=\"processCommandFailed\"";
    answer +=       ">";
    answer +=           "<description msg_id=\"processCommandFailed.0\">";
    answer +=               "internal error";
    answer +=           "</description>";
    answer +=       "</event>";
    answer +=   "</events>";
    answer += "</ipnoise>";

    // return events
    session->queue(answer);

out:
    return err;
}

