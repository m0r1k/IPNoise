/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev May 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <assert.h>

// stl
#include <string>
#include <vector>

using namespace std;

class NetClientObject;

typedef string                  Content;
typedef Content::iterator       ContentIt;

#ifndef NET_CLIENT_OBJECT_H
#define NET_CLIENT_OBJECT_H

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

// libevent
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/dns.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include <ipnoise-common/log.h>
#include "objects/ioObject.h"

class DomDocument;

class NetClientObject
    :   public IoObject
{
    public:
        NetClientObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_net_client"
        );
        ~NetClientObject();

        // internal
        virtual NetClientObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_net_client"
        );
        virtual void shutdown();
        virtual void registrySignals();

        // generic
        bool    isConnected();
        void    close();

        int accept(
            evutil_socket_t     socket,
            struct sockaddr     *,
            int
        );

        void    setPartialReadCb(
            void (*_partial_read_cb)(
                NetClientObject         *client,
                size_t                  current_offset,
                size_t                  current_length,
                void                    *ctx
            ),
            void *_partial_read_cb_ctx
        );

        void    setWriteCb(
            void (*_write_cb)(
                NetClientObject         *client,
                void                    *ctx
            ),
            void *_write_cb_ctx
        );

        void    setConnectedCb(
            void (*_connected_cb)(
                NetClientObject         *client,
                size_t                  current_offset,
                size_t                  current_length,
                void                    *ctx
            ),
            void *_connected_cb_ctx
        );

        void    setConnectClosedCb(
            void (*_connect_closed_cb)(
                NetClientObject         *client,
                size_t                  current_offset,
                size_t                  current_length,
                void                    *ctx
            ),
            void *_connect_closed_cb_ctx
        );

        int open_socket(
            int     domain,
            int     type,
            int     protocol
        );

#ifdef HOSTOS
        void    useHostOS(const bool &val = true);
#endif
        int     connect(string host, int port);
        int     haveSoError();

        ssize_t     write(const string &buffer);
        ssize_t     send_answer(const string &buffer);

        Content content;

        void    setState(const string &);
        string  getState();
        string  getDstAddr();
        string  getDstPort();

    protected:
        struct bufferevent  *_bev;
        bool                use_hostos;

        static void _readCb(struct bufferevent  *bev, void *ctx);
        static void _writeCb(struct bufferevent *bev, void *ctx);
        static void _eventCb(struct bufferevent *bev, short what, void *ctx);

        // partial read callback
        void (*partial_read_cb)(
            NetClientObject         *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );
        void *partial_read_cb_ctx;

        // write callback
        void (*write_cb)(
            NetClientObject         *client,
            void                    *ctx
        );
        void *write_cb_ctx;

        // connection established callback
        void (*connected_cb)(
            NetClientObject         *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );
        void *connected_cb_ctx;

        // connection closed callback
        void (*connect_closed_cb)(
            NetClientObject         *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );
        void *connect_closed_cb_ctx;
};

#endif

