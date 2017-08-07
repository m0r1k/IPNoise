/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

// stl
#include <string>
#include <vector>

using namespace std;

class ClientHandlerUnknownObject;

#ifndef CLIENT_HANDLER_UNKNOWN_OBJECT_H
#define CLIENT_HANDLER_UNKNOWN_OBJECT_H

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

#include <ipnoise-common/log.h>

#include "objects/object.h"
class DomDocument;
#include "objects/client/clientHandler.h"

class ClientHandlerUnknownObject
    :   public ClientHandler
{
    public:
        ClientHandlerUnknownObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_client_handler_unknown"
        );
        ~ClientHandlerUnknownObject();

        // internal
        virtual ClientHandlerUnknownObject* create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_client_handler_unknown"
        );

        // interface
        virtual int do_init();
        virtual int read_cb(
            char        *buffer1,
            char        *buffer2,
            int         len,
            ReadState   rd_state
        );

    protected:
        string  m_buffer;
};

#endif

