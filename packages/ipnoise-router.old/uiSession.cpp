#include <sstream>
#include "base64.h"
#include "uiSession.h"

#define PING_TIME_SEC 5

uint16_t __ui_sess_counter;

using namespace std;

uint64_t uiSessionClass::SessionSequence()
{
    time_t curtime = time(NULL);
    pid_t  curpid  = getpid();

    __ui_sess_counter++;

    return (uint64_t(curtime)<<32) | __ui_sess_counter<<16 | (uint16_t)curpid;
};

void uiSessionClass::setReq(evhttp_request *_req)
{
    req = _req;
};

void uiSessionClass::watchdog()
{
    struct evbuffer *evb = NULL;
    time_t cur_time      = 0;

    cur_time = time(NULL);

    if ((cur_time - last_active_time) >= PING_TIME_SEC){
        evb = evbuffer_new();
        evbuffer_add_printf(evb, "\n");
        sendEvbuff(evb);
        evbuffer_free(evb);
    }
}

int uiSessionClass::sendEvbuff(struct evbuffer *out)
{
    if (req != NULL && out != NULL){
        evhttp_send_reply_chunk(req, out);
        last_active_time = time(NULL);
    }
    return 0;
};

uiSessionClass::uiSessionClass()
{
    ostringstream osess_id;
    osess_id << SessionSequence();
    sess_id = osess_id.str();

    req                 = NULL;
    user                = NULL;
    last_active_time    = 0;

};

uiSessionClass::~uiSessionClass()
{
};

