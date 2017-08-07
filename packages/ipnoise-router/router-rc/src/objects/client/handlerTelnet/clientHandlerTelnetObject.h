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

#include "objects/client/handlerTelnet/clientHandlerTelnetCommandAbstract.h"

using namespace std;

class ClientHandlerTelnetObject;

typedef vector<ClientHandlerTelnetCommand *> TELNET_COMMANDS;

#ifndef CLIENT_HANDLER_TELNET_OBJECT_H
#define CLIENT_HANDLER_TELNET_OBJECT_H

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

// libevent
#include "event2/event.h"
#include "event2/http.h"
#include "event2/http_compat.h"
#include "event2/http_struct.h"
#include "event2/buffer.h"

// local
#include <ipnoise-common/log.h>
#include "objects/client/clientHandler.h"
class DomDocument;

#include "main.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetHistory.h"
#include "objects/netClientHttpObject.h"

#define IPNOISE_API_HOST    "::1"
#define IPNOISE_API_PORT    2210

class ClientHandlerTelnetObject
    :   public ClientHandler
{
    public:
        ClientHandlerTelnetObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_client_handler_telnet"
        );
        ~ClientHandlerTelnetObject();

        // internal
        virtual ClientHandlerTelnetObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_client_handler_telnet"
        );

        // generic
        enum Mode {
            MODE_COMMAND_ERROR = -1,
            MODE_COMMAND_EDIT,
            MODE_HISTORY_WALK
        };

        int write(const string &buffer);

        virtual     int     do_init();
        virtual     void    closed_cb();
        virtual     int     read_cb(
            const string    &a_data,
            ReadState       rd_state
        );

        string parse_telnet(const string &a_data);

        // telnet options
        string  terminal_type;

        int do_terminal_type;
        int do_suppress_go_ahead;
        int do_echo;
        int do_remote_flow_control;

        int will_terminal_type;
        int will_suppress_go_ahead;
        int will_echo;
        int will_remote_flow_control;

        // IPNoise client state {
        string  apiGetSessionId();
        void    apiSetSessionId(string &sessid);
        string  apiGetHostPort();
        void    apiSetHostPort(string &host_port);
        // IPNoise client state }

        // show|hide current history line
        void    history_line_hide();
        void    history_line_show();

        static void process_answer(
            NetClientHttpObject     *http_client,
            void                    *ctx
        );

        static void do_http_command_cb(
            NetClientHttpObject     *http_client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );

        static void http_api_wait_cb(
            NetClientHttpObject     *http_client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );

        static void http_api_wait_closed_cb(
            NetClientHttpObject     *http_client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );

        int do_http_command(string &command);

        // signal|splots implementation
        virtual void registrySignals();
        virtual void registrySlots();
        virtual void slot(
            const string    &signal_name,
            DomElement      *object
        );

    protected:
        int process_mode_edit(
            const string    &a_data,
            string          &prefix
        );
        int process_command(string &command);
        int open_http_client_wait();

    private:
        TELNET_COMMANDS         commands;
        Mode                    mode;
        History                 *history;
};

#endif

