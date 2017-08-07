/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Aug 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"

#include "apiCommandLinkSearchUsers.hpp"

ApiCommandLinkSearchUsers::ApiCommandLinkSearchUsers()
    :   ApiCommandLink(API_COMMAND_LINK_SEARCH_USERS)
{
}

ApiCommandLinkSearchUsers::~ApiCommandLinkSearchUsers()
{
}

void ApiCommandLinkSearchUsers::processNeigh(
    NetCommandObject    *nc,
    SkBuffObject        *skb,
    NeighObject         *neigh)
{
    map<string, ItemObject *>            items;
    map<string, ItemObject *>::iterator  items_it;

    IPNoiseObject   *ipnoise    = NULL;
    ServerObject    *server     = NULL;
    SkBuffObject    *new_skb    = NULL;
    DomElement      *out_dom    = NULL;
    DomElement      *ipnoise_el = NULL;
    DomElement      *events_el  = NULL;
    DomElement      *event_el   = NULL;
    DomElement      *users_el   = NULL;

    PDEBUG_OBJ(nc, 15, "searchUsers command was received\n");
    PDEBUG_OBJ(nc, 15, "from:        '%s'\n", skb->getAttribute("from").c_str());
    PDEBUG_OBJ(nc, 15, "from_ll:     '%s'\n", skb->getAttribute("from_ll").c_str());
    PDEBUG_OBJ(nc, 15, "to:          '%s'\n", skb->getAttribute("to").c_str());

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

    // get out dom
    out_dom = new_skb->getOutDom();

    // create ipnoise
    ipnoise_el = new_skb->getDocument()->createElement("ipnoise");
    out_dom->appendChildSafe(ipnoise_el);

    // create events
    events_el = new_skb->getDocument()->createElement("events");
    ipnoise_el->appendChildSafe(events_el);

    // create event
    event_el = new_skb->getDocument()->createElement("event");
    event_el->setAttributeSafe("type", "searchUsersAnswer");
    events_el->appendChildSafe(event_el);

    // add all "up" neighs to answer
    NetCommandObject::appendNeighsUp(event_el);

    // create users element
    users_el = new_skb->getDocument()->createElement("users");
    event_el->appendChildSafe(users_el);

    // get all items
    server->getAllItems(items);
    for (items_it = items.begin();
        items_it != items.end();
        items_it++)
    {
        DomElement  *user_el    = NULL;
        ItemObject  *item       = items_it->second;
        string      huid        = item->getHuid();

        if (MULTICAST_USER_HUID == huid){
            // skip multicast user
            continue;
        }

        user_el = new_skb->getDocument()->createElement("user");
        user_el->setAttributeSafe(
            "huid",
            huid
        );
        user_el->setAttributeSafe(
            "nickname",
            item->getAPINickName()
        );

        users_el->appendChildSafe(user_el);
    }

    new_skb->setAttributeSafe("to",         skb->getAttribute("from"));
    new_skb->setAttributeSafe("from",       skb->getAttribute("to"));
    new_skb->setAttributeSafe("to_ll",      neigh->getLLAddr());
    new_skb->delFromLL();

    // send answer to neigh device
    new_skb->setAttributeSafe("outdev_name",    neigh->getDev());
    new_skb->setAttributeSafe("outdev_index",   "-1");

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

int ApiCommandLinkSearchUsers::process(
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

    PDEBUG_OBJ(nc, 15, "searchUsers command was received\n");
    PDEBUG_OBJ(nc, 15, "from:        '%s'\n", skb->getAttribute("from").c_str());
    PDEBUG_OBJ(nc, 15, "from_ll:     '%s'\n", skb->getAttribute("from_ll").c_str());
    PDEBUG_OBJ(nc, 15, "to:          '%s'\n", skb->getAttribute("to").c_str());

    // get ipnoise
    ipnoise = ((IPNoiseObject *)nc->getDocument()->getRootElement());
    assert("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = ipnoise->getServerObject();

    // get neighs
    neighs = server->getCreateNeighsObject();

    // Searching additional neighs records
    // in skb and add are in IPList
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

