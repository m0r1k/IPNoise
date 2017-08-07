#ifndef AF_HOSTOS_H
#define AF_HOSTOS_H

#include <net/inet_sock.h>
#include <linux/ipnoise.h>

enum {
    HOSTOS_STATE_UNCONNECTED = 0,
    HOSTOS_STATE_LISTEN,
    HOSTOS_STATE_CONNECTING,
    HOSTOS_STATE_CONNECTED,
    HOSTOS_STATE_SHUTDOWN
};

struct peer
{
    struct sockaddr saddr;
    int             saddrlen;
    struct sockaddr daddr;
    int             daddrlen;
    int             fd;
};

struct hostos_sock
{
	// inet_sock MUST be the first member
	struct inet_sock inet;
    struct peer     peer;
    int             state;
    int             sock_err;
    unsigned int    poll_mask;
    int             fd;
};

static inline struct hostos_sock *hostos_sk(const struct sock *sk)
{
	return (struct hostos_sock *)sk;
}

static int hostos_create(struct net *net, struct socket *sock, int protocol);
static int _hostos_create(struct net *net, struct socket *sock, int protocol,
    int fd);
int hostos_wait_for_events(struct sock *sk, int *err, long *timeo_p);

#endif

