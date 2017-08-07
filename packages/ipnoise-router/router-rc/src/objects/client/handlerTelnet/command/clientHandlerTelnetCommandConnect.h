/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev May 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

using namespace std;

class ClientHandlerTelnetCommandConnect;

#ifndef CLIENT_HANDLER_TELNET_COMMAND_CONNECT_H
#define CLIENT_HANDLER_TELNET_COMMAND_CONNECT_H

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"
#include "objects/netClientHttpObject.h"

#define DEFAULT_HTTP_API_HOST_PORT  "127.0.0.1:2210"

class ClientHandlerTelnetCommandConnect
    :   public  ClientHandlerTelnetCommand
{
    public:
        ClientHandlerTelnetCommandConnect(ClientHandlerTelnetObject *handler);
        ~ClientHandlerTelnetCommandConnect();

        const char *getName();
        int run(int argc, char* const* argv);

    protected:

        static void do_close_conn(ClientHandlerTelnetObject *_handler);
        void do_show_status();
        int open_hostos_socket();

        // called when new data for read available
        static void http_partial_read_cb(
            NetClientHttpObject     *http_client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );

        // called when connection established
        static void http_connected_cb(
            NetClientHttpObject     *http_client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );

        // called when connection closed
        static void http_connect_closed_cb(
            NetClientHttpObject     *http_client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );
};

#endif

