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

class ClientHandler;

#ifndef CLIENT_HANDLER_ABSTRACT_H
#define CLIENT_HANDLER_ABSTRACT_H

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
#include <event2/event.h>
#include <event2/event_struct.h>

#include <ipnoise-common/log.h>
#include "objects/ioObject.h"
class DomDocument;

#include "objects/clientObject.h"

class ClientHandler
    :   public IoObject
{
    public:
        ClientHandler(
            xmlNodePtr      node,
            DomDocument     *_doc,
            const string    &_tagname
        );
        ~ClientHandler();

        virtual int do_init();

        ClientObject * getClient();

        enum ReadState {
            READ_STATE_FIRST_READ = 0,
            READ_STATE_READ
        };

        virtual int read_cb(
            const string    &,          // a_data
            ReadState       rd_state
        );

        virtual void closed_cb();

        int     write(const string &buffer);
        int     send_answer(const string &buffer);
        bool    isApiWait();
        bool    getApiWait();
        void    setApiWait(bool a_state);

};

#endif

