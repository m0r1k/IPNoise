/*
 *
 * Jul 25, 2010
 * Roman E. Chechnev <ipnoise|d|o|g|chechnev.ru>
 *
 */

#include <linux/module.h>
#include <net/sock.h>

#include "ipnoise_tcp.h"

#include <linux/mm.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/if_arp_ipnoise.h>
#include <linux/if_ipnoise_tcp.h>

#include <linux/ip.h>
#include <linux/ctype.h>

#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/freezer.h>
#include <linux/kthread.h>

#include "irq_kern.h"
#include "um_malloc.h"
#include "os.h"
#include "ipnoise_base64.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(MODULE_AUTH);
MODULE_DESCRIPTION(MODULE_DESCR);
MODULE_VERSION(MODULE_VER);

struct task_struct *thread  = NULL;
struct net_device  *dev     = NULL;

struct ipnoise_tcp_private
{
    struct net_device *dev;
};

#define NPACK           10
#define PORT            2210
#define BACKLOG_SIZE    20

static int listen_sd = -1;

DECLARE_WAIT_QUEUE_HEAD(wait_queue);

struct client clients[MAX_CLIENTS];

void wake_up_thread (void)
{
    wake_up(&wait_queue);
};

int do_close_conn(struct client *client, int close_os)
{
    int err = 0;
    if (client->used != USED_MAGIC){
        goto ret;
    }

    MDEBUG("[MORIK ] REQUEST TO CLOSE CONNECTION\n"
        "client descriptor: %d\n",
        client->os_sd
    );

    if (client->state >= CLIENT_STATE_CONNECTED){
        free_irq(IPNOISE_IO_IRQ, client);
    }

    if (close_os && client->os_sd >= 0){
        os_close_socket(client->os_sd);
        client->os_sd = -1;
    }

    if (close_os){
        // clear connection
        if (client->buffer_start != NULL){
            kfree(client->buffer_start);
        }
        memset(client, 0x00, sizeof(struct client));
    }

ret:
    return err;
}

static irqreturn_t os_sock_irq(int irq, void *data)
{
    struct  client *client      = (struct  client *)data;
    struct  client *free_client = NULL;

    struct  sockaddr_in os_saddr;
    int     saddr_len = sizeof(os_saddr), res, i, err;
    struct  linger l = { 1, 0 };
    int len = 0, ret = 0;

    // hm.. we have irq..
    MDEBUG("os_sock_irq\n");
    deactivate_fd(listen_sd, IPNOISE_IO_IRQ);

//    res = os_socket_getopt(client->os_sd, SOL_SOCKET, SO_ERROR, &ret, &len);
//    if (res == -1){
//        // socket error, close it
//        do_close_conn(client, 1);
//    }

    if (client && client->used != USED_MAGIC){
        // garbage :(
        client = NULL;
    }

    if (client){
        deactivate_fd(client->os_sd, IPNOISE_IO_IRQ);
        MDEBUG("os_sock_irq clent exist, sd: '%d', state: '%d'\n",
            client->os_sd,
            client->state
        );
        if (client->state == CLIENT_STATE_CONNECTING){
            res = os_socket_connect(client->os_sd, &client->os_saddr,
                client->saddr_len);
            MDEBUG("os_socket_connect sd: '%d', res: '%d'\n",
                client->os_sd,
                res
            );
            if (!res || res == -EALREADY){
                // XXX be careful, other OS functions
                // may don't known about -EALREADY :(
                // TODO we need emplement os_socket_is_connected method
                client->state = CLIENT_STATE_CONNECTED;
            }
        }


    } else {
        MDEBUG("os_sock_irq clent NOT exist\n");
    }

    // maybe it accept?
    res = os_socket_accept(listen_sd,
        (struct sockaddr *)&os_saddr,
        &saddr_len
    );

    if (res >= 0){
        // disable blocking
        os_set_fd_async(res);

        // yes, we have new connection
        MDEBUG("new connection accepted"
            " listen_sd: '%d', new_sd: '%d'\n",
            listen_sd,
            res
        );

/*
        // enable SO_LINGER
        err = os_socket_setopt(res, SOL_SOCKET, SO_LINGER,
            (char *)&l, sizeof(struct linger));
        if (err){
            MDEBUG("Failed set SO_LINGER option to socket\n");
        }
*/

        // try to search memory for client structure
        for (i = 0; i < MAX_CLIENTS; i++){
            free_client = &clients[i];
            if (free_client->used != USED_MAGIC){
                break;
            }
            free_client = NULL;
        }
        if (!free_client){
            // we have not empty memory
            // to many connections.. drop it
            MERROR("Connections count reach MAX_CLIENTS value,"
                " new connection will droped.. sorry..\n");
            os_close_socket(res);
            goto ret;
        }

        // get connection
        free_client->used = USED_MAGIC;

        // store client's descriptor
        free_client->os_sd = res;

        // copy address
        memcpy(&free_client->os_saddr, &os_saddr, saddr_len);
        free_client->saddr_len = saddr_len;

        // setup state
        free_client->state = CLIENT_STATE_ACCEPT;
    }

ret:

    // reactivate sd
    reactivate_fd(listen_sd, IPNOISE_IO_IRQ);

    wake_up_thread();
    return IRQ_HANDLED;
}

