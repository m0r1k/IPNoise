/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Aug 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include "libxml2/domDocument.h"

#include "objects/serverObject.h"
#include "objects/neighObject.h"
#include "objects/neighsObject.h"
#include "objects/netCommandObject.h"
#include "objects/ipnoiseObject.h"

#include "apiEventLinkSearchUsersAnswer.hpp"

ApiEventLinkSearchUsersAnswer::ApiEventLinkSearchUsersAnswer()
    :   ApiEventLink(API_EVENT_LINK_SEARCH_USERS_ANSWER)
{
}

ApiEventLinkSearchUsersAnswer::~ApiEventLinkSearchUsersAnswer()
{
}

int ApiEventLinkSearchUsersAnswer::process(
    NetEventObject  *nc,
    SkBuffObject    *skb)
{
    int             err         = -1;
    IPNoiseObject   *ipnoise    = NULL;
    ServerObject    *server     = NULL;
    UserObject      *user       = NULL;
    DomElement      *users_el   = NULL;
    DomElement      *neighs_el  = NULL;

    vector<DomElement *>            res;
    vector<DomElement *>::iterator  res_it;

    string answer;
    string indev;
    string from_lladdr;

    ipnoise = (IPNoiseObject *)nc->getDocument()->getRootElement();
    assert("ipn_ipnoise" == ipnoise->getTagName());

    server = ipnoise->getServerObject();

    PDEBUG_OBJ(nc, 15, "searchUsersAnswer event was received\n");
    PDEBUG_OBJ(nc, 15, "from:    '%s'\n", skb->getFrom().c_str());
    PDEBUG_OBJ(nc, 15, "from_ll: '%s'\n", skb->getFromLL().c_str());
    PDEBUG_OBJ(nc, 15, "to:      '%s'\n", skb->getTo().c_str());

    // collect "from" info
    indev       = skb->getInDevName();
    from_lladdr = skb->getFromLL();

    neighs_el = nc->getElementByTagName("neighs");
    if (neighs_el){
        res.clear();
        neighs_el->getElementsByTagName("neigh", res);
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            DomElement  *neigh_el = *res_it;
            string      huid;
            string      lladdr;
            string      dev;

            // get neigh info
            huid    = neigh_el->getAttribute("huid");
            lladdr  = neigh_el->getAttribute("lladdr");
            dev     = neigh_el->getAttribute("dev");

            // do add if not empty
            if (    huid.size()
                &&  lladdr.size()
                &&  dev.size())
            {
                server->getNeighsObject()->addNeigh(
                    huid,
                    dev,
                    lladdr
                );
            }
        }
    }

    // search user
    user = server->getUser(skb->getTo());
    if (not user){
        goto out;
    }

    users_el = nc->getElementByTagName("users");
    if (not users_el){
        goto out;
    }

    answer  = "<ipnoise";
    answer +=   " ver=\"0.01\"";
    answer += ">";
    answer +=   "<events>";
    answer +=       "<event";
    answer +=           " type=\"searchUsersAnswer\"";
    answer +=           " ver=\"0.01\"";
    answer +=       ">";
    answer +=           "<users>";
    res.clear();
    users_el->getElementsByTagName("user", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        ItemObject  *item       = NULL;
        DomElement  *user_el    = *res_it;
        string      huid        = user_el->getAttribute("huid");
        string      nickname    = user_el->getAttribute("nickname");
        answer += "<user";
        answer +=   " huid=\""      + huid      + "\"";
        answer +=   " nickname=\""  + nickname  + "\"";
        answer += "/>";
        // update/create item
        item = user->getCreateItem(huid);
        item->setAPIRealNickName(nickname);
    }
    answer +=           "</users>";
    answer +=       "</event>";
    answer +=   "</events>";
    answer += "</ipnoise>";

    user->queue(answer);

    // all ok
    err = 0;

out:
    return err;
}

