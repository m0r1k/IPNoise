/*
 * 
 * Feb 25, 2008
 * Roman E. Chechnev <ipnoise|d|o|g|chechnev.ru>
 *
 */

#include <linux/module.h>
#include <net/sock.h>

#include "ipnoise_udp.h"

#include <linux/mm.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/if_arp_ipnoise.h>
#include <linux/if_ipnoise_udp.h>

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

struct ipnoise_udp_private
{
    struct net_device *dev;
};

#define BUFLEN  65535
#define NPACK   10
#define PORT    2210

static int listen_sd = -1;

DECLARE_WAIT_QUEUE_HEAD(wait_queue);

void wake_up_thread (void)
{
    wake_up(&wait_queue);
};

static irqreturn_t os_sock_irq(int irq, void *data)
{
    wake_up_thread();
    return IRQ_HANDLED;
}

int ipnoise_udp_thread (void *data)
{
    struct sk_buff *skb;
    int    res, skb_len = 0;

    struct  sockaddr_in si_other;
    int     pkt_size, slen = sizeof(si_other);

    // TODO It is never will be free :(
    char    *buf    = kzalloc(BUFLEN, 0);
    char    *target = kzalloc(BUFLEN, 0);

    struct ipnoisehdr *l2hdr = NULL;

    if (buf == NULL || target == NULL){
        MERROR("Buffer alloc failed, was needed: '%d' byte(s)\n",
            BUFLEN);
        goto ret;
    }

    for (;;){
        DECLARE_WAITQUEUE(wait, thread);
        add_wait_queue(&wait_queue, &wait);

        test_and_clear_ti_thread_flag(
            task_thread_info(thread), TIF_SIGPENDING
        );
        set_current_state(TASK_INTERRUPTIBLE);

        sigdelsetmask(&thread->signal->shared_pending.signal,
            sigmask(SIGIO));

        schedule_timeout(1 * HZ);

        set_current_state(TASK_RUNNING);
        remove_wait_queue(&wait_queue, &wait);

        // do our work
        // ********** Prepare outgoing socket *********
        do {
            // prepare receiving
            memset(buf, 0x00, BUFLEN);

            // read from host system
            pkt_size = os_socket_recvfrom(
                listen_sd, buf, BUFLEN,
                0,
                (struct sockaddr *)&si_other,
                (int *)&slen
            );
            if (pkt_size <= 0){
                break;
            }

            MDEBUG("******************************\n");
            MDEBUG("Received packet:\n'%s'\nlen: %d\n", (char *)buf, pkt_size);

            if (pkt_size > BUFLEN){
                MERROR("Big packet received, size: '%d'"
                    " packet will droped\n",
                    pkt_size);
                dev->stats.rx_errors++;
                continue;
            }
            if (slen <= 0){
                MERROR("Invalid source addr size: '%d'"
                    " packet will droped\n",
                    slen);
                dev->stats.rx_errors++;
                continue;
            }

            res      = base64_decode(buf, target, BUFLEN);
            pkt_size = res;
            if (pkt_size <= 0){
                MERROR("Cannot decode packet\n");
                kfree(target);
                dev->stats.rx_errors++;
                continue;
            }

            ipnoise_hexdump((unsigned char *)target, pkt_size);

            skb_len = pkt_size + NET_IP_ALIGN + dev->hard_header_len;
            skb     = netdev_alloc_skb(dev, skb_len);
            if (!likely(skb)){
                MERROR("Cannot alloc skb: '%d' byte(s) was needed\n",
                    skb_len);

                dev->stats.rx_errors++;
                kfree(target);
                continue;
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
                (si_other.sin_addr.s_addr & 0x000000FF),
                (si_other.sin_addr.s_addr & 0x0000FF00) >> 8,
                (si_other.sin_addr.s_addr & 0x00FF0000) >> 16,
                (si_other.sin_addr.s_addr & 0xFF000000) >> 24,
                ntohs(si_other.sin_port)
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

            MDEBUG("Was received packet from: '%s' dev: '%s'\n",
                l2hdr->h_source,
                dev->name
            );

            MDEBUG("Prepare netif_receive_skb skb->head:\n");
            ipnoise_hexdump((unsigned char *)skb->head, (skb->end - skb->head));

            // do kernl receive
            MDEBUG("Now call netif_receive_skb with next skb->data:\n");
            ipnoise_hexdump((unsigned char *)skb->data, skb->len);

            netif_receive_skb (skb);
        } while (1);
        reactivate_fd(listen_sd, IPNOISE_IO_IRQ); 
    }

ret:
    if (buf != NULL){
        kfree(buf);
    }
    if (target != NULL){
        kfree(target);
    }

    return 0;
}

static int ipnoise_udp_mac_addr(struct net_device *dev, void *p)
{
    struct sockaddr *addr = p;

    if (netif_running(dev))
        return -EBUSY;
    if (!is_valid_ipnoise_udp_addr(addr->sa_data))
        return -EADDRNOTAVAIL;
    memcpy(dev->dev_addr, addr->sa_data, IPNOISE_ALEN);
    return 0;
}

static int ipnoise_udp_change_mtu(struct net_device *dev, int new_mtu)
{
    if (new_mtu < 1 || new_mtu > IPNOISE_UDP_DATA_LEN)
        return -EINVAL;
    dev->mtu = new_mtu;
    return 0;
}

static int ipnoise_udp_validate_addr(struct net_device *dev)
{
    if (!is_valid_ipnoise_udp_addr(dev->dev_addr))
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
        printk(KERN_EMERG "Cannot allocate memory\n");
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

static int ipnoise_udp_start_xmit (struct sk_buff *skb,
    struct net_device *dev)
{
    struct sockaddr_in si_other;
    int    i, res, slen = sizeof(si_other);
    unsigned char   *dst_ip;
    unsigned char   *dst_ip_ptr;
    char            *dst_port;
    unsigned int    dst_in_port;
    unsigned int    dst_in_ip;

    struct neighbour *neighbour = NULL;
    struct dst_entry *dst       = NULL;

    int   targetlen = 0;
    char *target    = NULL;

    struct ipnoisehdr *hdr  = (struct ipnoisehdr *)skb->data;
    char *packet            = skb->data;
    int  packet_len         = skb->len;

    if (listen_sd < 0){
        dev->stats.tx_errors++;
        goto ret;
    }

    MDEBUG("-------------- SEND PACKET UDP --------------\n");

    dst_ip = kzalloc(MAX_ADDR_LEN, 0);
    if (!dst_ip){
        MERROR("Cannot allocate memory\n");
        dev->stats.tx_errors++;
        goto ret;
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
        MDEBUG("Invalid address format, must be (ip:port)\n");
        dev->stats.tx_errors++;
        goto free_ret;
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
    target    = kmalloc(targetlen, UM_GFP_KERNEL);
    if (target == NULL){
        dev->stats.tx_errors++;
        goto free_ret;
    }

    base64_encode(packet, packet_len, target, targetlen);
    MDEBUG("Need send base64 text: '%s'\n", target);
    for (i = 0; i < 3; i++){
        deactivate_fd(listen_sd, IPNOISE_IO_IRQ);
        res = os_socket_sendto(
            listen_sd, target, strlen(target),
            MSG_DONTWAIT,
            (const struct sockaddr *)&si_other,
            (int)slen
        );
        reactivate_fd(listen_sd, IPNOISE_IO_IRQ);
        if (res >= 0){
            break;
        }
    }
    kfree(target);

    MDEBUG("os_socket_sendto: res: %d\n", res);
    dev->trans_start = jiffies;

    if (res < 0){
        dev->stats.tx_errors++;
        goto free_ret;
    }

    dev->stats.tx_packets++;
    dev->stats.tx_bytes += packet_len;

free_ret:
    kfree(dst_ip);
ret:
    dev_kfree_skb_irq(skb);

    return 0;
};

static int ipnoise_udp_close (struct net_device *dev)
{
    netif_stop_queue(dev);
    free_irq(IPNOISE_IO_IRQ, &listen_sd);
    return 0;
};


static int ipnoise_udp_open (struct net_device *dev)
{
    int res, ret = 0;

    // reqiest irq on host read event
    res = um_request_irq(IPNOISE_IO_IRQ, listen_sd, IRQ_READ,
        os_sock_irq,
        IRQF_DISABLED | IRQF_SHARED,
        "ipnoise_udp",
        &listen_sd
    );
    if (res){
        MERROR("Failed to request irq\n");
        ret = res;
        goto fail;
    }

    thread = kthread_create(ipnoise_udp_thread, NULL, "ipnoise_udp");

    sigaddsetmask(&thread->real_blocked, sigmask(SIGIO));
    snprintf(thread->comm, sizeof(thread->comm) - 1, "ipnoise_udp");

    wake_up_process(thread);
    MDEBUG("THREAD_PID: '%d'\n", thread->pid);

fail:
    return ret;
};

int header_ops_create (struct sk_buff *skb, struct net_device *dev,
    unsigned short type, const void *daddr,
    const void *saddr, unsigned len)
{
    printk(KERN_DEBUG "%s %s:%d\n", __FILE__, __FUNCTION__, __LINE__);
    printk(KERN_DEBUG "--- header_ops_create -- {\n");
	dump_stack();
    printk(KERN_DEBUG "daddr: '%s'\n", daddr);
    printk(KERN_DEBUG "saddr: '%s'\n", saddr);
    printk(KERN_DEBUG "len:   '%d'\n", len);

    int err = 0;
    char *packet    = skb->data;
    int  packet_len = skb->len;

    printk(KERN_DEBUG "skb->data before:\n");
    ipnoise_hexdump((unsigned char *)skb->data, skb->len);

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

    printk(KERN_DEBUG "skb->data after:\n");
    ipnoise_hexdump((unsigned char *)skb->data, skb->len);

    printk(KERN_DEBUG "--- header_ops_create -- }\n");
    return err;
};

/**
 * header_ops_parse - extract hardware address from packet
 * @skb: packet to extract header from
 * @haddr: destination buffer
 */
int header_ops_parse(const struct sk_buff *skb, unsigned char *haddr)
{
    printk(KERN_DEBUG "%s %s:%d\n", __FILE__, __FUNCTION__, __LINE__);
    printk(KERN_DEBUG "--- header_ops_parse -- {\n");
	dump_stack();

    char *packet    = skb->data;
    int  packet_len = skb->len;

    const struct ipnoisehdr *hdr = (struct ipnoisehdr *)skb->data;
    //ipnoise_hdr(skb);
    memcpy(haddr, hdr->h_source, IPNOISE_ALEN);

    printk(KERN_DEBUG "getting source address: '%s'\n", haddr);
    ipnoise_hexdump((unsigned char*)skb->data, skb->len);

    printk(KERN_DEBUG "--- header_ops_parse -- }\n");
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

void ipnoise_udp_setup(struct net_device *dev)
{
    static const struct header_ops header_ops = {
        .create           = header_ops_create,
        .parse            = header_ops_parse,
        .rebuild          = header_ops_rebuild,
        .cache            = NULL,   //header_ops_cache,
        .cache_update     = NULL    //header_ops_cache_update
    };

    dev->header_ops         = &header_ops;

    dev->change_mtu         = ipnoise_udp_change_mtu;
    dev->set_mac_address    = ipnoise_udp_mac_addr;
    dev->validate_addr      = ipnoise_udp_validate_addr;

    dev->type               = ARPHRD_IPNOISE_UDP;

    dev->select_queue       = NULL;
    dev->hard_header_len    = IPNOISE_HLEN;
    dev->mtu                = IPNOISE_UDP_DATA_LEN;
    dev->addr_len           = IPNOISE_ALEN;
    dev->tx_queue_len       = 1;
}

static int __init ipnoise_udp_init(void)
{
    struct ipnoise_udp_private *ipnoise_udp;
    struct sockaddr_in si_me;
    int res, ret = 0, on = 1;
    MINFO("module inited, version %s (%s)\n", MODULE_VER, MODULE_AUTH);

    // trying to create host socket
    listen_sd = os_open_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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
    }

    // set incoming address
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family        = AF_INET;
    si_me.sin_port          = htons(PORT);
    si_me.sin_addr.s_addr   = htonl(INADDR_ANY);

    // trying to bind
    res = os_socket_bind(listen_sd, (const struct sockaddr *)&si_me, sizeof(si_me));
    if (res < 0){
        MERROR("************* Cannot bind socket *************\n");
        ret = res;
        goto fail_close;
    }

    // allocate net device
    dev = alloc_netdev_mq(sizeof(struct ipnoise_udp_private),
        "udp%d", ipnoise_udp_setup, 1);
    if (dev == NULL){
        MWARN("Cannot allocate ipnoise udp network device");
        ret = -ENOMEM;
        goto fail_close;
    }

    ipnoise_udp = netdev_priv(dev);
    ipnoise_udp->dev = dev;

    dev->open = ipnoise_udp_open;
    dev->stop = ipnoise_udp_close;
    dev->hard_start_xmit = ipnoise_udp_start_xmit;

    strcpy(dev->dev_addr, "0.0.0.0:2210\0");

    res = register_netdev(dev);
    if (res){
        MWARN("Cannot register ipnoise_udp network device");
        ret = res;
        goto fail_free;
    }

    // start queue
    netif_start_queue (dev);

ret:
    return ret;

fail_free:
    free_netdev(dev);
fail_close:
    os_close_socket(listen_sd);
    goto ret;
}

static void __exit ipnoise_udp_exit(void)
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

module_init(ipnoise_udp_init);
module_exit(ipnoise_udp_exit);