// return 0 if success
int prepare_client(struct client *client)
{
    int err = 0;

    MDEBUG("START prepare_client\n");

    // disable blocking
    os_set_fd_async(client->os_sd);

    // request irq for this descriptor
    err = um_request_irq(IPNOISE_IO_IRQ, client->os_sd, IRQ_READ,
        os_sock_irq,
        IRQF_DISABLED | IRQF_SHARED,
        "ipnoise_tcp",
        client
    );
    if (err){
        MDEBUG("Failed to request irq\n");
    } else {
        client->state = CLIENT_STATE_CONNECTED;
    }

    MDEBUG("END  prepare_client\n");
    return err;
}

int rx_packet(struct client *client, char *buffer, int size)
{
    int res, err = 0;
    int skb_len = 0, pkt_size;

    struct ipnoisehdr *l2hdr    = NULL;
    static char *target         = NULL;
    struct sk_buff *skb         = NULL;

    MDEBUG("packet received, decode %d byte(s)\n", size);

    // alloc memory
    target = kzalloc(size, UM_GFP_KERNEL);
    if (!target){
        MERROR("Cannot alloc memory\n");
        goto fail;
    }

    // ok we have data, create packet
    res = base64_decode(buffer, target, size);
    pkt_size = res;
    if (pkt_size <= 0){
        MERROR("Cannot decode packet\n");
        goto fail;
    }

    skb_len = pkt_size + NET_IP_ALIGN + dev->hard_header_len;
    skb     = netdev_alloc_skb(dev, skb_len);
    if (!likely(skb)){
        MERROR("Cannot alloc skb: '%d' byte(s) was needed\n",
            skb_len);
        goto fail;
    }

     // reset mac header
    skb_reset_mac_header(skb);

    dev->last_rx                = jiffies;
    dev->stats.rx_bytes         += skb_len;
    dev->stats.rx_packets++;

    skb->dev = dev;
    skb_set_mac_header(skb, 0);

    MDEBUG("skb_network_header: 0x%x\n",
        (unsigned int)skb_network_header(skb));
    MDEBUG("skb_mac_header:     0x%x\n",
        (unsigned int)skb_mac_header(skb));
    MDEBUG("skb->data:          0x%x\n", (unsigned int)skb->data);
    MDEBUG("skb->head:          0x%x\n", (unsigned int)skb->head);


    // Reserver space for L2 header
    l2hdr = ipnoise_hdr(skb);
    skb_reserve(skb, dev->hard_header_len);

    skb_set_network_header(skb, 0);

    // Copy the bytes
    char *ptr = skb_put(skb, pkt_size);
    memcpy(ptr, target, pkt_size);

    snprintf(l2hdr->h_source,
        IPNOISE_ALEN,
        "%d.%d.%d.%d:%d",
        (client->os_saddr.sin_addr.s_addr & 0x000000FF),
        (client->os_saddr.sin_addr.s_addr & 0x0000FF00) >> 8,
        (client->os_saddr.sin_addr.s_addr & 0x00FF0000) >> 16,
        (client->os_saddr.sin_addr.s_addr & 0xFF000000) >> 24,
        ntohs(client->os_saddr.sin_port)
    );
    skb->protocol = htons(ETH_P_IPV6);
    MDEBUG("skb_network_header: 0x%x\n",
        (unsigned int)skb_network_header(skb));
    MDEBUG("skb_mac_header:     0x%x\n",
        (unsigned int)skb_mac_header(skb));
    MDEBUG("skb->data:          0x%x\n",
        (unsigned int)skb->data);
    MDEBUG("skb->head:          0x%x\n",
        (unsigned int)skb->head);
    MDEBUG("\n");

    MDEBUG("Was received packet from: '%s' dev: '%s' via sd: '%d'\n",
        l2hdr->h_source,
        dev->name,
        client->os_sd
    );

    MDEBUG("Prepare netif_receive_skb skb->head:\n");
    ipnoise_hexdump((unsigned char *)skb->head, (skb->end - skb->head));

    // do kernl receive
    MDEBUG("Now call netif_receive_skb with next skb->data:\n");
    ipnoise_hexdump((unsigned char *)skb->data, skb->len);

    netif_receive_skb (skb);

ret:
    if (target != NULL){
        kfree(target);
        target = NULL;
    }
    return err;

fail:
    err = 1;
    goto ret;
}

