#ifndef SERVER_HPP
#define SERVER_HPP

#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/socket.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/event_struct.h>

// fuse
#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 26
#include "fuse_lowlevel.h"
#endif

// stl
#include <string>

// Local
#include "ipnoise-common/log.h"
#include "ipnoise-common/system.h"
#include "ipnoise-common/hexdump.h"
#include "ipnoise-common/strings.h"

#define FUSE_DIR "/mnt/hostfs/"

using namespace std;

struct dirbuf
{
    char    *p;
    size_t  size;
};

class Server
{
    public:
        Server();
        ~Server();

        int32_t do_init(
            struct event_base *a_base
        );

        int32_t initFuse();
        int32_t stopFuse();

    protected:
        static void shutdown_cb(evutil_socket_t, short, void *arg);

    private:
        struct event_base       *m_base;
        struct event            *m_timer;
        struct bufferevent      *m_bev;
        static void timer_cb(
            evutil_socket_t     fd,
            short               event,
            void                *arg
        );

        // fuse
        struct fuse_args        *m_fuse_args;
        struct event            *m_fuse_ev;
        struct fuse_session     *m_fuse_se;
        struct fuse_chan        *m_fuse_ch;
        char                    *m_fuse_mountpoint;

        // fuse io handlers
        static void fuseSessionLoop(
            int32_t                 a_socket,
            short                   a_event,
            void                    *a_arg
        );
        static int32_t  fuse_stat(
//            DomElement              *a_ipnoise,
            fuse_ino_t              a_ino,
            struct stat             *a_stbuf
        );
        static void fuse_ll_getattr(
            fuse_req_t              a_req,
            fuse_ino_t              a_ino,
            struct fuse_file_info   *a_fi
        );
        static void fuse_ll_lookup(
            fuse_req_t              a_req,
            fuse_ino_t              a_parent,
            const char              *a_name
        );
        static void dirbuf_add(
            fuse_req_t              a_req,
            struct dirbuf           *a_b,
            const char              *a_name,
            fuse_ino_t              a_ino
        );
        static void fuse_ll_readdir(
            fuse_req_t              a_req,
            fuse_ino_t              a_ino,
            size_t                  a_size,
            off_t                   a_off,
            struct fuse_file_info   *a_fi
        );
        static void fuse_ll_open(
            fuse_req_t              a_req,
            fuse_ino_t              a_ino,
            struct fuse_file_info   *a_fi
        );
        static void fuse_ll_read(
            fuse_req_t              a_req,
            fuse_ino_t              a_ino,
            size_t                  a_size,
            off_t                   a_off,
            struct fuse_file_info   *a_fi
        );
        static void fuse_ll_release(
            fuse_req_t              a_req,
            fuse_ino_t              a_ino,
            struct fuse_file_info   *a_fi
        );
};

#endif

