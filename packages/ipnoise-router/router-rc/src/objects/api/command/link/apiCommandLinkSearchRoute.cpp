/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Oct 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"

#include "apiCommandLinkSearchRoute.hpp"

ApiCommandLinkSearchRoute::ApiCommandLinkSearchRoute()
    :   ApiCommandLink(API_COMMAND_LINK_SEARCHROUTE)
{
}

ApiCommandLinkSearchRoute::~ApiCommandLinkSearchRoute()
{
}

void ApiCommandLinkSearchRoute::processNeigh(
    NetCommandObject    *nc,
    SkBuffObject        *skb,
    NeighObject         *neigh)
{
    IPNoiseObject   *ipnoise    = NULL;
    ServerObject    *server     = NULL;
    SkBuffObject    *new_skb    = NULL;
    DomElement      *ipnoise_el = NULL;
    DomElement      *out_dom    = NULL;
    DomElement      *events     = NULL;
    DomElement      *event      = NULL;

    string  id = nc->getAttribute("id");

    // get ipnoise
    ipnoise = ((IPNoiseObject *)nc->getDocument()->getRootElement());
    assert("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = ipnoise->getServerObject();

    // prepare answer, create new skb
    new_skb = server->getDocument()->createElement<SkBuffObject>(
        "ipn_skbuff"
    );
    if (!new_skb){
        PERROR_OBJ(nc, "%s\n", "Cannot allocate new skb");
        goto fail;
    }

    // prepare fields
    //skb->dump2stderr(this, "test\n");
    new_skb->setAttributeSafe("to",         skb->getAttribute("from"));
    new_skb->setAttributeSafe("from",       skb->getAttribute("to"));
    new_skb->setAttributeSafe("to_ll",      neigh->getLLAddr());
    new_skb->delFromLL();

    // send answer to neigh device
    new_skb->setAttributeSafe("outdev_name",    neigh->getDev());
    new_skb->setAttributeSafe("outdev_index",   "-1");

    // get out dom
    out_dom     = new_skb->getOutDom();
    ipnoise_el  = new_skb->getDocument()->createElement("ipnoise");
    out_dom->appendChildSafe(ipnoise_el);

    // create events
    events = out_dom->getDocument()->createElement("events");
    ipnoise_el->appendChildSafe(events);

    // create event
    event = out_dom->getDocument()->createElement("event");
    event->setAttributeSafe("type", "searchRouteAnswer");
    if (not id.empty()){
        event->setAttributeSafe("id", id);
    }

    // add all "up" neighs to answer
    NetCommandObject::appendNeighsUp(event);
    events->appendChildSafe(event);

    // send packet
    new_skb->prepare();
    server->getDocument()->emitSignalAndDelObj(
        (const char *)"packet_outcoming",
        new_skb
    );

out:
    return;

fail:
    goto out;
}

int ApiCommandLinkSearchRoute::process(
    NetCommandObject    *nc,
    SkBuffObject        *skb)
{
    int i, err = 0;
    char buffer[1024] = { 0x00 };

    vector<DomElement *> items;

    IPNoiseObject   *ipnoise    = NULL;
    ServerObject    *server     = NULL;
    NeighsObject    *neighs     = NULL;
    NeighObject     *neigh      = NULL;
    UsersObject     *users      = NULL;
    UserObject      *user       = NULL;
    ItemObject      *item       = NULL;
    string          tmp_str;

    PDEBUG_OBJ(nc, 15, "searchRoute command was received\n");
    PDEBUG_OBJ(nc, 15, "from:        '%s'\n", skb->getAttribute("from").c_str());
    PDEBUG_OBJ(nc, 15, "from_ll:     '%s'\n", skb->getAttribute("from_ll").c_str());
    PDEBUG_OBJ(nc, 15, "to:          '%s'\n", skb->getAttribute("to").c_str());

    // get ipnoise
    ipnoise = ((IPNoiseObject *)nc->getDocument()->getRootElement());
    assert("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = ipnoise->getServerObject();

    // get users
    users = server->getCreateUsersObject();

    // get user
    tmp_str = skb->getAttribute("to");
    user    = users->getUserByHuid(tmp_str);
    if (!user){
        PERROR("Cannot found user with huid: '%s'\n",
            skb->getAttribute("to").c_str());
        goto out;
    }

    // get item
    item = user->getItem(skb->getAttribute("from"));
    if (!item){
        PERROR("Cannot found item with huid: '%s',"
            " for user with huid: '%s'\n",
            skb->getAttribute("from").c_str(),
            skb->getAttribute("to").c_str()
        );
        goto out;
    }

    // get neighs
    neighs = server->getCreateNeighsObject();

    // Searching additional neighs records in skb and add are in IPList
    neighs->addNeighsFromSkb(skb);

    // ok, here is our strategy:
    // 1. if we have at least one "ipn_neigh" record with status "up",
    // then we send answer only thru it for trafic economy,
    // else we send answer to all "ipn_neigh"
    // 20110117 morik@
    snprintf(buffer, sizeof(buffer),
        "//ipn_neigh[@state=\"up\" and @dst_huid=\"%s\"]",
        skb->getAttribute("from").c_str()
    );
    items.clear();
    neighs->getElementsByXpath(buffer, items);
    if (items.size()){
        neigh = (NeighObject *)items[0];
    }

    if (neigh){
        // send only via active neigh
        processNeigh(nc, skb, neigh);
        goto out;
    }

    // send via every neighs
    snprintf(buffer, sizeof(buffer),
        "//ipn_neigh[@dst_huid=\"%s\"]",
        skb->getAttribute("from").c_str()
    );
    items.clear();
    neighs->getElementsByXpath(buffer, items);
    for (i = 0; i < (int)items.size(); i++){
        neigh = (NeighObject *)items[i];
        processNeigh(nc, skb, neigh);
    }

    // all ok
    err = 0;

out:
    return err;
}

