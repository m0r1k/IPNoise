/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"

#include "objects/serverObject.h"
#include "objects/neighObject.h"
#include "objects/neighsObject.h"
#include "objects/netCommandObject.h"
#include "objects/ipnoiseObject.h"

#include "apiEventLinkSearchRouteAnswer.hpp"

ApiEventLinkSearchRouteAnswer::ApiEventLinkSearchRouteAnswer()
    :   ApiEventLink(API_EVENT_LINK_SEARCHROUTE_ANSWER)
{
}

ApiEventLinkSearchRouteAnswer::~ApiEventLinkSearchRouteAnswer()
{
}

int ApiEventLinkSearchRouteAnswer::process(
    NetEventObject  *nc,
    SkBuffObject    *skb)
{
    int err = -1;
    char buffer[1024];
    string          id = nc->getAttribute("id");
    NeighObject     *neigh              = NULL;
    DomElement      *checking           = NULL;
    IPNoiseObject   *ipnoise            = NULL;
    NeighsObject    *neighs             = NULL;
    int32_t         uptime              = 0;
    int32_t         recheck_time        = 0;

    ipnoise = (IPNoiseObject *)nc->getDocument()->getRootElement();
    assert("ipn_ipnoise" == ipnoise->getTagName());

    PDEBUG_OBJ(nc, 15, "searchRouteAnswer event was received\n");
    PDEBUG_OBJ(nc, 15, "from:        '%s'\n", skb->getFrom().c_str());
    PDEBUG_OBJ(nc, 15, "from_ll:     '%s'\n", skb->getFromLL().c_str());
    PDEBUG_OBJ(nc, 15, "to:          '%s'\n", skb->getTo().c_str());

    if (id.empty()){
        goto ret;
    }

    snprintf(buffer, sizeof(buffer),
        "//ipn_neighs/"
        "ipn_neigh/"
        "ipn_neigh_checkings"
        "/ipn_neigh_checking[@id=\"%s\"]",
        id.c_str()
    );

    checking = ipnoise->getElementByXpath(buffer);
    if (!checking){
        goto ret;
    }

    // mark what answer was received
    checking->setAttributeSafe("answer_was_received", "true");

    // get parent
    neigh = (NeighObject *)checking->getParentNodeByTagName(
        "ipn_neigh"
    );
    if (not neigh){
        PERROR_OBJ(nc, "%s\n",
            "Internal error, parent tag must be 'ipn_neigh'"
        );
        goto ret;
    }

    // get "ipn_neighs"
    neighs = (NeighsObject *)neigh->getParentNode();

    // recalculate uptime
    uptime          =  neigh->getUpTime();
    recheck_time    =  neigh->getRecheckTime();
    uptime          += recheck_time;
    neigh->setUpTime(uptime);

    // use current source huid for all next checkings of this neigh
    neigh->setSrcHuid(checking->getAttribute("src_huid"));

    // no we have up state
    neigh->setStateUp();

    // add neighs from skb
    neighs->addNeighsFromSkb(skb);

    // all ok
    err = 0;

ret:
    return err;
}

