/**
* IPNoise router, supported by libevent
*
* @autor rchechnev<at>mail<dot>ru
* @website http://www.chechnev.ru/
*/
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include <locale.h>

#include "shttp.h"
#include "parts.h"

#include <getopt.h>

#include <linux/filter.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

struct myEnviroment myenv;
int    debug_level;

// timer
struct event    ev_timer;
struct timeval  tv;

int main(int argc, char** argv)
{
    struct event_base *evbase   = NULL;
    struct evhttp *evhttp       = NULL;
    int res;

    int icmp_sock = -1;
    struct event icmp_ev;

    struct timeval ti;
    gettimeofday(&ti, NULL);
    myenv.server_start_time     = ti.tv_sec;
    myenv.conf.bind_addr        = (char *)"0.0.0.0";
    myenv.conf.bind_port        = 2210;
    myenv.conf.root_dir         = (char *)"/etc/ipnoise/";

    struct icmp6_filter filter;
    int csum_offset, sz_opt, hold, working_recverr;

    // get args
    while(1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"saddr",       1, 0, 's'},
            {"sport",       1, 0, 'p'},
            {"version",     0, 0, 'v'},
            {"debug",       1, 0, '1'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv, "hs:p:v1:",
            long_options, &option_index
        );
        if (option == -1){
            break;
        }

        switch (option) {
            case 0:
                break;

            case 'h':
                fprintf (stdout,
                    "Usage:     %s [-options]                     \n"
                    "options:                                     \n"
                    "         -h, --help        This help         \n"
                    "         -s, --saddr       Bind address      \n"
                    "         -p, --sport       Bind port         \n"
                    "         -v, --version     Program version   \n"
                    "         --debug <level>   Debug level       \n"
                    "\n",
                    argv[0]
                );
                exit(0);

            case 'r':
                if (!optarg){
                    PERROR("Missing root dir\n");
                    exit (1);
                }
                myenv.conf.root_dir = optarg;
                if (myenv.conf.root_dir[myenv.conf.root_dir.size() - 1] != '/'){
                    myenv.conf.root_dir += "/";
                };
                break;

            case 's':
                myenv.conf.bind_addr = optarg;
                break;

            case '1':
                debug_level = atoi(optarg);
                break;

            case 'p':
                myenv.conf.bind_port = atoi(optarg);
                if (myenv.conf.bind_port < 1 || myenv.conf.bind_port > 65535){
                    fprintf(stderr,
                        "Invalid port number: '%s' (Must be 1-65535)\n",
                        optarg
                    );
                    exit (1);
                }
                break;

            case 'v':
                fprintf (stdout, "Version: '%s'\n", VERSION);
                exit(0);

            case '?':
                exit (1);
        }
    }

    // check another args
    if (optind < argc){
        fprintf(stderr, "Warning: argument(s) will be ignored: ");
        while (optind < argc){
            fprintf(stderr, "'%s' ", argv[optind++]);
        }
        fprintf(stderr, "\n");
    }

    PINFO("Options:\n");
    PINFO("bind_addr:      %s\n", myenv.conf.bind_addr);
    PINFO("bind_port:      %d\n", myenv.conf.bind_port);
    PINFO("root_dir:       %s\n", myenv.conf.root_dir.c_str());
    PINFO("debug-level:    %d\n", debug_level);
    PINFO("\n");

    // Force locale to be LOCALE_CHARSET
    setlocale(LC_CTYPE, "ru_RU."LOCALE_CHARSET);
    res = shttpd_init();
    if (res){
        PERROR("shttpd_init() failed\n");
        return res;
    }

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)        return (1);
    if (signal(SIGTERM, shttpd_destroy) == SIG_ERR) return (0);
    if (signal(SIGINT,  shttpd_destroy) == SIG_ERR) return (0);

    /* 1). event initialization */
    evbase = event_init();
    if (NULL == evbase){
        PERROR("event_base_new\n");
        return 1;
    }

   /* 2). event http initialization */
    evhttp = evhttp_new(evbase);
    if (NULL == evhttp){
        PERROR("evhttp_new\n");
        return 2;
    }

    /* 3). set general callback of http request */
    evhttp_set_gencb(evhttp, &fhttpd_gencb, NULL);
    evhttp_set_timeout(evhttp, 60);

    /* 4). bind socket */
    res = evhttp_bind_socket(evhttp,
        myenv.conf.bind_addr,
        myenv.conf.bind_port
    );

    if (res){
        PERROR("evhttp_bind_socket\n");
        return 3;
    }

    // 5). init timer
    evtimer_set(&ev_timer, watchdog_cb, &ev_timer);
    tv.tv_usec = 0;
    tv.tv_sec  = WATCHDOG_TIME_SEC;
    evtimer_add(&ev_timer, &tv);

    // 6). create icmp socket
    icmp_sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
    if (icmp_sock < 0) {
        PERROR("Cannot open icmp socket");
        goto ret;
    }
    if (fcntl(icmp_sock, F_SETFL, O_NONBLOCK) == -1){
        PERROR("Cannot set icmp socket in non-blocked mode\n");
        goto ret;
    }

    working_recverr = 1;
    hold = 1;
    if (setsockopt(icmp_sock, SOL_IPV6, IPV6_RECVERR,
        (char *)&hold, sizeof(hold)))
    {
        fprintf(stderr,
            "WARNING: your kernel is veeery old. No problems.\n");
        working_recverr = 0;
    }

    csum_offset = 2;
    sz_opt = sizeof(int);

    res = setsockopt(icmp_sock, SOL_RAW,
        IPV6_CHECKSUM, &csum_offset, sz_opt);

    if (res < 0) {
        PERROR("setsockopt(RAW_CHECKSUM)");
        goto ret;
    }

    ICMP6_FILTER_SETBLOCKALL(&filter);
    if (!working_recverr) {
        ICMP6_FILTER_SETPASS(ICMP6_DST_UNREACH, &filter);
        ICMP6_FILTER_SETPASS(ICMP6_PACKET_TOO_BIG, &filter);
        ICMP6_FILTER_SETPASS(ICMP6_TIME_EXCEEDED, &filter);
        ICMP6_FILTER_SETPASS(ICMP6_PARAM_PROB, &filter);
    }

    ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY, &filter);

    res = setsockopt(icmp_sock, IPPROTO_ICMPV6, ICMP6_FILTER, &filter,
             sizeof(struct icmp6_filter));

    if (res < 0) {
        PERROR("setsockopt(ICMP6_FILTER)");
        goto ret;
    }

    // EV_READ|EV_PERSIST
    event_set(&icmp_ev, icmp_sock, EV_READ|EV_PERSIST, icmp_cb, NULL);
    event_base_set(evbase, &icmp_ev);
    event_add(&icmp_ev, NULL);

    /* 7). start http server */
    if (0 == event_base_dispatch(evbase)){
        PERROR("event_base_dispatch\n");
    }

