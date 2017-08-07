/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Sep 2011 (c)
 *  <morik@ipnoise.ru>
 *
 */

#include <string>

class SessionItemObject;

using namespace std;

#ifndef SESSION_ITEM_OBJECT_H
#define SESSION_ITEM_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/sessionObject.h"

class DomDocument;

#include "objects/netClientHttpObject.h"
#include "objects/acceptedClientObject.h"

class SessionItemObject
    :   public SessionObject
{
    public:
        SessionItemObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_session_item"
        );
        virtual ~SessionItemObject();

        // internal
        virtual SessionItemObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_session_item"
        );
        virtual void    registrySignals();

        // interface
        virtual int connect();

        // generic
        ItemObject      *   getItem();
        bool                isConnected();

        // outcoming
        static void outcomingPartialReadCb(
            NetClientObject         *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );
        static void outcomingConnectedCb(
            NetClientObject         *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );
        static void outcomingConnectClosedCb(
            NetClientObject         *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );

    private:
        string  m_client_buffer;
};

#endif

