/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jul 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

// stl
#include <string>
#include <vector>

using namespace std;

class ClientObject;

#ifndef CLIENT_OBJECT_H
#define CLIENT_OBJECT_H

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/netClientObject.h"
class DomDocument;

#include "objects/client/clientHandler.h"

class ClientObject
    :   public NetClientObject
{
    public:
        ClientObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &_tagname = "ipn_client"
        );
        ~ClientObject();

        // internal
        virtual ClientObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_client"
        );

        // generic
        ssize_t             send(const string &buffer);
        ssize_t             write(const string &buffer);
        string              getRxDevName();
        void                setRxDevName(const string &api);
        void                setSessId(const string &);
        string              getSessId();
        void                setType(const string &);
        string              getType();
        ClientHandler   *   getHandler();
        void                xmit(const string &a_data);

        static void partialReadCb(
            NetClientObject     *client,
            size_t              current_offset,
            size_t              current_length,
            void                *ctx
        );

        static void connectClosedCb(
            NetClientObject     *client,
            size_t              current_offset,
            size_t              current_length,
            void                *ctx
        );
};

#endif

