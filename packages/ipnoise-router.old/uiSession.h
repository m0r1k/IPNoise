#ifndef UI_SESSIONCLASS_H
#define UI_SESSIONCLASS_H

#include <map>
#include <string>

#include <event.h>
#include <evhttp.h>
#include "shttp.h"

using namespace std;

class uiSessionClass
{
    public:
        uiSessionClass();
        ~uiSessionClass();

        uint64_t SessionSequence();

        int             error;
        string          sess_id;

        // session owner (UserClass *)
        void *user;
        void setReq(evhttp_request *_req);
        int  sendEvbuff(struct evbuffer *out);
        void watchdog();

    private:
        time_t          last_active_time;
        evhttp_request  *req;
};

#endif

