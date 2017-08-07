/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>
#include <map>

using namespace std;

class NetEventObject;

#include "objects/api/apiEvent.hpp"

typedef map<string, ApiEvent *>     NetEvents;
typedef NetEvents::iterator         NetEventsIt;

#ifndef NET_EVENT_OBJECT_H
#define NET_EVENT_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/netCommandAndEventObject.h"

class DomDocument;

class NetEventObject
    :   public NetCommandAndEventObject
{
    public:
        NetEventObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "event"
        );
        ~NetEventObject();

        // internal
        virtual NetEventObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "event"
        );

        // generic
        virtual int processEvent(SkBuffObject *skb);

    protected:
        void    registerEventsClient();
        void    registerEventsLink();
        void    registerEventsIPNoise();

};

#endif

