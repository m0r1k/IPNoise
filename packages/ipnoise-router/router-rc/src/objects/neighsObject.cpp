/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep, 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <ipnoise-common/log.h>
#include <ipnoise-common/system.h>

#include "libxml2/domDocument.h"
#include "objects/neighObject.h"
#include "objects/serverObject.h"
#include "objects/ipnoiseObject.h"

#include "objects/neighsObject.h"

NeighsObject * NeighsObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new NeighsObject(a_node, a_doc, a_tagname);
}

NeighsObject::NeighsObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   Object(a_node, a_doc, a_tagname)
{
};

NeighsObject::~NeighsObject()
{
};

int32_t NeighsObject::autorun()
{
    int32_t     err         = 0;
    int32_t     priveleges  = 0;

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

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}


/*  addNeighsFromSkb - Searching "neighs" elements in incoming skb,
 *      and add it in IPList
 *
 *  >> SkBuffObject  - incoming skb
 *
 */
void NeighsObject::addNeighsFromSkb(SkBuffObject *incoming_skb)
{
    char buffer[1024];
    vector <DomElement *> neighs;
    vector <DomElement *> items;
    NeighObject *neigh = NULL;
    int i;

    ServerObject    *server     = NULL;
    IPNoiseObject   *ipnoise    = NULL;

    string indev_name   = incoming_skb->getInDevName();
    string from_huid    = incoming_skb->getFrom();
    string from_ll      = incoming_skb->getFromLL();

    if (    from_huid.size()
        &&  indev_name.size()
        &&  from_ll.size())
    {
        addNeigh(from_huid, indev_name, from_ll);
    }

    // get ipnoise
    ipnoise = (IPNoiseObject *)getDocument()->getRootElement();
    assert ("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = (ServerObject *)ipnoise->getServerObject();
    assert ("ipn_server" == server->getTagName());

    // search additional neighs in answer
    snprintf(buffer, sizeof(buffer), "//neigh");

    neighs.clear();
    incoming_skb->getInDom()->getElementsByXpath(buffer, neighs);
    if (neighs.size()){
        // check what we have src and dst users
        snprintf(buffer, sizeof(buffer),
            "//ipn_users/ipn_user[@huid=\"%s\"]"
            "/ipn_items/ipn_item[@huid=\"%s\"]",
            incoming_skb->getAttribute("to").c_str(),
            incoming_skb->getAttribute("from").c_str()
        );
        items.clear();
        ipnoise->getElementsByXpath(buffer, items);
        if (!items.size()){
            // we have not such pair: user-item
            goto ret;
        }
        if (items.size() > 0){
            PWARN_OBJ(server,
                "Duplicate items was found while: '%s'\n",
                buffer
            );
        }
    }
    for (i = 0; i < (int)neighs.size(); i++){
        neigh = (NeighObject *)neighs[i];
        assert(neigh);
        addNeigh(
            incoming_skb,                       // Element for search "ipnoise"
            incoming_skb->getAttribute("from"), // Item
            neigh->getLLAddr(),                 // Link address
            neigh->getDev(),                    // Link device
            "1"                                 // metric
       );
    }
ret:
    return;
}

void NeighsObject::getNeighs(
    vector<NeighObject *>   &a_out,
    const string            &a_huid)
{
    vector<DomElement *>           res;
    vector<DomElement *>::iterator res_it;

    getElementsByTagName("ipn_neigh", res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        NeighObject *neigh = (NeighObject *)*res_it;
        if (    a_huid.size()
            &&  a_huid != neigh->getDstHuid())
        {
            continue;
        }
        a_out.push_back(neigh);
    }
}

void NeighsObject::getOnlineNeighs(
    vector<NeighObject *>   &a_out,
    const string            &a_huid)
{
    vector<NeighObject *>               neighs;
    vector<NeighObject *>::iterator     neighs_it;

    getNeighs(neighs, a_huid);

    for (neighs_it = neighs.begin();
        neighs_it != neighs.end();
        neighs_it++)
    {
        NeighObject *neigh = *neighs_it;
        if (not neigh->isUp()){
            continue;
        }
        a_out.push_back(neigh);
    }
}

NeighObject *NeighsObject::getNeigh(
    const string    &a_dst_huid,
    const string    &a_dev,
    const string    &a_lladdr)
{
    NeighObject *ret = NULL;
    vector <DomElement *>           items;
    vector <DomElement *>::iterator items_i;

    getElementsByTagName("ipn_neigh", items);

    for (items_i = items.begin();
        items_i != items.end();
        items_i++)
    {
        NeighObject *neigh = (NeighObject *)*items_i;
        if (    (neigh->getDstHuid() == a_dst_huid)
            &&  (neigh->getDev()     == a_dev)
            &&  (neigh->getLLAddr()  == a_lladdr))
        {
            ret = neigh;
            break;
        }
    }

    return ret;
}

NeighObject *NeighsObject::addNeigh(
    const string    &a_dst_huid,
    const string    &a_dev,
    const string    &a_lladdr,
    const string    &a_metric)
{
    NeighObject *neigh = NULL;

    string std_out, std_err;

    if (    a_dst_huid.empty()
        ||  a_dev.empty()
        ||  a_lladdr.empty())
    {
        // huid, dev name or lladdr is empty
        goto out;
    }

    if (MULTICAST_USER_HUID == a_dst_huid){
        // it is multicast huid, skip
        goto out;
    }

    neigh = getNeigh(a_dst_huid, a_dev, a_lladdr);
    if (neigh){
        // neigh already exist
        goto out;
    }

    neigh = getDocument()->createElement<NeighObject>("ipn_neigh");
    if (    neigh
        &&  "tcp" == a_dev.substr(0, 3))
    {
        string::size_type pos = string::npos;
        // don't save "tcp" neighs after exit
        // (except 22102 port)
        pos = a_lladdr.find(":22102");
        if (string::npos == pos){
            neigh->setExpired("0");
        }
    }

    neigh->setDstHuid(a_dst_huid);
    neigh->setMetric(a_metric);
    neigh->setLLAddr(a_lladdr);
    neigh->setDev(a_dev);

    appendChild(neigh);

out:
    return neigh;
}

NeighObject *NeighsObject::addNeigh(
    DomElement      *, // skb   // Element for search "ipnoise"
    const string    &huid,      // Item neigh for
    const string    &lladdr,    // Link address
    const string    &dev,       // Link device
    const string    &metric)    // Metric
{
    NeighObject *neigh = NULL;

    if (    huid.empty()
        ||  lladdr.empty()
        ||  dev.empty())
    {
        goto out;
    }

    if (MULTICAST_USER_HUID == huid){
        // it is multicast huid, skip
        goto out;
    }

    // check what neigh not exist
    neigh = getNeigh(huid, dev, lladdr);
    if (neigh){
        //  neigh already exist
        goto out;
    }

    // add new neigh record
    neigh = addNeigh(huid, dev, lladdr, metric);

out:
    return neigh;
}

void NeighsObject::updateIPList(SkBuffObject *skb)
{
    vector <DomElement *> users;
    vector <DomElement *> res;
    vector <DomElement *> items;

    NeighObject *neigh          = NULL;
    string      cur_time_str    = getServer()->getCurTimeStr();

    if (skb->getAttribute("indev_name") == CLIENTS_DEV_NAME){
        // it's client's command
        goto out;
    }

    if (skb->getAttribute("indev_name") == IPNOISE_DEV_NAME){
        // it's ipnoise clients command
        goto out;
    }

    neigh = addNeigh(
        skb,                                // Element for search "ipnoise"
        skb->getAttribute("from"),          // Item
        skb->getAttribute("from_ll"),       // Link address
        skb->getAttribute("indev_name"),    // Link device
        "0"                                 // metric
    );

    if (neigh){
        neigh->setLastRecv(cur_time_str);
    }

out:
    return;
}

void NeighsObject::registrySignals()
{
    getDocument()->registrySignal(
        "best_neigh_found",
        "ipn_neigh"
    );
    getDocument()->registrySignal(
        "new_route_found",
        "ipn_neigh"
    );
}

void NeighsObject::registrySlots()
{
    getDocument()->registrySlot(
        this,
        "packet_incoming",
        "ipn_skbuff"
    );
    getDocument()->registrySlot(
        this,
        "neigh_up",
        "ipn_neigh"
    );
    getDocument()->registrySlot(
        this,
        "neigh_down",
        "ipn_neigh"
    );
}

void NeighsObject::slot(
    const string    &a_signal_name,
    DomElement      *a_object)
{
    string          signal_name     = a_signal_name;
    string          object_type     = "";
    ServerObject    *server         = NULL;
    IPNoiseObject   *ipnoise        = NULL;

    assert(a_object != NULL);
    // get object name
    object_type = a_object->getTagName();

    // get ipnoise
    ipnoise = (IPNoiseObject *)getDocument()->getRootElement();
    assert ("ipn_ipnoise" == ipnoise->getTagName());

    // get server
    server = (ServerObject *)ipnoise->getServerObject();
    assert ("ipn_server" == server->getTagName());

    if (signal_name == "packet_incoming"){
        if (object_type == "ipn_skbuff"){
            NeighsObject::updateIPList(
                (SkBuffObject *)a_object
            );
        }
    } else if ( "neigh_up"  == signal_name
        &&      "ipn_neigh" == object_type)
    {
        // search best neigh and setup route
        vector<NeighObject *>           neighs;
        vector<NeighObject *>::iterator neighs_it;
        NeighObject *neigh      = NULL;
        NeighObject *best_neigh = NULL;

        // get object
        neigh = (NeighObject *)a_object;
        neigh->setInNeighTable();

        PDEBUG(10, "neigh up:\n"
            "   src_huid:   '%s'\n"
            "   dst_huid:   '%s'\n"
            "   lladdr:     '%s'\n"
            "   dev:        '%s'\n",
            neigh->getSrcHuid().c_str(),
            neigh->getDstHuid().c_str(),
            neigh->getLLAddr().c_str(),
            neigh->getDev().c_str()
        );

        // search best neigh
        getNeighs(neighs);
        for (neighs_it = neighs.begin();
            neighs_it != neighs.end();
            neighs_it++)
        {
            NeighObject *cur_neigh = *neighs_it;

            if (!cur_neigh->isUp()){
                // neigh is not up
                continue;
            }

            if (cur_neigh->getDstHuid() != neigh->getDstHuid()){
                // not our destination
                continue;
            }

            if (!best_neigh){
                best_neigh = cur_neigh;
                continue;
            }

            if ((*cur_neigh) > (*best_neigh)){
                best_neigh = cur_neigh;
            }
        }

        if (best_neigh){
            string huid    = best_neigh->getDstHuid();
            string lladdr  = best_neigh->getLLAddr();
            string dev     = best_neigh->getDev();

            PDEBUG(10, "best neigh:\n"
                "   src_huid:   '%s'\n"
                "   dst_huid:   '%s'\n"
                "   lladdr:     '%s'\n"
                "   dev:        '%s'\n",
                best_neigh->getSrcHuid().c_str(),
                best_neigh->getDstHuid().c_str(),
                best_neigh->getLLAddr().c_str(),
                best_neigh->getDev().c_str()
            );

            // setup route via best neigh
            best_neigh->setInNeighTable();
            best_neigh->setInRouteTable();

            PDEBUG_OBJ(this, 15,
                "setup new route to huid: '%s' via: '%s' dev: '%s'\n",
                huid.c_str(),
                lladdr.c_str(),
                dev.c_str()
            );

            // inform about new route
            getDocument()->emitSignal(
                (const char *)"new_route_found",
                best_neigh
            );
        }
    } else if ( "neigh_down" == signal_name
        &&      "ipn_neigh"  == object_type)
    {
        NeighObject *neigh = NULL;

        neigh = (NeighObject *)a_object;
        neigh->unsetInNeighTable();

        PDEBUG(10, "neigh down:\n"
            "   src_huid:   '%s'\n"
            "   dst_huid:   '%s'\n"
            "   lladdr:     '%s'\n"
            "   dev:        '%s'\n",
            neigh->getSrcHuid().c_str(),
            neigh->getDstHuid().c_str(),
            neigh->getLLAddr().c_str(),
            neigh->getDev().c_str()
        );

        // if there are no more up neighs to this destination,
        // then showdown route
        {
            vector<NeighObject *> neighs;
            server->getNeighsByDstHuid(
                neigh->getDstHuid(),    // dst huid
                neighs,                 // result
                1                       // only UP
            );
            if (not neighs.size()){
                neigh->unsetInRouteTable();
            }
        }
    }
}

