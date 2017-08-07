#ifndef SHTTP_H
#define SHTTP_H

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

#include <sys/queue.h>
#include <event.h>
#include <evhttp.h>

#include <sys/stat.h>

#define VERSION             "0.08"
#define LOCALE_CHARSET      "UTF-8"
#define WATCHDOG_TIME_SEC   1
#define NEW_DIRS_MODE       0755

#include "log.h"

using namespace std;

struct config
{
    string           root_dir;       //!< Root directory
    char            *bind_addr;      //!< Bind addr (host or IP addr as string)
    unsigned short   bind_port;      //!< Bind port
};

struct myEnviroment
{
    char            *ip;
    char            *uri;
    struct          config conf;
    int             server_start_time;
};

int     shttpd_init();
void    shttpd_destroy(int);

int s_file(struct evbuffer *, struct evhttp_request *,
    struct parts *, void *, struct myEnviroment &);
int s_API(struct evbuffer *, struct evhttp_request *,
    struct parts *, void *, struct myEnviroment &);
int s_Events(struct evbuffer *out, struct evhttp_request *req,
    struct parts *pt, void *arg, struct myEnviroment &env);
void fhttpd_gencb(struct evhttp_request * evreq, void * arg);
void icmp_cb(int fd, short event, void *arg);
void watchdog_cb(int fd, short event, void *arg);
int  add2out(struct evbuffer *out, string &data);
void save_all();

inline void my_system(const char *_cmd)
{
    string cmd = _cmd;
    cmd += " >/dev/null 2>&1";
    PDEBUG(5, "MORIK: EXEC: '%s'\n", cmd.c_str());
    system(cmd.c_str());
}

#endif

