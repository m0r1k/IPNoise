#include <linux/err.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <net/protocol.h>
#include <net/sock.h>
#include <net/af_hostos.h>
#include <net/inet_common.h>

#include "irq_kern.h"
#include <net/af_hostos.h>

#define RECV_BUFF_SIZE 4096

#define MODULE_NAME "pf_hostos"

#define MDEBUG(fmt, args...) \
        printk(KERN_DEBUG MODULE_NAME, fmt, ## args);
//        PDEBUG(MODULE_NAME, fmt, ## args);

#define MERROR(fmt, args...) \
        printk(KERN_ERR MODULE_NAME, fmt, ## args);
//        PERROR(MODULE_NAME, fmt, ## args);

#define MWARN(fmt, args...) \
        printk(KERN_WARNING MODULE_NAME, fmt, ## args);
//        PWARN(MODULE_NAME, fmt, ## args);

#define MINFO(fmt, args...) \
        printk(KERN_INFO MODULE_NAME, fmt, ## args);
//        PINFO(MODULE_NAME, fmt, ## args);

void __reactivate_fd(int fd)
{
    reactivate_fd(fd, IPNOISE_IO_IRQ);
}

/*
 *  Change socket state without lock.
 *  Socket must be locked already
 */
void _set_state_change(struct socket *sock, int new_state)
{

	struct hostos_sock  *hostos = NULL;
	struct sock         *sk     = sock->sk;

	hostos          = hostos_sk(sk);
    hostos->state   = new_state;

    switch (hostos->state){
        case HOSTOS_STATE_SHUTDOWN:
            sock->state = SS_DISCONNECTING;
            break;

        case HOSTOS_STATE_UNCONNECTED:
            sock->state = SS_UNCONNECTED;
            break;

        case HOSTOS_STATE_CONNECTED:
            sock->state = SS_CONNECTED;
            break;

        case HOSTOS_STATE_CONNECTING:
            sock->state = SS_CONNECTING;
            break;

        case HOSTOS_STATE_LISTEN:
            sock->state = SS_CONNECTED;
            break;


        default:
            MERROR("Unknown hostos socket state: '%d'\n", new_state);
            break;
    };

    sk->sk_state_change(sk);
}

int is_connected(struct socket *sock)
{
	struct hostos_sock  *hostos = NULL;
	struct sock         *sk     = sock->sk;
    int is_connected            = 0;

    if (!sk){
        goto out;
    }

	hostos = hostos_sk(sk);
    if (!hostos){
        goto out;
    }

    if (    sock->type      == SOCK_STREAM
        &&  sock->state     == SS_CONNECTED
        &&  hostos->state   == HOSTOS_STATE_CONNECTED)
    {
        // for stream connections
        is_connected = 1;
    }
    if (    sock->type      == SOCK_DGRAM
        &&  sock->state     == SS_CONNECTED
        &&  (   hostos->state   == HOSTOS_STATE_LISTEN
            ||  hostos->state   == HOSTOS_STATE_CONNECTED))
    {
        // for dgram connections
        is_connected = 1;
    }

out:
    return is_connected;
}

int is_shutdown(struct socket *sock)
{
	struct hostos_sock  *hostos = NULL;
	struct sock         *sk     = sock->sk;
    int is_shutdown             = 0;

    if (!sk){
        goto out;
    }

	hostos = hostos_sk(sk);

    if (hostos
        && hostos->state == HOSTOS_STATE_SHUTDOWN)
    {
        is_shutdown = 1;
    }

out:
    return is_shutdown;
}


int is_listen(struct socket *sock)
{
	struct hostos_sock  *hostos = NULL;
	struct sock         *sk     = sock->sk;
    int is_listen               = 0;

    if (!sk){
        goto out;
    }

	hostos = hostos_sk(sk);

    if (hostos
        && hostos->state == HOSTOS_STATE_LISTEN)
    {
        is_listen = 1;
    }

out:
    return is_listen;
}


int hostos_connect(
    struct socket       *sock,
    struct sockaddr     *uaddr,
	int                 addr_len,
    int                 flags)
{
	struct hostos_sock  *hostos = NULL;
	struct sock         *sk     = sock->sk;
	int err = -EINVAL, error = 0;
	long timeo;

    if (sock->type != SOCK_STREAM){
        err = -ENOTCONN;
        goto ret_dont_release;
    }

	hostos = hostos_sk(sk);

    if (is_shutdown(sock)){
        // socket have closing state..
        err = -ESHUTDOWN;
        goto ret_dont_release;
    }

	lock_sock(sk);

    // reactivate fd
    __reactivate_fd(hostos->peer.fd);

    timeo = sock_sndtimeo(sk, flags & O_NONBLOCK);

    do {
        if (hostos->state == HOSTOS_STATE_CONNECTING){
            err = -EALREADY;
            break;
        } else if (hostos->state == HOSTOS_STATE_LISTEN){
            err = -EINVAL;
            break;
        } else if (hostos->state == HOSTOS_STATE_CONNECTED){
            err = -EISCONN;
            break;
        }

        _set_state_change(sock, HOSTOS_STATE_CONNECTING);

        hostos->peer.daddrlen = min(addr_len, sizeof(hostos->peer.daddr));
        memcpy(
            &hostos->peer.daddr,
            uaddr,
            hostos->peer.daddrlen
        );

        // setup address family
        hostos->peer.daddr.sa_family = AF_INET;

        // clear errors
        hostos->sock_err = 0;

        os_socket_connect(
            hostos->peer.fd,
            &hostos->peer.daddr,
            hostos->peer.daddrlen
        );
    } while (0);

    do {
        // wait events
        if (!timeo){
            break;
        }
    } while (!hostos_wait_for_events(sk, &error, &timeo));

    // get statues
    err = hostos->sock_err;

    if (hostos->state == HOSTOS_STATE_CONNECTED){
        err = 0;
    }

ret:
	release_sock(sk);
ret_dont_release:
	return err;
}

int hostos_sendmsg(
    struct kiocb    *iocb,
    struct socket   *sock,
    struct msghdr   *msg,
    size_t          total_len)
{
    struct sock         *sk     = NULL;
	struct hostos_sock  *hostos = NULL;
	struct iovec        *iov    = NULL;
    char                *buffer = NULL;

	int i, iovlen, flags;
	int err, notcopied, copied = -1;
    int seglen = 0;
    unsigned char __user *from = NULL;

    sk = sock->sk;
    if (!sk){
        goto ret_dont_release;
    }

	hostos = hostos_sk(sk);

    if (is_shutdown(sock)){
        // socket have closing state..
        copied = 0;
        MERROR("Attempt to sendmsg in socket"
            " what have 'shutdown' state\n");
        goto ret_dont_release;
    }

	lock_sock(sk);

    // reactivate fd
    __reactivate_fd(hostos->peer.fd);

    if (!is_connected(sock)){
        err = -ENOTCONN;
        MERROR("Attempt to sendmsg in socket"
            " what have 'not-connected' state\n");
        goto fail;
    }

    flags = msg->msg_flags;

	/* Ok commence sending. */
	iovlen = msg->msg_iovlen;

	err = -EPIPE;
	if (sk->sk_err || (sk->sk_shutdown & SEND_SHUTDOWN))
		goto fail;

	for (i = 0; i < iovlen; i++){
	    iov     = &msg->msg_iov[i];
		seglen  = iov->iov_len;
		from    = iov->iov_base;

        buffer = kzalloc(seglen, GFP_KERNEL);
        if (buffer == NULL){
            goto fail;
        }

        notcopied = copy_from_user(buffer, from, seglen);
		while (seglen > 0) {
			int copy;

            if (msg->msg_name && msg->msg_namelen > 0){
                // we have destination address, so trying
                // use sendto instead write
                copy = os_socket_sendto(hostos->peer.fd,
                    buffer + (iov->iov_len - seglen),
                    seglen,
                    0,          // flags
                    msg->msg_name,
                    msg->msg_namelen
                );
            } else {
                // use generic write
                copy = os_write_socket(hostos->peer.fd,
                    buffer + (iov->iov_len - seglen),
                    seglen
                );
            }

            if (copy > 0){
                copied += copy;
                seglen -= copy;
            } else if (!copy){
                // socket closed
                break;
            } else {
                // error..
                break;
            }
		}
        kfree(buffer);
        buffer = NULL;
	}

ret:
	release_sock(sk);
ret_dont_release:
	return copied;

fail:
    goto ret;
}

int hostos_listen(struct socket *sock, int backlog)
{
	struct sock         *sk     = NULL;
	struct hostos_sock  *hostos = NULL;
    int err;

    sk     = sock->sk;
	hostos = hostos_sk(sk);

    if (sock->type != SOCK_STREAM){
        err = -ENOTCONN;
        goto ret;
    }

    err = os_socket_listen(hostos->peer.fd, backlog);
    if (!err){
	    lock_sock(sk);
        _set_state_change(sock, HOSTOS_STATE_LISTEN);
	    release_sock(sk);
    }

ret:
    return err;
}

int	hostos_ioctl(
    struct socket   *sock,
    unsigned int    cmd,
    unsigned long   arg)
{
    return 0;
}

int	hostos_shutdown(struct socket *sock, int flags)
{
    return -EINVAL;
}

int	hostos_accept(
    struct socket *sock,
    struct socket *newsock,
    int flags)
{
	struct sock         *sk         = NULL;
	struct hostos_sock  *hostos     = NULL;
	struct sock         *new_sk     = NULL;
	struct hostos_sock  *new_hostos = NULL;

    struct sockaddr     address;
    int                 address_len =  sizeof(address);
    long timeo;
    int res, err = -EINVAL, error = 0;

	sk      = sock->sk;
	hostos  = hostos_sk(sk);
    timeo   = sock_rcvtimeo(sk, flags & O_NONBLOCK);

    if (is_shutdown(sock)){
        // socket have closing state..
        err = -ESHUTDOWN;
        goto ret_dont_release;
    }

	lock_sock(sk);

    // reactivate fd
    __reactivate_fd(hostos->peer.fd);

    if (!is_listen(sock)){
        MERROR("attempt to accept from socket"
            " with hostos->state != HOSTOS_STATE_LISTEN\n");
        goto fail;
    }

    do {
        err = os_socket_accept(
            hostos->peer.fd,
            &address,
            &address_len
        );

        if (err >= 0){
            // we have new socket accepted
            res = _hostos_create(
                sock_net(sk),           // net
                newsock,                // new socket
                sk->sk_protocol,        // some protocol
                err                     // new host os descriptor
            );
            if (res){
                os_close_socket(err);
                err = res;
                break;
            }

            new_sk                      = newsock->sk;
	        new_hostos                  = hostos_sk(new_sk);
            new_hostos->peer.daddrlen   = min(
                address_len,
                sizeof(new_hostos->peer.daddr)
            );

            memcpy(
                &new_hostos->peer.daddr,
                &address,
                new_hostos->peer.daddrlen
            );
            new_hostos->peer.daddr.sa_family = AF_INET;

            // mark as connected
            _set_state_change(newsock, HOSTOS_STATE_CONNECTED);
            break;
        }

        if (!timeo){
            // timeout
            err = -EAGAIN;
            break;
        }

    } while (!hostos_wait_for_events(sk, &error, &timeo));

    if (error){
        err = error;
    }

ret:
	release_sock(sk);

ret_dont_release:
    return err;

fail:
    goto ret;
}

int hostos_bind(
    struct socket       *sock,
    struct sockaddr     *uaddr,
    int                 addr_len)
{
	struct sockaddr_in  *addr   = NULL;
	struct sock         *sk     = NULL;
	struct hostos_sock  *hostos = NULL;
    int err;

	addr   = (struct sockaddr_in *)uaddr;
	sk     = sock->sk;
	hostos = hostos_sk(sk);

	lock_sock(sk);
    err = os_socket_bind(hostos->peer.fd, (struct sockaddr *)addr, addr_len);
	release_sock(sk);

	return err;
}

int hostos_release(struct socket *sock)
{
    int err = 0;
	struct sock         *sk     = NULL;
	struct hostos_sock  *hostos = NULL;

    sk = sock->sk;
    if (sk == NULL){
        goto fail;
    }

    lock_sock(sk);

    hostos = hostos_sk(sk);

    deactivate_fd(hostos->peer.fd, IPNOISE_IO_IRQ);
    free_irq(IPNOISE_IO_IRQ, sock);

    os_close_socket(hostos->peer.fd);
    hostos->peer.fd = -1;

    _set_state_change(sock, HOSTOS_STATE_SHUTDOWN);

ret:
    if (sk != NULL){
        release_sock(sk);
    }
    return err;
fail:
    goto ret;
}

/*
 *	Is a socket 'connection oriented' ?
 */
static inline int connection_based(struct sock *sk)
{
	return sk->sk_type == SOCK_SEQPACKET
        || sk->sk_type == SOCK_STREAM;
}


/*
 * Wait for a packet..
 */
int hostos_wait_for_events(
    struct sock     *sk,
    int             *err,
    long            *timeo_p)
{
	int error;
	DEFINE_WAIT(wait);

	prepare_to_wait(sk->sk_sleep, &wait, TASK_INTERRUPTIBLE);

	/* handle signals */
	if (signal_pending(current))
		goto interrupted;

	error = 0;

	release_sock(sk);
	*timeo_p = schedule_timeout(*timeo_p);
	lock_sock(sk);

out:
	finish_wait(sk->sk_sleep, &wait);
	return error;

interrupted:
	error = sock_intr_errno(*timeo_p);
	goto out;
out_noerr:
	*err = 0;
	error = 1;
	goto out;
}

int hostos_recvmsg(
    struct kiocb    *iocb,
    struct socket   *sock,
    struct msghdr   *msg,
    size_t          total_len,
    int             flags)
{
    struct sock         *sk     = NULL;
	struct hostos_sock  *hostos = NULL;
    long timeo;
    int err, error, res, rd;
    int copied  = 0;
    char *buff  = NULL;

    sk = sock->sk;
    if (!sk){
        goto ret_dont_release;
    }

    hostos = hostos_sk(sk);
    if (is_shutdown(sk->sk_socket)){
        // socket have closing state..
        err = 0;
        goto ret_dont_release;
    }

	lock_sock(sk);

    if (!is_connected(sk->sk_socket)){
	    err = -ENOTCONN;
		goto fail;
    }

    // get buffer
    buff = kzalloc(RECV_BUFF_SIZE, GFP_KERNEL);
    if (buff == NULL){
        err = -ENOMEM;
        goto fail;
    }

    // get timeout
	timeo = sock_rcvtimeo(sk, flags & MSG_DONTWAIT);

    // setup error
    err = -EAGAIN;
	do {
	    if (sock->type == SOCK_STREAM){
            rd = os_read_socket(hostos->peer.fd, buff, RECV_BUFF_SIZE);
            memcpy(
                msg->msg_name,
                &hostos->peer.daddr,
                min(msg->msg_namelen, hostos->peer.daddrlen)
            );
        } else {
            rd = os_socket_recvfrom(
                hostos->peer.fd,
                buff,
                RECV_BUFF_SIZE,
                0, // TODO flags
                msg->msg_name,
                &msg->msg_namelen
            );
        }

        if (rd > 0){
            buff[rd] = '\0';
            res = memcpy_toiovec(
                msg->msg_iov,
                buff,
                rd
            );
            if (res){
                err = res;
                break;
            }
            copied += rd;
            break;
        } else if (!rd) {
            _set_state_change(sk->sk_socket, HOSTOS_STATE_SHUTDOWN);
            err = 0;
            break;
        }

        if (!timeo){
            // timeout
            err = -EAGAIN;
            break;
        }

    } while (!hostos_wait_for_events(sk, &error, &timeo));

    if (copied > 0){
        err = copied;
    }

ret:
    release_sock(sk);

ret_dont_release:
    if (buff){
        kfree(buff);
        buff = NULL;
    }

    if (hostos){
        // reactivate fd
        __reactivate_fd(hostos->peer.fd);
    }
    return err;

fail:
    goto ret;
}

struct proto hostos_tcp_prot = {
	.name			= "HOSTOS_TCP",
	.owner			= THIS_MODULE,
	.obj_size		= sizeof(struct hostos_sock),
};

struct proto hostos_udp_prot = {
	.name			= "HOSTOS_UDP",
	.owner			= THIS_MODULE,
	.obj_size		= sizeof(struct hostos_sock),
};

int hostos_getname(
    struct socket   *sock,
    struct sockaddr *uaddr,
    int             *uaddr_len,
    int peer)
{
    int                 len     = 0;
	struct sock         *sk     = NULL;
	struct hostos_sock  *hostos = NULL;

    sk = sock->sk;
    if (!sk){
        goto ret;
    }

    hostos = hostos_sk(sk);

    if (peer){
        len = min(hostos->peer.daddrlen, *uaddr_len);
        memcpy(uaddr, &hostos->peer.daddr, len);
        *uaddr_len = len;
    } else {
        len = min(hostos->peer.saddrlen, *uaddr_len);
        memcpy(uaddr, &hostos->peer.saddr, len);
        *uaddr_len = len;
    }

ret:
	return 0;
}

unsigned int hostos_poll(
    struct file                 *file,
    struct socket               *sock,
    struct poll_table_struct    *wait)
{
	unsigned int mask           = 0;
	struct sock         *sk     = NULL;
	struct hostos_sock  *hostos = NULL;
    int err                     = 0;
    long timeo                  = 0;
    int flags                   = 0;

    sk = sock->sk;
    if (!sk){
        goto ret;
    }

    hostos = hostos_sk(sk);

    if (is_shutdown(sock)){
        // socket have closing state..
        poll_wait(file, sk->sk_sleep, wait);
        // mask |= POLLERR;
        mask = 0;
        goto ret;
    }

    __reactivate_fd(hostos->peer.fd);
	poll_wait(file, sk->sk_sleep, wait);

    if (hostos->poll_mask > 0){
        mask = hostos->poll_mask;
        goto ret;
    }

ret:
    if (sk){
        lock_sock(sk);
        hostos->poll_mask = 0;
        release_sock(sk);
    }

    return mask;
}

const struct proto_ops hostos_proto_ops = {
	.family		   = PF_HOSTOS,
	.owner		   = THIS_MODULE,
	.recvmsg	   = hostos_recvmsg,
	.bind		   = hostos_bind,
	.connect	   = hostos_connect,
	.release	   = hostos_release,
	.listen		   = hostos_listen,
	.accept		   = hostos_accept,
	.getname	   = hostos_getname,
	.socketpair	   = sock_no_socketpair,
	.sendmsg	   = hostos_sendmsg,
	.poll		   = hostos_poll,

	.shutdown	   = hostos_shutdown,
	.setsockopt	   = sock_common_setsockopt,
	.getsockopt	   = sock_common_getsockopt,
	.ioctl		   = hostos_ioctl, // need implement <morik>
	.mmap		   = sock_no_mmap,
};

static irqreturn_t hostos_sock_irq(int irq, void *data)
{
    struct socket       *sock   = (struct socket *)data;
    struct sock         *sk     = NULL;
    struct hostos_sock  *hostos = NULL;
    int sockerr = 0;
    int res, err;

    if (sock == NULL){
        goto ret;
    }

    sk = sock->sk;
    if (sk == NULL){
        goto ret;
    }

    // get hostos socket
    hostos = hostos_sk(sk);
    if (!hostos){
        goto ret;
    }

    // deactivate fd
    deactivate_fd(hostos->peer.fd, IPNOISE_IO_IRQ);

    // get OS events
    hostos->poll_mask |= um_get_irq_events(
        hostos->peer.fd,
        IPNOISE_IO_IRQ
    );

    if (hostos->state == HOSTOS_STATE_CONNECTING){
        err = os_socket_connect(
            hostos->peer.fd,
            &hostos->peer.daddr,
            hostos->peer.daddrlen
        );

        // store result
        hostos->sock_err = err;

        // TODO XXX How we should determinate connected state?
        // good we have connecting done, this will not works
        // for other host OS that Linux
        //
        switch (err){
            case (0):
            case (-EISCONN):
                _set_state_change(sock, HOSTOS_STATE_CONNECTED);
                break;

            case (-EALREADY):
            case (-EINPROGRESS):
                // waiting..
                break;

            default:
                // error happen
                _set_state_change(sock, HOSTOS_STATE_UNCONNECTED);
                break;
        };

    } else if (hostos->state == HOSTOS_STATE_CONNECTED){
        res = os_socket_getopt(
            hostos->peer.fd,
            SOL_SOCKET,
            SO_ERROR,
            &sockerr,
            sizeof(sockerr)
        );
        if (!res){
            // option was read successfull
        }

        // Wake up anyone sleeping in poll
        sk->sk_state_change(sk);

    } else if (hostos->state == HOSTOS_STATE_LISTEN){
        // Wake up anyone sleeping in poll
        sk->sk_state_change(sk);
    } else if ( hostos->state == HOSTOS_STATE_SHUTDOWN
        ||      hostos->state == HOSTOS_STATE_UNCONNECTED)
    {
        // socket closed
        sk->sk_state_change(sk);
    }

ret:
    return IRQ_HANDLED;
}

/*
 *	Create an hostos socket.
 */

static int _hostos_create(
    struct net      *net,       // net
    struct socket   *sock,      // socket
    int             protocol,   // protocol
    int             fd)         // Host OS descriptor or -1 if need new
{
	int on = 1, err = 0;
	struct sock         *sk     = NULL;
	struct hostos_sock  *hostos = NULL;
    int irq_flags               = 0;

    // check what we are have support this sock type
	err = -ESOCKTNOSUPPORT;
	if (    sock->type != SOCK_STREAM
        &&  sock->type != SOCK_DGRAM)
    {
        MERROR("Unsupported sock type: '%d' for AF_HOSTOS\n",
            sock->type
        );
        goto fail;
    }

    if (!protocol){
        // protocol == 0 work around stupid libevent code..
        // libevent-2.0.6-rc, file: listener.c, line: 630
        // function: evconnlistener_new_bind
        //
        // fd = socket(family, SOCK_STREAM, 0);
        //                                  ^
        // Where is protocol number?        |    WTF????
        // ---------------------------------/
        protocol = IPPROTO_TCP;
    }

    // check what we are have support this protocol
    if (IPPROTO_TCP == protocol){
        // good,  now try to allocate TCP socket
	    sk = sk_alloc(net, PF_HOSTOS, GFP_KERNEL, &hostos_tcp_prot);
        if (sk == NULL){
            err = -ENOBUFS;
            MERROR("Failed to allocate PF_HOSTOS TCP socket\n");
            goto fail;
        }
        sock->state = HOSTOS_STATE_UNCONNECTED;
        irq_flags |= IRQ_READ | IRQ_WRITE;
    } else if (IPPROTO_UDP == protocol){
        // good,  now try to allocate UDP socket
	    sk = sk_alloc(net, PF_HOSTOS, GFP_KERNEL, &hostos_udp_prot);
        if (sk == NULL){
            err = -ENOBUFS;
            MERROR("Failed to allocate PF_HOSTOS UDP socket\n");
            goto fail;
        }
        sock->state = HOSTOS_STATE_CONNECTED;
        irq_flags |= IRQ_READ;
    } else {
	    err = -ESOCKTNOSUPPORT;
        MERROR("Unsupported protocol: '%d' for PF_HOSTOS\n",
            protocol
        );
        goto fail;
    }

    // get hostos socket and init states
    hostos = hostos_sk(sk);

    // clear errors
    hostos->sock_err = 0;

    if (fd < 0){
        // trying to create HOSTOS socket
        hostos->peer.fd = os_open_socket(PF_INET, sock->type, protocol);
    } else {
        // use exist HOSTOS descriptor
        hostos->peer.fd = fd;
    }

    if (hostos->peer.fd < 0){
        err = -ENETUNREACH;
        goto fail;
    }

    // disable blocking
    os_set_fd_async(hostos->peer.fd);

    // enable reuse (TODO move to .setsockopt)
    os_socket_setopt(hostos->peer.fd, SOL_SOCKET, SO_REUSEADDR,
        (char *)&on, sizeof(on));

    sk->sk_protocol     = protocol;
    sock->ops           = &hostos_proto_ops;

    // good, now init socket data
    sock_init_data(sock, sk);

    // request irq for this descriptor
    err = um_request_irq(
        IPNOISE_IO_IRQ,
        hostos->peer.fd,
        irq_flags,
        hostos_sock_irq,
        IRQF_DISABLED | IRQF_SHARED,
        "pf_hostos",
        (void *)sock
    );
    if (err){
        MERROR("Failed to request irq\n");
        goto fail;
    }

    // all ok
    err = 0;

    // set states unconnected
    _set_state_change(sock, sock->state);

ret:
    return err;
fail:
    if (hostos != NULL){
        if (hostos->peer.fd >= 0){
            os_close_socket(hostos->peer.fd);
            hostos->peer.fd = -1;
        }
    }

    if (sk != NULL){
        sk_common_release(sk);
        sk = NULL;
    }

    goto ret;
}

static int hostos_create(
    struct net      *net,
    struct socket   *sock,
    int protocol)
{
    return _hostos_create(net, sock, protocol, -1);
}

struct net_proto_family hostos_family_ops = {
    .family  = AF_HOSTOS,
    .create  = hostos_create,
    .owner   = THIS_MODULE
};

static int __init hostos_init(void)
{
    int err = 0;

    err = proto_register(&hostos_tcp_prot, 1);
    if (err){
        goto fail;
    }

    err = proto_register(&hostos_udp_prot, 1);
    if (err){
        goto fail;
    }

	(void)sock_register(&hostos_family_ops);

ret:
    return err;
fail:
    goto ret;
}

fs_initcall(hostos_init);

