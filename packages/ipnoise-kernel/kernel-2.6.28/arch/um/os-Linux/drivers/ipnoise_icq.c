/*
 * 
 * Feb 25, 2008
 * Roman E. Chechnev <kernel|d|o|g|chechnev.ru>
 *
 */

#include <linux/module.h>
#include <net/sock.h>

#include "ipnoise_icq.h"

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/if_ipnoise_icq.h>

#include "os.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(MODULE_AUTH);
MODULE_DESCRIPTION(MODULE_DESCR);
MODULE_VERSION(MODULE_VER);

struct net_device *dev;

static int icq_mac_addr(struct net_device *dev, void *p)
{
    struct sockaddr *addr = p;

    if (netif_running(dev))
        return -EBUSY;
    if (!is_valid_icq_addr(addr->sa_data))
        return -EADDRNOTAVAIL;
    memcpy(dev->dev_addr, addr->sa_data, IPNOISE_ICQ_ALEN);
    return 0;
}

static int icq_change_mtu(struct net_device *dev, int new_mtu)
{
    if (new_mtu < 1 || new_mtu > IPNOISE_ICQ_DATA_LEN)
        return -EINVAL;
    dev->mtu = new_mtu;
    return 0;
}

static int icq_validate_addr(struct net_device *dev)
{
    if (!is_valid_icq_addr(dev->dev_addr))
        return -EINVAL;

    return 0;
}

static int icq_start_xmit (struct sk_buff *skb,
    struct net_device *dev)
{
    int i;
    struct dst_entry *dst = skb->dst;
    struct neighbour *neighbour = dst->neighbour;
	printk (KERN_EMERG "icq_start_xmit called\n");
    printk ("icq_start_xmit: dst->dev->name: %s\n", dst->dev->name);    

//    char *buffer = kzalloc(1000, 0);
//    for (i = 0; i < MAX_ADDR_LEN; i++){
//        sprintf (buffer + strlen(buffer), "%2.2x ", neighbour->ha[i]);
//    }
//    sprintf(buffer + strlen(buffer), "\n");
    printk("DST HW ADDR: %s\n", neighbour->ha);
    printk("SRC HW ADDR: %s\n", dev->dev_addr);

//    kfree(buffer);
	return 0;
};

struct icq_private
{
	struct net_device	*dev;
};

static int icq_close (struct net_device *dev)
{
// MORIK {
    printk("[MORIK: %s %s:%d]\n",
        __FILE__, __FUNCTION__, __LINE__
    );
// MORIK }
	struct icq_private *icq = netdev_priv(dev);
    netif_stop_queue(dev);
	return 0;
};

static int icq_open (struct net_device *dev)
{
// MORIK {
    printk("[MORIK: %s %s:%d]\n",
        __FILE__, __FUNCTION__, __LINE__
    );
// MORIK }
	struct icq_private *icq = netdev_priv(dev);
	netif_start_queue (dev);
	return 0;
};

void icq_setup(struct net_device *dev)
{
// MORIK {
    printk("[MORIK: %s %s:%d]\n",
        __FILE__, __FUNCTION__, __LINE__
    );
// MORIK }


    dev->change_mtu         = icq_change_mtu;
    dev->set_mac_address    = icq_mac_addr;
    dev->validate_addr      = icq_validate_addr;

    dev->type               = ARPHRD_IPNOISE_ICQ;

    dev->hard_header_len    = IPNOISE_ICQ_HLEN;
    dev->mtu                = IPNOISE_ICQ_DATA_LEN;
    dev->addr_len           = IPNOISE_ICQ_ALEN;
    dev->tx_queue_len       = 1000;
//    dev->flags              = IFF_UP | IFF_DEBUG | IFF_RUNNING;
}

static int __init ipnoise_icq_init(void)
{
	PINFO("module inited, version %s (%s)\n", MODULE_VER, MODULE_AUTH);
	struct icq_private *icq;
    int err;

    dev = alloc_netdev_mq(sizeof(struct icq_private), "icq%d", icq_setup, 1);
    if (dev == NULL){
        PWARN("Cannot allocate icq network device");
        return -ENOMEM;
    }

	icq = netdev_priv(dev);
	icq->dev = dev;

	dev->open = icq_open;
	dev->stop = icq_close;
	dev->hard_start_xmit = icq_start_xmit;

    if (err = register_netdev(dev)){
        free_netdev(dev);
        PWARN("Cannot register icq network device");
        return err;
    }

    strcpy(dev->dev_addr, "260260116\0");

    return 0;
}

static void __exit ipnoise_icq_exit(void)
{
	PINFO("module unloaded\n");
    if (dev){
        unregister_netdev(dev);
        free_netdev(dev);
    }
}

module_init(ipnoise_icq_init);
module_exit(ipnoise_icq_exit);