// return 0 if success
int _read_from_client(struct client *client)
{
    int i, res, err = 0;

//    MDEBUG("START _read_from_client\n");

    // prepare receiving
    if (client->buffer_start == NULL){
        client->buffer_start = kzalloc(BUFFER_SIZE, UM_GFP_KERNEL);
        if (client->buffer_start == NULL){
            MERROR("Cannot allocate memory\n");
            goto fail;
        }
        client->buffer_end      = client->buffer_start + BUFFER_SIZE;
        client->buffer_ptr      = client->buffer_start;
        client->buffer_state    = BUFFER_STATE_READ;
    }

    do {
        // trying to read
        res = os_read_file(client->os_sd, client->buffer_ptr,
             client->buffer_end - client->buffer_ptr);

        if (!res){
            // looks like conection was closed from client's part
            err = 1;
            goto ret;
        }

        if (res < 0){
            // check error, if it's not EGAIN, close conn
            //if (res != -EWOULDBLOCK){
            //    MDEBUG("os_read_file failed: '%d'\n", res);
            //    err = 1;
            //}
            break;
        }

        // data was received
        MDEBUG("Was received from client: '%d' byte(s)\n", res);

        // search end of packet
        for (i = 0; i < res; i++){
            if (client->buffer_ptr[0] == '\n'){
                if (client->buffer_state == BUFFER_STATE_READ){
                    // end of packet was found
                    client->buffer_ptr[0] = '\0';
                    res = rx_packet(client,
                        client->buffer_start,
                        client->buffer_ptr - client->buffer_start
                    );
                    if (res){
                        dev->stats.rx_errors++;
                    }
                }
                memmove(client->buffer_start,
                    &client->buffer_ptr[ ((i + 1) < res) ? i + 1 : i ],
                    ((res - i - 1) < 0) ? 0 : (res - i - 1)
                );
                client->buffer_ptr = client->buffer_start;
                break;
            }
            client->buffer_ptr++;
        }

        if (client->buffer_ptr >= client->buffer_end){
            client->buffer_state = BUFFER_STATE_WAIT_NEXT;
            client->buffer_ptr = client->buffer_start;
        }

    } while (1);

ret:
//    MDEBUG("END   _read_from_client\n");
    return err;

fail:
    err = 1;
    goto ret;
}

