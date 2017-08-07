#ifndef _HOSTOS_TCP_H
#define _HOSTOS_TCP_H

extern struct proto hostos_tcp_prot;

extern void hostos_tcp_close(struct sock *sk, long timeout);
extern int hostos_tcp_v4_connect(
    struct sock         *sk,
    struct sockaddr     *uaddr,
	int                 addr_len
);
extern int hostos_tcp_disconnect(struct sock *sk, int flags);
extern int hostos_tcp_ioctl(
    struct sock *sk,
    int cmd,
    unsigned long arg
);

extern void hostos_tcp_v4_destroy_sock(struct sock *sk);
extern void	hostos_tcp_shutdown (struct sock *sk, int how);

extern int hostos_tcp_setsockopt(
    struct sock     *sk,
    int             level,
	int             optname,
    char __user     *optval,
    int             optlen
);

extern int hostos_tcp_getsockopt(
    struct sock     *sk,
    int             level,
    int             optname,
    char __user     *optval,
    int __user      *optlen
);

extern int hostos_tcp_recvmsg(
    struct kiocb    *iocb,
    struct sock     *sk,
	struct msghdr   *msg,
	size_t          len,
    int             nonblock,
	int             flags,
    int             *addr_len
);

extern int hostos_tcp_v4_do_rcv(
    struct sock     *sk,
	struct sk_buff  *skb
);

extern void hostos_tcp_enter_memory_pressure(struct sock *sk);
extern atomic_t hostos_tcp_sockets_allocated;
extern atomic_t hostos_tcp_orphan_count;
extern atomic_t hostos_tcp_memory_allocated;
extern int hostos_tcp_memory_pressure;
extern int hostos_sysctl_tcp_mem[3];
extern int hostos_sysctl_tcp_wmem[3];
extern int hostos_sysctl_tcp_rmem[3];
extern struct inet_hashinfo hostos_tcp_hashinfo;


#endif

