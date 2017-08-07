/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Dec 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>

class AcceptedClientObject;

using namespace std;

#ifndef ACCEPTED_CLIENT_OBJECT_H
#define ACCEPTED_CLIENT_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/ioObject.h"
class DomDocument;

#include "objects/clientObject.h"
#include "objects/sessionItemObject.h"

class AcceptedClientObject
    :   public  IoObject
{
    public:
        AcceptedClientObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_accepted_client"
        );
        virtual ~AcceptedClientObject();

        // internal
        virtual AcceptedClientObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_accepted_client"
        );

        // generic
        SessionItemObject * getSessionItemObject();
        void                getClients(vector<ClientObject *> &clients);
        int                 write(const string &a_data);
        bool                isConnected();
};

#endif