int ipnoise_tcp_thread (void *data)
{
    int i, m, res;
    struct  client *client;
    int close_conn = 0;

    for (;;){
        DECLARE_WAITQUEUE(wait, thread);

        test_and_clear_ti_thread_flag(
            task_thread_info(thread), TIF_SIGPENDING
        );

        MDEBUG("_____ THREAD GOING SLEEP  _____\n");
        set_current_state(TASK_INTERRUPTIBLE);

        add_wait_queue(&wait_queue, &wait);
        sigdelsetmask(&thread->signal->shared_pending.signal, sigmask(SIGIO));

        // schedule_timeout(1 * HZ);
        schedule();
        MDEBUG("_____ THREAD RUNNING      _____\n");

        remove_wait_queue(&wait_queue, &wait);
        set_current_state(TASK_RUNNING);

        // do our work

        // ********** Prepare outgoing socket *********

        for (m = 0; m < 2; m++){
            for (i = 0; i < MAX_CLIENTS; i++){
                // get cur client
                client = &clients[i];
                if (client->used != USED_MAGIC){
                    // it's free connection
                    continue;
                }

                close_conn = 0;
                if (client->state == CLIENT_STATE_ACCEPT){
                    res = prepare_client(client);
                    if (res){
                        close_conn = 1;
                    }
                }
                if (client->state == CLIENT_STATE_CONNECTED){
                    // try to read data from client
                    res = _read_from_client(client);
                    if (res){
                        close_conn = 1;
                    }
                }
                if (close_conn){
                    do_close_conn(client, 1);
                } else {
                    reactivate_fd(client->os_sd, IPNOISE_IO_IRQ);
                }
            }
        }
    }

    return 0;
}

static int ipnoise_tcp_mac_addr(struct net_device *dev, void *p)
{
    struct sockaddr *addr = p;

    if (netif_running(dev))
        return -EBUSY;
    if (!is_valid_ipnoise_tcp_addr(addr->sa_data))
        return -EADDRNOTAVAIL;
    memcpy(dev->dev_addr, addr->sa_data, IPNOISE_ALEN);
    return 0;
}

static int ipnoise_tcp_change_mtu(struct net_device *dev, int new_mtu)
{
    if (new_mtu < 1 || new_mtu > IPNOISE_TCP_DATA_LEN)
        return -EINVAL;
    dev->mtu = new_mtu;
    return 0;
}

static int ipnoise_tcp_validate_addr(struct net_device *dev)
{
    if (!is_valid_ipnoise_tcp_addr(dev->dev_addr))
        return -EINVAL;

    return 0;
}

static int ipn_atoi(const char **s)
{
    int i = 0;

    while (isdigit(**s))
        i = i*10 + *((*s)++) - '0';
    return i;
}

// convert ipv4 address to unsigned int
unsigned int ip2uint(unsigned char *ip_addr)
{
    unsigned int ip = 0x00;
    int i, octen = 0;
    int buf_size = MAX_ADDR_LEN;
    unsigned char *buf;
    unsigned char *octen_ptr;
    unsigned char octen_value;

    if (!ip_addr){
        goto ret;
    }
    buf = kzalloc(buf_size + 1, 0);
    if (!buf){
        MERROR("Cannot allocate memory\n");
        goto ret;
    }

    octen_ptr = buf;
    memset(buf, 0x00, buf_size + 1);
    strncpy(buf, ip_addr, buf_size);

    for (i = 0; i < buf_size; i++){
        if (buf[i] == '.' || buf[i] == 0x00){
            // get octen
            octen++;
            buf[i] = 0x00;
            // recalculate ip address
            if (!octen_ptr){
                continue;
            }
            octen_value = (unsigned char)ipn_atoi(
                (const char **)&octen_ptr
            );
            ip += (octen_value << ((4 - octen) * 8));
            if (octen >= 4 || buf[i + 1] == 0x00){
                break;
            }
            octen_ptr = buf + i + 1;
        }
    }

    if (octen != 4){
        ip = 0x00;
    }

    kfree(buf);

ret:
    return ip;
}

static int ipnoise_tcp_start_xmit (struct sk_buff *skb,
    struct net_device *dev)
{
    struct  client  *client     = NULL;
    struct  client  *client_tmp = NULL;
    struct sockaddr_in si_other;
    int i, res, slen = sizeof(si_other);
    unsigned char   *dst_ip;
    unsigned char   *dst_ip_ptr;
    char            *dst_port;
    unsigned int    dst_in_port;
    unsigned int    dst_in_ip;

    struct neighbour *neighbour = NULL;
    struct dst_entry *dst       = NULL;

    int   targetlen             = 0;
    char *target                = NULL;

