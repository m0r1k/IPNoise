/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep, 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

class NeighsObject;

using namespace std;

#ifndef NEIGHS_OBJECT_H
#define NEIGHS_OBJECT_H

#include <ipnoise-common/log.h>
#include "objects/object.h"

class DomDocument;

#include "objects/skBuffObject.h"
#include "objects/neighObject.h"
#include "objects/itemObject.h"

class NeighsObject
    :   public Object
{
    public:
        NeighsObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_neighs"
        );
        ~NeighsObject();

        // internal
        virtual NeighsObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_neighs"
        );
        int32_t         autorun();
        virtual void    registrySignals();
        virtual void    registrySlots();
        virtual void    slot(
            const string    &a_signal_name,
            DomElement      *object
        );

        // generic
        void            getNeighs(
            vector<NeighObject *>   &a_out,
            const string            &a_huid = ""
        );
        NeighObject *   getNeigh(
            const string    &a_dst_huid,
            const string    &a_dev,
            const string    &a_lladdr
        );
        void getOnlineNeighs(
            vector<NeighObject *>   &a_out,
            const string            &a_huid = ""
        );

        NeighObject *   addNeigh(
            const string    &a_dst_huid,
            const string    &a_dev,
            const string    &a_lladdr,
            const string    &a_metric = "0"
        );
        NeighObject *   addNeigh(
            DomElement      *skb,       // Element for search "ipnoise"
            const string    &huid,       // Item neigh for
            const string    &lladdr,     // Link address
            const string    &dev,        // Link device
            const string    &metric      // Metric
        );
        void updateIPList(SkBuffObject *skb);
        void addNeighsFromSkb(SkBuffObject *incoming_skb);
};

#endif

