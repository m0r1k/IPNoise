#ifndef CHANNEL_H
#define CHANNEL_H

#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include "ipnoise-common/log.h"
#include "ipnoise-common/list.h"

class TcpV4Transport;

using namespace std;

typedef struct
{
    char                *buffer;
    int                 len;
    struct list_head    list;
} QueuePacket;

class Channel
{
    public:
        Channel(struct event_base *base, TcpV4Transport *transport);
        ~Channel();

        int do_init(const char *dst_addr);
        int do_init(struct bufferevent *bev, char *dst_addr);

        int start_xmit(const char *buffer, int len);

        static void readcb(struct bufferevent *bev, void *ctx);
        static void eventcb(struct bufferevent *bev, short what, void *ctx);

        int isConnected();
        int isConnecting();
        int isUnConnected();
        int isError();
        int isClosed();
        string getState();

        TcpV4Transport *getTransport();

    protected:
        void            setState(const string &new_state);
        QueuePacket *   allocate_packet();
        void            free_packet(QueuePacket *);
        void            flushTxQueue();
        QueuePacket *   queue_tx_packet(
            const char *buffer,
            int                 len
        );
        int _start_xmit(
            const char *buffer,
            int len
        );
    private:
        struct event_base   *_base;
        struct bufferevent  *_bev;
        sockaddr_in         connect_to_addr;
        TcpV4Transport      *_transport;

        // channel state
        string              _state;

        // destination addr (remote part address)
        string              _dst_addr;
        QueuePacket         *tx_queue;
};
#endif