    struct ipnoisehdr *hdr      = (struct ipnoisehdr *)skb->data;
    char *packet                = skb->data;
    int  packet_len             = skb->len;

    MDEBUG("-------------- SEND PACKET TCP --------------\n");

    dst_ip = kzalloc(MAX_ADDR_LEN, 0);
    if (!dst_ip){
        MERROR("Cannot allocate memory\n");
        goto fail;
    }
    dst_ip_ptr = dst_ip;

    dst = skb->dst;
    if (dst != NULL){
        neighbour = dst->neighbour;
    }

    if (neighbour != NULL
        && neighbour->ha != NULL
        && strncmp(neighbour->ha, "", MAX_ADDR_LEN))
    {
        strncpy(dst_ip, neighbour->ha, MAX_ADDR_LEN);
    } else {
        strncpy(dst_ip, hdr->h_dest, IPNOISE_ALEN);
    }

    packet     += IPNOISE_HLEN;
    packet_len -= IPNOISE_HLEN;


    // dump packet
    ipnoise_hexdump((unsigned char *)packet, packet_len);

    MDEBUG("DST HW ADDR: %s\n", dst_ip);
    MDEBUG("SRC HW ADDR: %s\n", dev->dev_addr);
    MDEBUG("Protocol:    %d\n", skb->protocol);

    dst_port = strchr(dst_ip_ptr, ':');
    if (!dst_port){
        MERROR("Invalid address format, must be (ip:port)\n");
        goto fail;
    }

    *dst_port++ = 0x00;

    dst_in_port = ipn_atoi((const char **)&dst_port);
    dst_in_ip   = ip2uint((unsigned char*)dst_ip_ptr);

    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_port           = htons((unsigned short)dst_in_port);
    si_other.sin_addr.s_addr    = htonl(dst_in_ip);
    si_other.sin_family         = AF_INET;

    MDEBUG("SEND FOR host: %d.%d.%d.%d\n",
        (ntohl(si_other.sin_addr.s_addr) & 0xFF000000) >> 24,
        (ntohl(si_other.sin_addr.s_addr) & 0x00FF0000) >> 16,
        (ntohl(si_other.sin_addr.s_addr) & 0x0000FF00) >> 8,
        (ntohl(si_other.sin_addr.s_addr) & 0x000000FF)
    );
    MDEBUG("SEND FOR port: %d\n", ntohs(si_other.sin_port));

    targetlen = packet_len*4;
    target    = kzalloc(targetlen, UM_GFP_KERNEL);
    if (target == NULL){
        MERROR("Cannot alloc memory\n");
        goto fail;
    }

    base64_encode(packet, packet_len, target, targetlen - 1);
    // add new line symbol
    target[strlen(target)] = '\n';

    MDEBUG("Need send base64 text: '%s'\n", target);

    // search already open connection
    for (i = 0; i < MAX_CLIENTS; i++){
        client_tmp = &clients[i];
        if (client_tmp->used != USED_MAGIC){
            // empty slot
            continue;
        }
        res = memcmp(&si_other.sin_addr.s_addr,
            &client_tmp->os_saddr.sin_addr.s_addr,
            sizeof(client_tmp->os_saddr.sin_addr.s_addr)
        );
        if (!res && si_other.sin_port == client_tmp->os_saddr.sin_port){
            // connection already exist! use it :)
            MDEBUG("client already found\n");
            client = client_tmp;
            break;
        }
    }

