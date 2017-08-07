/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Mar 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

// stl
#include <string>
#include <vector>

using namespace std;

class ClientHandlerRawObject;

#ifndef CLIENT_HANDLER_RAW_OBJECT_H
#define CLIENT_HANDLER_RAW_OBJECT_H

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

#include <ipnoise-common/log.h>
#include "objects/client/clientHandler.h"
class DomDocument;

class ClientHandlerRawObject
    :   public ClientHandler
{
    public:
        ClientHandlerRawObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_client_handler_raw"
        );
        ~ClientHandlerRawObject();

        // internal
        virtual ClientHandlerRawObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_client_handler_raw"
        );

        // interface
        virtual     int     do_init();
        virtual     void    closed_cb();
        virtual     int     read_cb(
            const string    &a_data,
            ReadState       rd_state
        );

    protected:
        string  m_buffer;
};

#endif

