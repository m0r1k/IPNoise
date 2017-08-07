/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Aug 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "hmac-md5/gen-huid.h"
#include "libxml2/domDocument.h"

#include "apiCommandClientSearchUsers.hpp"

ApiCommandClientSearchUsers::ApiCommandClientSearchUsers()
    :   ApiCommandClient(API_COMMAND_CLIENT_SEARCH_USERS)
{
}

ApiCommandClientSearchUsers::~ApiCommandClientSearchUsers()
{
}

int ApiCommandClientSearchUsers::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int err = 0;
    UserObject      *user           = NULL;
    SessionObject   *session        = NULL;
    string          sessid          = "";
    string          link_name       = "";
    string          link_addr       = "";
    string          answer          = "";

    vector <DomElement *>           items;
    vector <DomElement *>::iterator items_i;

    // search session
    session = nc->getSession(skb);
    if (!session){
        err = nc->errorSessionFailed(skb);
        goto out;
    }

    // get session ID
    sessid = session->getSessId();

    // search user for this session
    user = session->getOwner();
    if (!user){
        err = nc->errorNotLogged(skb);
        goto out;
    }

    // get link name
    link_name = nc->getAttribute("link_name");
    if (!link_name.size()){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    // get link addr
    link_addr = nc->getAttribute("link_addr");
    if (!link_addr.size()){
        err = nc->errorSyntaxError(skb);
        goto out;
    }

    {
        SkBuffObject *skb = NULL;
        skb = nc->getDocument()->createElement<SkBuffObject>(
            "ipn_skbuff"
        );
        skb->setFrom(user->getHuid());
        // multicast
        skb->setTo(MULTICAST_USER_HUID);
        skb->setToLL(link_addr);
        skb->setOutDevName(link_name + "_0");

        DomElement *ipnoise  = NULL;
        DomElement *command  = NULL;
        DomElement *commands = NULL;
        DomElement *out_dom  = NULL;

        out_dom = skb->getOutDom(1);

        ipnoise = skb->getDocument()->createElement("ipnoise");
        out_dom->appendChildSafe(ipnoise);

        command = skb->getDocument()->createElement("command");
        command->setAttributeSafe("type", "searchUsers");
        commands = skb->getDocument()->createElement("commands");
        commands->appendChildSafe(command);
        ipnoise->appendChildSafe(commands);

        // append "up" neighs
        NetCommandObject::appendNeighsUp(command);

        // send packet,
        // NOTE: packet will be deleted after sendPacket call
        // tx packet (emitSignal will create clone of element)
        skb->prepare();
        nc->getDocument()->emitSignalAndDelObj(
            "packet_outcoming",
            skb
        );
    }

out:
    return err;
}