    if (client == NULL){
        MDEBUG("client was not found, created\n");

        // trying to create new connection
        // try to search memory for client structure
        for (i = 0; i < MAX_CLIENTS; i++){
            client = &clients[i];
            if (client->used != USED_MAGIC){
                break;
            }
            client = NULL;
        }
        if (client == NULL){
            // we have not empty memory
            // to many connections.. drop it
            MERROR("Connections count reach MAX_CLIENTS value,"
                " new connection will droped.. sorry..\n");
            goto fail;
        }

        // ok we have empty client structure, fill it
        client->saddr_len = sizeof(client->os_saddr);
        memcpy(&client->os_saddr, &si_other, client->saddr_len);

        // create socket
        client->os_sd = os_open_socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        MDEBUG("created socket: '%d'\n", client->os_sd);
        if (client->os_sd < 0){
            MERROR("Failed to create socket.\n");
            goto fail;
        }

        // disable blocking
        os_set_fd_async(client->os_sd);

       // request irq for this descriptor
        res = um_request_irq(IPNOISE_IO_IRQ, client->os_sd, IRQ_READ,
            os_sock_irq,
            IRQF_DISABLED | IRQF_SHARED,
            "ipnoise_tcp",
            client
        );
        if (res){
            MERROR("Failed to request irq\n");
            goto fail;
        }

// MORIK XXX looks like I have problem here
// connection established but res != 0

/*
        if (    res < 0
            &&  res != -EINPROGRESS)
        {
            MERROR("Connect failed\n");
            goto fail;
        }
*/

        // all ok, now we have connected socket
        client->used    = USED_MAGIC;
        client->state   = CLIENT_STATE_CONNECTING;
    }

    // trying to connect
    if (client && client->state == CLIENT_STATE_CONNECTING){
        // try to connect (XXX now socket still in BLOCKED mode,
        // are You sure what we will not problems here?)
        // freeze as example
        res = os_socket_connect(client->os_sd, &client->os_saddr,
            client->saddr_len);
        MDEBUG("os_socket_connect, res: '%d'\n", res);

        if (!res || res == -EALREADY){
            // XXX be careful, other OS functions
            // may don't known about -EALREADY :(
            // TODO we need emplement os_socket_is_connected method
            client->state = CLIENT_STATE_CONNECTED;
        }
    }

    // if still not connected, go away
    if (    client == NULL
        ||  client->state != CLIENT_STATE_CONNECTED)
    {
        goto fail;
    }

    for (i = 0; i < 3; i++){
        deactivate_fd(client->os_sd, IPNOISE_IO_IRQ);
        res = os_write_file(
            client->os_sd, target, strlen(target)
        );
        MDEBUG("os_socket_sendto descriptor:%d\n%s\nres: %d\n",
            client->os_sd, target, res
        );
        MDEBUG("reactivate_fd(%d, %d)\n", client->os_sd, IPNOISE_IO_IRQ);
        reactivate_fd(client->os_sd, IPNOISE_IO_IRQ);
        if (res >= 0){
            break;
        }
    }


    if (res < 0){
        goto fail;
    }

    dev->trans_start = jiffies;

    dev->stats.tx_packets++;
    dev->stats.tx_bytes += packet_len;

ret:

    if (dst_ip != NULL){
        kfree(dst_ip);
        dst_ip = NULL;
    }
    if (target != NULL){
        kfree(target);
        target = NULL;
    }

    dev_kfree_skb_irq(skb);

    return 0;

fail:
    dev->stats.tx_errors++;
//    if (client != NULL){
//        do_close_conn(client, 1);
//    }
    goto ret;

};

static int ipnoise_tcp_close (struct net_device *dev)
{
    netif_stop_queue(dev);
    return 0;
};

static int ipnoise_tcp_open (struct net_device *dev)
{
    int res, ret = 0;

ret:
    return ret;

fail:
    goto ret;
};

int header_ops_create (struct sk_buff *skb, struct net_device *dev,
    unsigned short type, const void *daddr,
    const void *saddr, unsigned len)
{
//    printk(KERN_DEBUG "%s %s:%d\n", __FILE__, __FUNCTION__, __LINE__);
//    printk(KERN_DEBUG "--- header_ops_create -- {\n");
///	dump_stack();
//    printk(KERN_DEBUG "daddr: '%s'\n", daddr);
//    printk(KERN_DEBUG "saddr: '%s'\n", saddr);
//    printk(KERN_DEBUG "len:   '%d'\n", len);

    int err = 0;
    char *packet    = skb->data;
    int  packet_len = skb->len;

//    printk(KERN_DEBUG "skb->data before:\n");
//    ipnoise_hexdump((unsigned char *)skb->data, skb->len);

    struct ipnoisehdr *hdr = (struct ipnoisehdr *)skb_push(skb,
        IPNOISE_HLEN);

	if (!saddr)
		saddr = dev->dev_addr;

    memcpy(hdr->h_source, saddr, IPNOISE_ALEN);

    if (daddr){
        memcpy(hdr->h_dest, daddr, IPNOISE_ALEN);
        err = IPNOISE_HLEN;
        goto ret;
    }

    err = -IPNOISE_HLEN;

ret:
    packet     = skb->data;
    packet_len = skb->len;

//    printk(KERN_DEBUG "skb->data after:\n");
//    ipnoise_hexdump((unsigned char *)skb->data, skb->len);

//    printk(KERN_DEBUG "--- header_ops_create -- }\n");
    return err;
};