ret:
    /* 8). free resource before exit */

    if (icmp_sock >= 0){
        close(icmp_sock);
    }
    evhttp_free(evhttp);
    event_base_free(evbase);

    shttpd_destroy(0);

    return 0;
}

//void cleanup(struct evhttp_connection *evcon, void *arg)
//{
//    PERROR("disconnected uid \n");
//}

void fhttpd_gencb(struct evhttp_request * evreq, void * arg)
{
    struct evbuffer     *evbuff = NULL;
    struct parts        parts;

    initParts(&parts, evreq->uri, '/');

    char *xRealIP = (char *)evhttp_find_header(
        evreq->input_headers,
        "X-Real-IP"
    );

    myenv.ip  = xRealIP == NULL ? evreq->remote_host : xRealIP;
    myenv.uri = evreq->uri;

    evbuff = evbuffer_new();
    if (NULL == evbuff){
        perror("evbuffer_new");
        return;
    }

    if (parts.count < 2){
        goto s_file;
    }

    if (strcmp(parts.part[1], "API") != 0){
        goto s_file;
    }

    // it's API request
    if (parts.count >= 3 && strcmp(parts.part[2], "events") == 0){
        s_Events(evbuff, evreq, &parts, arg, myenv);
    } else {
        s_API(evbuff, evreq, &parts, arg, myenv);
    }

    goto ret;

s_file:
    s_file(evbuff, evreq, &parts, arg, myenv);

ret:
    freeParts(&parts);
    /* don't forget to release evbuffer */
    evbuffer_free(evbuff);
}

