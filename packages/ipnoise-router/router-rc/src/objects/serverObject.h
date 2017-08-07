/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <assert.h>

// stl
#include <string>
#include <map>

class ServerObject;

using namespace std;

#ifndef SERVER_OBJECT_H
#define SERVER_OBJECT_H

class DomDocument;

#include <stdint.h>
#include <time.h>

#include "net/command.h"
#include "objects/object.h"

#include "objects/usersObject.h"
#include "objects/linksObject.h"
#include "objects/linkObject.h"
#include "objects/skBuffObject.h"
#include "objects/neighObject.h"
#include "objects/sessionsObject.h"
#include "objects/neighsObject.h"
#include "huid.hpp"

// libevent
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/dns.h>
#include <event2/event.h>
#include <event2/event_struct.h>

// fuse
#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 26
#include "fuse_lowlevel.h"
#endif

#define MAX_BAD_IFS             15
#define FUSE_DIR                "/etc/ipnoise/fuse"

#define MULTICAST_USER_REAL_NICKNAME "multicast"
#define MULTICAST_USER_HUID "2210:0000:0000:0000:0000:0000:0000:0001"

struct dirbuf
{
    char    *p;
    size_t  size;
};

#include <ipnoise-common/utils.hpp>
#include <ipnoise-common/log.h>

class ServerObject
    :   public Object
{
    public:
        ServerObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_server"
        );
        virtual ~ServerObject();

        // internal
        virtual ServerObject *  create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_server"
        );
        virtual int32_t         autorun();
        virtual void            shutdown();
        virtual Command *       getCmd();
        virtual void            timer();

        // generic
        uint32_t            getUpTime();
        string              getUpTimeStr();
        time_t              getCurTime();
        string              getCurTimeStr();

        void                getAllItems(
            map<string, ItemObject *> &
        );

        UserObject              *   getUser(const string &huid);
        SessionObject           *   getSession(const string &);
        SessionsObject          *   getCreateSessionsObject();
        DomElement              *   getCreateClientsObject();
        NeighObject             *   addNeigh(
            const string  &a_dst_huid,
            const string  &a_dev,
            const string  &a_lladdr,
            const string  &a_metric = "0"
        );

        NeighsObject    *   getNeighsObject();
        NeighsObject    *   getCreateNeighsObject();
        void                getNeighs(
            vector<NeighObject *> &a_out,
            const int             &a_only_up = 0
        );

        void getNeighsByDstHuid(
            const string            &a_dst_huid,
            vector<NeighObject *>   &a_out,
            const int               &a_only_up = 0
        );

        void addLinks(
            const string    &a_huid,
            DomElement      *a_out
        );
        UsersObject * getCreateUsersObject();

        HUID generateLocalHuid();

        int32_t init();

        struct evconnlistener   *listener;
        struct evconnlistener   *listener_hostos;

    private:
        int64_t m_uptime;

        // fuse
        struct fuse_args        *_fuse_args;
        struct event            *_fuse_ev;

        Command *command;

        int32_t                 initSignalPortTCP();
        int32_t                 initSignalPortUDP();
        int32_t                 initHostOsSignalPort();

        static void read_udp_cb(
            int socket,
            short event,
            void *arg
        );

        static void acceptSignalSocket(
            struct evconnlistener   *listener,
            evutil_socket_t         socket,
            struct sockaddr         *sa,
            int                     socklen,
            void                    *user_data
        );

    public:
        // fuse TODO create accessors and remove from here
        struct fuse_session     *_fuse_se;
        struct fuse_chan        *_fuse_ch;
        char                    *_fuse_mountpoint;
        int stopFuse();

        LinksObject *   getCreateLinksObject();
        void            CheckInterfaces();
        void            sendPacket(SkBuffObject *);
        void            netif_rx(SkBuffObject *);
        void            process_packet(SkBuffObject *);

        // send UDP messages
        static ssize_t sendMsg(
            DomElement      *a_msg,
            const string    &a_dst_huid,
            const string    &a_src_huid             = "",
            const int32_t   &a_delete_after_send    = 1
        );

        static ssize_t sendMsg(
            const string    &a_msg,
            const string    &a_dst_huid,
            const string    &a_src_huid = ""
        );

        // fuse
        static void fuseSessionLoop(
            int     socket,
            short   event,
            void    *arg
        );
        int         initFuse();
        static int  fuse_stat(
            DomElement  *ipnoise,
            fuse_ino_t  ino,
            struct stat *stbuf
        );
        static void fuse_ll_getattr(
            fuse_req_t              req,
            fuse_ino_t              ino,
            struct fuse_file_info   *fi
        );
        static void fuse_ll_lookup(
            fuse_req_t      req,
            fuse_ino_t      parent,
            const char      *name
        );
        static void dirbuf_add(
            fuse_req_t      req,
            struct dirbuf   *b,
            const char      *name,
            fuse_ino_t      ino
        );
        static void fuse_ll_readdir(
            fuse_req_t              req,
            fuse_ino_t              ino,
            size_t                  size,
            off_t                   off,
            struct fuse_file_info   *fi
        );
        static void fuse_ll_open(
            fuse_req_t              req,
            fuse_ino_t              ino,
            struct fuse_file_info   *fi
        );
        static void fuse_ll_read(
            fuse_req_t              req,
            fuse_ino_t              ino,
            size_t                  size,
            off_t                   off,
            struct fuse_file_info   *fi
        );
        static void fuse_ll_release(
            fuse_req_t              req,
            fuse_ino_t              ino,
            struct fuse_file_info   *fi
        );

        void SetupNewItem(ItemObject *item);
        void SetupNewUser(UserObject *item);

        // signal|splots implementation
        virtual void registrySignals();
        virtual void registrySlots();
        virtual void slot(
            const string    &signal_name,
            DomElement      *object
        );

    private:
        string  m_udp_buffer;
};

#endif