/**
 * header_ops_parse - extract hardware address from packet
 * @skb: packet to extract header from
 * @haddr: destination buffer
 */
int header_ops_parse(const struct sk_buff *skb, unsigned char *haddr)
{
//    printk(KERN_DEBUG "%s %s:%d\n", __FILE__, __FUNCTION__, __LINE__);
//    printk(KERN_DEBUG "--- header_ops_parse -- {\n");
//	dump_stack();

    char *packet    = skb->data;
    int  packet_len = skb->len;

    const struct ipnoisehdr *hdr = (struct ipnoisehdr *)skb->data;
    memcpy(haddr, hdr->h_source, IPNOISE_ALEN);

//    printk(KERN_DEBUG "getting source address: '%s'\n", haddr);
//    ipnoise_hexdump((unsigned char*)skb->data, skb->len);

//    printk(KERN_DEBUG "--- header_ops_parse -- }\n");
    return IPNOISE_ALEN;
};

/**
 * header_ops_rebuild - rebuild the IPNoise MAC header.
 * @skb: socket buffer to update
 *
 * This is called after an ARP or IPV6 ndisc it's resolution on this
 * sk_buff. We now let protocol (ARP) fill in the other fields.
 *
 * This routine CANNOT use cached dst->neigh!
 * Really, it is used only when dst->neigh is wrong.
 */
int header_ops_rebuild(struct sk_buff *skb)
{
    printk(KERN_DEBUG "%s %s:%d\n", __FILE__, __FUNCTION__, __LINE__);
    struct ipnoisehdr *hdr = (struct ipnoisehdr *)skb->data;
    struct net_device *dev = skb->dev;
    memcpy(hdr->h_source, dev->dev_addr, IPNOISE_ALEN);
    return 0;
};

int header_ops_cache(const struct neighbour *neigh, struct hh_cache *hh)
{
    printk(KERN_DEBUG "%s %s:%d\n", __FILE__, __FUNCTION__, __LINE__);
    struct ipnoisehdr *hdr = NULL;

    hdr = (struct ipnoisehdr *)
        (((u8 *) hh->hh_data) + (HH_DATA_OFF(sizeof(*hdr))));

    return -1;
};

void header_ops_cache_update(struct hh_cache *hh,
    const struct net_device *dev,
    const unsigned char *haddr)
{
    printk(KERN_DEBUG "%s %s:%d\n", __FILE__, __FUNCTION__, __LINE__);
    memcpy(((u8 *) hh->hh_data) + HH_DATA_OFF(sizeof(struct ipnoisehdr)),
        haddr, IPNOISE_ALEN);
    return;
};

void ipnoise_tcp_setup(struct net_device *dev)
{
    static const struct header_ops tcp_header_ops = {
        .create           = header_ops_create,
        .parse            = header_ops_parse,
        .rebuild          = header_ops_rebuild,
        .cache            = NULL,   //header_ops_cache,
        .cache_update     = NULL    //header_ops_cache_update
    };

    dev->header_ops         = &tcp_header_ops;

    dev->change_mtu         = ipnoise_tcp_change_mtu;
    dev->set_mac_address    = ipnoise_tcp_mac_addr;
    dev->validate_addr      = ipnoise_tcp_validate_addr;

    dev->type               = ARPHRD_IPNOISE_TCP;

    dev->select_queue       = NULL;
    dev->hard_header_len    = IPNOISE_HLEN;
    dev->mtu                = IPNOISE_TCP_DATA_LEN;
    dev->addr_len           = IPNOISE_ALEN;
    dev->tx_queue_len       = 1;
}

static int __init ipnoise_tcp_init(void)
{
    struct ipnoise_tcp_private *ipnoise_tcp;
    struct sockaddr_in si_me;
    int res, ret = 0, on = 1;
    MINFO("module inited, version %s (%s)\n", MODULE_VER, MODULE_AUTH);

    // trying to create host socket
    listen_sd = os_open_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    MDEBUG("listen_sd: %d", listen_sd);
    if (listen_sd < 0){
        MERROR("Cannot create socket\n");
        ret = listen_sd;
        goto ret;
    }

    // disable blocking
    os_set_fd_async(listen_sd);

    // enable reuse
    res = os_socket_setopt(listen_sd, SOL_SOCKET,  SO_REUSEADDR,
        (char *)&on, sizeof(on));
    if (res){
        MERROR("Failed set SO_REUSEADDR option to socket\n");
        ret = res;
        goto fail_close;
    } else {
        MDEBUG("setsockopt socket: '%d' success\n", listen_sd);
    }

    // set incoming address
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family        = AF_INET;
    si_me.sin_port          = htons(PORT);
    si_me.sin_addr.s_addr   = htonl(INADDR_ANY);

    // trying to bind
    res = os_socket_bind(listen_sd, (const struct sockaddr *)&si_me,
        sizeof(si_me));
    if (res < 0){
        MERROR("************* Cannot bind socket *************\n");
        ret = res;
        goto fail_close;
    } else {
        MDEBUG("bind socket: '%d' success\n", listen_sd);
    }

    // listen backlog
    res = os_socket_listen(listen_sd, BACKLOG_SIZE);
    if (res < 0){
        MDEBUG("************* Cannot listen socket *************\n");
        ret = res;
        goto fail_close;
    } else {
        MDEBUG("listen socket: '%d' success\n", listen_sd);
    }

    // reqiest irq on host read event
    res = um_request_irq(IPNOISE_IO_IRQ, listen_sd, IRQ_READ,
        os_sock_irq,
        IRQF_DISABLED | IRQF_SHARED,
        "ipnoise_tcp",
        &listen_sd
    );
    if (res){
        MDEBUG("Failed to request irq\n");
        ret = res;
        goto fail_close;
    } else {
         MDEBUG("request IRQ: '%d' for socket: '%d' success\n", IPNOISE_IO_IRQ, listen_sd);
    }

    thread = kthread_create(ipnoise_tcp_thread, NULL, "ipnoise_tcp");

    sigaddsetmask(&thread->real_blocked, sigmask(SIGIO));
    snprintf(thread->comm, sizeof(thread->comm) - 1, "ipnoise_tcp");

    wake_up_process(thread);
    MDEBUG("THREAD_PID: '%d'\n", thread->pid);

    // allocate net device
    dev = alloc_netdev_mq(sizeof(struct ipnoise_tcp_private),
        "tcp%d", ipnoise_tcp_setup, 1);
    if (dev == NULL){
        MERROR("Cannot allocate ipnoise tcp network device\n");
        ret = -ENOMEM;
        goto fail_close;
    }

    ipnoise_tcp = netdev_priv(dev);
    ipnoise_tcp->dev = dev;

    dev->open = ipnoise_tcp_open;
    dev->stop = ipnoise_tcp_close;
    dev->hard_start_xmit = ipnoise_tcp_start_xmit;

    strcpy(dev->dev_addr, "0.0.0.0:2210\0");

    res = register_netdev(dev);
    if (res){
        MERROR("Cannot register ipnoise_tcp network device\n");
        ret = res;
        goto fail_free;
    }

    // start queue
    netif_start_queue (dev);

ret:
    return ret;

fail_free:
    free_netdev(dev);

fail_deact:
    free_irq(IPNOISE_IO_IRQ, &listen_sd);

fail_close:
    os_close_socket(listen_sd);
    goto ret;
}

static void __exit ipnoise_tcp_exit(void)
{
    free_irq(IPNOISE_IO_IRQ, &listen_sd);
    os_close_socket(listen_sd);
    kthread_stop(thread);
    if (dev){
        unregister_netdev(dev);
        free_netdev(dev);
    }
    MINFO("module unloaded\n");
}

module_init(ipnoise_tcp_init);
module_exit(ipnoise_tcp_exit);

