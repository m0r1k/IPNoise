#include "ipnoise.h"

#define DRV_NAME    "ipnoise"
#define DRV_VERSION "0.01"

MODULE_AUTHOR("Roman E. Chechnev");
MODULE_DESCRIPTION("Driver for IPNoise");
MODULE_LICENSE("GPL");

static DEFINE_PCI_DEVICE_TABLE(ipnoise_pci_tbl) = {
    { PCI_DEVICE(PCI_VENDOR_ID_IPNOISE, PCI_DEVICE_ID_IPNOISE), },
    { }
};

MODULE_DEVICE_TABLE(pci, ipnoise_pci_tbl);

static          IPNoise ipnoise;
static int32_t  g_debug_level = 0;

static void ipnoise_free_last_packet(
    IPNoisePCIDev *a_pcidev)
{
    if (!a_pcidev){
        goto out;
    }
    if (!a_pcidev->last_packet){
        goto out;
    }
    if (a_pcidev->last_packet->data){
        kfree(a_pcidev->last_packet->data);
        a_pcidev->last_packet->data = NULL;
    }
    a_pcidev->last_packet->data_size = 0;
    kfree(a_pcidev->last_packet);
    a_pcidev->last_packet = NULL;

out:
    return;
}

/*
 *  Change socket state without lock.
 *  Socket must be locked already
 */
static void _set_state_change(
    struct socket   *sock,
    int             new_state)
{
    struct hostos_sock  *hostos = NULL;
    struct sock         *sk     = NULL;

    pdebug(25, "new_state: '%d'\n",
        new_state
    );

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

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
            sk->sk_err  = 0;
            break;

        case HOSTOS_STATE_CONNECTING:
            sock->state = SS_CONNECTING;
            sk->sk_err  = EAGAIN;
            break;

        case HOSTOS_STATE_LISTEN:
            sock->state = SS_CONNECTED;
            break;


        default:
            perr("unknown hostos socket state: '%d'\n",
                new_state
            );
            break;
    };

    sk->sk_state_change(sk);

out:
    return;
fail:
    goto out;
}

static void ipnoise_downlink_process_packet(
    IPNoise         *ipnoise,
    IPNoisePacket   *packet)
{
    struct sock         *sk             = NULL;
    struct hostos_sock  *hostos         = NULL;
    int32_t             do_process      = 0;
    uint32_t            size            = 0;
    int32_t             sk_state_change = 0;
    int32_t             sk_data_ready   = 0;

    sk                  = (struct sock *)packet->os_private;
    hostos              = hostos_sk(sk);
    hostos->poll_mask   = packet->poll_revents;
    sk->sk_err          = packet->last_sock_err;

    if (50 <= g_debug_level){
        ipnoise_dump_packet("ipnoise_downlink_process_packet",
            packet
        );
    }

    switch (packet->type){
        case IPNOISE_PACKET_FREE:
            ipnoise_dump_packet(
                "attempt to process empty uplink packet",
                packet
            );
            break;

        case IPNOISE_PACKET_POLL:
            sk_state_change = 1;

            if (HOSTOS_STATE_CONNECTING == hostos->state){
                if (IPNOISE_POLLIN & hostos->poll_mask){
                    _set_state_change(
                        sk->sk_socket,
                        HOSTOS_STATE_UNCONNECTED
                    );
                } else if ((IPNOISE_POLLOUT & hostos->poll_mask)
                    && !sk->sk_err)
                {
                    _set_state_change(
                        sk->sk_socket,
                        HOSTOS_STATE_CONNECTED
                    );
                }
            }
            break;

        case IPNOISE_PACKET_SENDMSG:
        case IPNOISE_PACKET_RECVMSG:
        case IPNOISE_PACKET_OPEN_SOCKET:
        case IPNOISE_PACKET_CLOSE:
        case IPNOISE_PACKET_CONNECT:
        case IPNOISE_PACKET_BIND:
        case IPNOISE_PACKET_LISTEN:
        case IPNOISE_PACKET_ACCEPT:
        case IPNOISE_PACKET_SOCKOPT_GET:
        case IPNOISE_PACKET_SOCKOPT_SET:
            do_process      = 1;
            sk_state_change = 1;
            break;

        default:
            perr("unsupported packet type: '%d'\n",
                packet->type
            );
            break;
    }

    if (!do_process){
        goto out;
    }

    // copy packet
    size = min(
        (uint32_t)sizeof(*hostos->pcidev.last_packet),
        (uint32_t)sizeof(*packet)
    );
    hostos->pcidev.last_packet = kzalloc(size, GFP_KERNEL);
    if (!hostos->pcidev.last_packet){
        perr("cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            size
        );
        goto fail;
    }
    memcpy(hostos->pcidev.last_packet, packet, size);

    // copy packet data if exist
    if (packet->data_size > 0){
        hostos->pcidev.last_packet->data_size = packet->data_size;
        hostos->pcidev.last_packet->data = kzalloc(
            hostos->pcidev.last_packet->data_size,
            GFP_KERNEL
        );
        if (!hostos->pcidev.last_packet->data){
            perr("cannot allocate memory,"
                " was needed '%d' byte(s)\n",
                hostos->pcidev.last_packet->data_size
            );
            goto fail;
        }
        memcpy(
            hostos->pcidev.last_packet->data,
            packet->data,
            hostos->pcidev.last_packet->data_size
        );
    }

out:
    if (sk_data_ready){
        // wake up anyone sleeping in poll
        sk->sk_data_ready(
            sk,
            hostos->pcidev.last_packet->data_size
        );
    } else if (sk_state_change){
        // wake up anyone sleeping in poll
        sk->sk_state_change(sk);
    }


    return;

fail:
    goto out;
}

static irqreturn_t ipnoise_interrupt(
    int     irq,
    void    *ctx)
{
    int             need_more_irq   = 0;
    uint32_t        cr              = 0;
    IPNoise         *ipnoise        = ctx;
    unsigned char   *dma_uplink_ptr = NULL;
    IPNoiseRegsInfo ri_cmd;

    // read control register
    cr = ipnoise_read_reg(IPNOISE_REG_CR);

    pdebug(20, "ipnoise_interrupt: cr: '%x'\n", cr);

    if (50 <= g_debug_level){
        ipnoise_dump_queue("downlink before", &ipnoise->downlink);
        ipnoise_dump_queue("uplink before", &ipnoise->uplink);
    }

    if (!(cr & IPNOISE_CR_ULDD)){
        // uplink DMA empty, copy packets if exist
        memset(ipnoise->dma_uplink, 0x00, UPLINK_DMA_SIZE);

        dma_uplink_ptr = ipnoise->dma_uplink;

        while (!list_empty(&ipnoise->uplink.list)){
            struct list_head    *first          = NULL;
            IPNoisePacket       *packet         = NULL;
            uint32_t            packet_size     = 0;
            unsigned char       *packet_data    = NULL;

            // ok, list is not empty
            first       = ipnoise->uplink.list.next;
            packet      = list_entry(first, IPNoisePacket, list);
            packet_size = sizeof(*packet) + packet->data_size;

            // check what we have DMA space for this packet
            if ((dma_uplink_ptr + packet_size)
                > (ipnoise->dma_uplink + UPLINK_DMA_SIZE))
            {
                // there are no space for this packet :(
                pr_warn("not enough uplink DMA size:\n"
                    "ipnoise->dma_uplink:   '%x'\n"
                    "dma_uplink_ptr:        '%x'\n"
                    "packet_size:           '%d'\n"
                    "UPLINK_DMA_SIZE:       '%d'\n",
                    (unsigned int)ipnoise->dma_uplink,
                    (unsigned int)dma_uplink_ptr,
                    packet_size,
                    UPLINK_DMA_SIZE
                );
                need_more_irq = 1;
                break;
            }

            // remove from queue
            list_del(first);

            // store packet data pointer and remove it from packet
            packet_data     = packet->data;
            packet->data    = NULL;

            // copy packet header to uplink DMA
            memcpy(dma_uplink_ptr, packet, sizeof(*packet));
            dma_uplink_ptr += sizeof(*packet);

            // copy packet data to uplink DMA
            if (0 < packet->data_size){
                if (packet_data){
                    memcpy(dma_uplink_ptr, packet_data, packet->data_size);
                    dma_uplink_ptr += packet->data_size;
                    kfree(packet_data);
                    packet_data = NULL;
                } else {
                    perr("internal error: packet->data_size > 0 (%d),"
                        " but packet->data is NULL\n",
                        packet->data_size
                    );
                }
            }

            // free packet
            kfree(packet);
        }

        if (dma_uplink_ptr != ipnoise->dma_uplink){
            // mark uplink dma dirty
            CLEAR_REG_INFO(&ri_cmd);
            ri_cmd.cmd_id = IPNOISE_CMD_SET_ULDD;
            _ipnoise_process_command(&ri_cmd);
        }
    }

    if (cr & IPNOISE_CR_DLDD){
        // downlink DMA dirty
        unsigned char *dma_ptr = ipnoise->dma_downlink;
        while ((dma_ptr - ipnoise->dma_downlink) < DOWNLINK_DMA_SIZE){
            IPNoisePacket *packet = (IPNoisePacket *)dma_ptr;

            if (IPNOISE_PACKET_FREE == packet->type){
                // no more packets available
                break;
            }

            if (packet->data_size > 0){
                packet->data = dma_ptr + sizeof(*packet);
            } else {
                packet->data = NULL;
            }

            ipnoise_downlink_process_packet(ipnoise, packet);

            // seek to next packet
            dma_ptr += sizeof(*packet) + packet->data_size;
        }

        {
            // mark downlink dma free
            CLEAR_REG_INFO(&ri_cmd);
            ri_cmd.cmd_id = IPNOISE_CMD_CLEAR_DLDD;
            _ipnoise_process_command(&ri_cmd);
        }
    }

    if (50 <= g_debug_level){
        ipnoise_dump_queue("downlink after",    &ipnoise->downlink);
        ipnoise_dump_queue("uplink after",      &ipnoise->uplink);
    }

    // reset IRQ flag (pci device will flush DMA)
    {
        CLEAR_REG_INFO(&ri_cmd);
        ri_cmd.cmd_id       = IPNOISE_CMD_SET_IRQ_LEVEL;
        ri_cmd.cmd_reg_0    = 0;
        _ipnoise_process_command(&ri_cmd);
    }

    // request more irq
    if (need_more_irq){
        CLEAR_REG_INFO(&ri_cmd);
        ri_cmd.cmd_id    = IPNOISE_CMD_SET_IRQ_LEVEL;
        ri_cmd.cmd_reg_0 = 1;

        _ipnoise_process_command(&ri_cmd);
    }

    pdebug(20, "IRQ_HANDLED\n");
    return IRQ_HANDLED;
}

static u32 ipnoise_read_reg(u32 reg_name)
{
    unsigned long ioaddr = pci_resource_start(ipnoise.pdev, 0);
    return inl(ioaddr + reg_name);
}

// --- ATTENTION ---
// must be atomic and with IRQ disabled
// because few IO operations used
static void _ipnoise_process_command(IPNoiseRegsInfo *ri_cmd)
{
    unsigned long ioaddr = pci_resource_start(ipnoise.pdev, 0);

    outl(ri_cmd->cmd_reg_0, ioaddr + IPNOISE_REG_CMD_0);
    outl(ri_cmd->cmd_reg_1, ioaddr + IPNOISE_REG_CMD_1);
    outl(ri_cmd->cmd_reg_2, ioaddr + IPNOISE_REG_CMD_2);
    outl(ri_cmd->cmd_reg_3, ioaddr + IPNOISE_REG_CMD_3);
    outl(ri_cmd->cmd_reg_4, ioaddr + IPNOISE_REG_CMD_4);

    // writing command ID --- MUSTBE BE LAST ---
    // chip will process command after it ID will wrote
    outl(ri_cmd->cmd_id, ioaddr + IPNOISE_REG_CMD_ID);
}

static void ipnoise_process_command(IPNoiseRegsInfo *ri_cmd)
{
    disable_irq(ipnoise.pdev->irq);
    _ipnoise_process_command(ri_cmd);
    enable_irq(ipnoise.pdev->irq);
}

struct proto hostos_tcp_prot = {
    .name           = "HOSTOS_TCP",
    .owner          = THIS_MODULE,
    .obj_size       = sizeof(struct hostos_sock),
    .setsockopt     = hostos_setsockopt,
    .getsockopt     = hostos_getsockopt
};

struct proto hostos_udp_prot = {
    .name           = "HOSTOS_UDP",
    .owner          = THIS_MODULE,
    .obj_size       = sizeof(struct hostos_sock),
    .setsockopt     = hostos_setsockopt,
    .getsockopt     = hostos_getsockopt
};

struct net_proto_family hostos_family_ops = {
    .family  = PF_HOSTOS,
    .create  = ipnoise_hostos_socket,
    .owner   = THIS_MODULE
};

static ssize_t sysfs_show_debug(
    struct device           *a_dev,
    struct device_attribute *a_attr,
    char                    *a_buf)
{
    return sprintf(a_buf, "%d\n", g_debug_level);
}

static ssize_t sysfs_store_debug(
    struct device           *a_dev,
    struct device_attribute *a_attr,
    const char              *buf,
    size_t                  count)
{
    IPNoiseRegsInfo ri_cmd;
    int32_t         res, tmp = -1;

    // convert string to int
    res = kstrtoint(buf, 10, &tmp);
    if (res){
        perr("kstrtoint failed for: '%s'\n",
            buf
        );
    }

    if (0 <= tmp){
        g_debug_level = tmp;
        pinfo("debug level set to: '%d'\n",
            g_debug_level
        );

        // enable debug
        CLEAR_REG_INFO(&ri_cmd);
        ri_cmd.cmd_id       = IPNOISE_CMD_SET_DEBUG_LEVEL;
        ri_cmd.cmd_reg_0    = g_debug_level;
        ipnoise_process_command(&ri_cmd);
    } else {
        perr("invalid debug level: '%d',"
            " must be integer >= 0\n",
            tmp
        );
    }

    return count;
}

static DEVICE_ATTR(
    debug,
    S_IWUSR | S_IRUGO,
    sysfs_show_debug,
    sysfs_store_debug
);

static struct attribute *ipnoise_sysfs_entries[] = {
    &dev_attr_debug.attr,
    NULL,
};

static struct attribute_group ipnoise_attribute_group = {
    .attrs = ipnoise_sysfs_entries,
};

static int ipnoise_pci_init_one(
    struct pci_dev              *pdev,
    const struct pci_device_id  *ent)
{
    int32_t res = 0;
    int32_t err = -ENODEV;

    if (ipnoise.pdev){
        perr("PCI device already was inited,"
            " only one device is supported\n"
        );
        goto out;
    }

    // store pci device
    ipnoise.pdev = pdev;

    // check that IRQ is assigned
    if (!pdev->irq) {
        perr("IRQ not assigned\n");
        goto fail;
    }

    // check that DMA is supported
    res = pci_dma_supported(pdev, IPNOISE_DMA_MASK);
    if (!res){
        perr("DMA not supported\n");
        goto fail;
    }

    // try to enable device
    res = pci_enable_device(pdev);
    ipnoise.device_enabled = res ? 0 : 1;
    if (0 > res){
        err = res;
        perr("cannot enable device, err: '%d'\n", err);
        goto fail;
    }

    // set master
    pci_set_master(pdev);

    // start resource
    ipnoise.ioaddr = pci_resource_start(pdev, 0);
    if (!ipnoise.ioaddr){
        perr("pci resourse start failed\n");
        goto fail;
    }

    // trying request irq
    res = request_irq(
        pdev->irq,
        ipnoise_interrupt,
        IRQF_SHARED,
        DRV_NAME,
        &ipnoise
    );
    ipnoise.irq_allocated = res ? 0 : pdev->irq;
    if (res){
        perr("request irq failed\n");
        goto fail;
    }

    // disable irq
    disable_irq(pdev->irq);

    // trying request IO ports region
    ipnoise.ioports = request_region(
        ipnoise.ioaddr,
        IPNOISE_IOPORT_SIZE,
        "ipnoise_pci_init_one"
    );
    if (!ipnoise.ioports){
        perr("cannot request PCI IO ports window\n");
        err = -EBUSY;
        goto fail;
    }

    // alloc DMA region
    ipnoise.dma_buffer = dma_alloc_coherent(
        &ipnoise.pdev->dev,
        DMA_TOTAL_SIZE,
        &ipnoise.dma_buffer_addr,
        GFP_KERNEL
    );
    if (!ipnoise.dma_buffer){
        perr("cannot allocate DMA buffer\n");
        err = -ENOMEM;
        goto fail;
    }

    // store DMA ptrs
    ipnoise.dma_downlink = ipnoise.dma_buffer;
    ipnoise.dma_uplink   = ipnoise.dma_buffer
        + DOWNLINK_DMA_SIZE;

    // trying to init device
    res = ipnoise_init_device();
    if (res){
        err = res;
        perr("device init failed\n");
        goto fail;
    }

    // register hostos tcp proto
    err = proto_register(&hostos_tcp_prot, 1);
    if (err){
        perr("cannot register tcp proto\n");
        goto fail;
    }

    // register hostos udp proto
    err = proto_register(&hostos_udp_prot, 1);
    if (err){
        perr("cannot register udp proto\n");
        goto fail;
    }

    // register hostos sock
    (void)sock_register(&hostos_family_ops);

    // init sysfs support
    err = sysfs_create_group(
        &pdev->dev.kobj,
        &ipnoise_attribute_group
    );
    if (err){
        perr("cannot init sysfs support\n");
        goto fail;
    }

    // enable irq
    enable_irq(pdev->irq);

    // all ok
    err = 0;

out:
    return err;

fail:
    if (0 <= err){
        err = -ENODEV;
    }
    ipnoise_uninit_device(pdev);
    goto out;
}

static int32_t ipnoise_init_device()
{
    uint32_t    res;
    int32_t     err = -ENODEV;

    IPNoiseRegsInfo ri_cmd;

    // reset chip
    CLEAR_REG_INFO(&ri_cmd);
    ri_cmd.cmd_id       = IPNOISE_CMD_RESET;
    ri_cmd.cmd_reg_0    = sizeof(IPNoisePacket);
    _ipnoise_process_command(&ri_cmd);

    // wait
    msleep(500);

    // check state, read control register
    res = ipnoise_read_reg(IPNOISE_REG_CR);
    if (!(IPNOISE_CR_SELF_TEST_PASSED & res)){
        perr("device self test failed, "
            " CR value: 0x'%x'\n",
            res
        );
        goto fail;
    }

    // ------------------------- DOWNLINK -----------------------

    // send information about downlink DMA address
    CLEAR_REG_INFO(&ri_cmd);
    ri_cmd.cmd_id       = IPNOISE_CMD_SET_DOWNLINK_DMA_ADDR;
    ri_cmd.cmd_reg_0    = ipnoise.dma_buffer_addr;
    _ipnoise_process_command(&ri_cmd);

    // send information about downlink DMA size
    CLEAR_REG_INFO(&ri_cmd);
    ri_cmd.cmd_id    = IPNOISE_CMD_SET_DOWNLINK_DMA_SIZE;
    ri_cmd.cmd_reg_0 = DOWNLINK_DMA_SIZE;
    _ipnoise_process_command(&ri_cmd);

    // read control register
    res = ipnoise_read_reg(IPNOISE_REG_CR);
    if (!(IPNOISE_CR_DOWNLINK_DMA_READY & res)){
        perr("device init failed, failed setup DMA downlink\n");
        goto fail;
    }

    // ------------------------- UPLINK -------------------------

    // send information about uplink DMA address
    CLEAR_REG_INFO(&ri_cmd);
    ri_cmd.cmd_id       = IPNOISE_CMD_SET_UPLINK_DMA_ADDR;
    ri_cmd.cmd_reg_0    = ipnoise.dma_buffer_addr + DOWNLINK_DMA_SIZE;
    _ipnoise_process_command(&ri_cmd);

    // send information about uplink DMA size
    CLEAR_REG_INFO(&ri_cmd);
    ri_cmd.cmd_id    = IPNOISE_CMD_SET_UPLINK_DMA_SIZE;
    ri_cmd.cmd_reg_0 = UPLINK_DMA_SIZE;
    _ipnoise_process_command(&ri_cmd);

    // read control register
    res = ipnoise_read_reg(IPNOISE_REG_CR);
    if (!(IPNOISE_CR_UPLINK_DMA_READY & res)){
        perr("device init failed,"
            " failed setup DMA uplink\n"
        );
        goto fail;
    }

    // enable debug
    if (g_debug_level){
        CLEAR_REG_INFO(&ri_cmd);
        ri_cmd.cmd_id       = IPNOISE_CMD_SET_DEBUG_LEVEL;
        ri_cmd.cmd_reg_0    = g_debug_level;
        _ipnoise_process_command(&ri_cmd);
    }

    // all ok
    err = 0;

    pr_info("device was inited successfull\n");

out:
    return err;
fail:
    if (0 <= err){
        err = -ENODEV;
    }
    goto out;
}

static int32_t ipnoise_pm_suspend(
    struct pci_dev  *pdev,
    pm_message_t    state)
{
    pci_save_state(pdev);
    return 0;
}

static int32_t ipnoise_pm_resume(
    struct pci_dev *pdev)
{
    return 0;
}

static void ipnoise_uninit_device(
    struct pci_dev *pci_dev)
{
    sysfs_remove_group(
        &pci_dev->dev.kobj,
        &ipnoise_attribute_group
    );

    if (ipnoise.irq_allocated){
        disable_irq(ipnoise.pdev->irq);
        free_irq(
            ipnoise.pdev->irq,
            &ipnoise
        );
        ipnoise.irq_allocated = 0;
    }
    if (ipnoise.dma_buffer){
        dma_free_coherent(
            &ipnoise.pdev->dev,
            DMA_TOTAL_SIZE,
            ipnoise.dma_buffer,
            ipnoise.dma_buffer_addr
        );
        ipnoise.dma_buffer = NULL;
    }
    if (ipnoise.ioports){
        release_region(
            ipnoise.ioaddr,
            IPNOISE_IOPORT_SIZE
        );
        ipnoise.ioports = NULL;
    }
    if (ipnoise.device_enabled){
        pci_disable_device(ipnoise.pdev);
        ipnoise.device_enabled = 0;
    }

    pci_set_drvdata(ipnoise.pdev, NULL);

    ipnoise.pdev = NULL;
}

static void ipnoise_pci_remove_one(
    struct pci_dev *pci_dev)
{
    ipnoise_uninit_device(pci_dev);
}

static int32_t ipnoise_pci_setsockopt(
    struct sock     *a_sk,
    int             a_level,
    int             a_optname,
    char __user     *a_optval,
    unsigned int    a_optlen)
{
    struct hostos_sock  *hostos     = NULL;
    IPNoisePacket       *packet     = NULL;
    long                timeo       = 0;
    int32_t             res         = 0;
    int32_t             error       = 0;
    int32_t             err         = -1;
    int32_t             notcopied   = 0;

    if (!a_sk){
        perr("sock->sk is NULL\n");
        goto out;
    }

    hostos = hostos_sk(a_sk);
    if (!hostos){
        perr("hostos_sk(sk) is NULL\n");
        goto out;
    }

    if (hostos->peer.fd < 0){
        perr("hostos->peer.fd < 0\n");
        err = -ESHUTDOWN;
        goto fail;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        err = -ENOMEM;
        goto fail;
    }

    packet->fd          = hostos->peer.fd;
    packet->type        = IPNOISE_PACKET_SOCKOPT_SET;
    packet->os_private  = (void *)a_sk;
    packet->un.sockopt.level    = a_level;
    packet->un.sockopt.optname  = a_optname;
    packet->un.sockopt.optlen   = a_optlen;

    if (0 < a_optlen){
        packet->data = kzalloc(
            a_optlen,
            GFP_KERNEL
        );
        if (!packet->data){
            perr("cannot allocate memory,"
                " was needed %d byte(s)\n",
                a_optlen
            );
            goto fail;
        }

        packet->data_size = a_optlen;
        notcopied = copy_from_user(
            packet->data,
            a_optval,
            a_optlen
        );
        packet->data_size -= notcopied;
    }

    res = ipnoise_add_uplink(packet);
    if (0 > res){
        goto fail;
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    timeo = sock_sndtimeo(a_sk, 0);

    do {
        if (hostos->pcidev.last_packet){
            // already processed
            break;
        }
        if (!timeo){
            break;
        }
    } while (!ipnoise_wait_for_events(a_sk, &error, &timeo));

    // get result code
    if (error){
        err = error;
    } else if (!timeo && !hostos->pcidev.last_packet){
        err = -EAGAIN;
    } else {
        err = hostos->pcidev.last_packet->res;
    }

out:
    pdebug(20, "after set flags:\n"
        "level:   '%d'\n"
        "optname: '%d'\n"
        "optlen:  '%d'\n"
        "err:     '%d'\n",
        a_level,
        a_optname,
        a_optlen,
        err
    );
    ipnoise_free_last_packet(&hostos->pcidev);
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

static int32_t hostos_setsockopt(
    struct sock     *a_sk,
    int             a_level,
    int             a_optname,
    char __user     *a_optval,
    unsigned int    a_optlen)
{
    int32_t err = -ESHUTDOWN;

    pdebug(20, "hostos_setsockopt,"
        " optname: '%d'\n",
        a_optname
    );

    // check sk
    if (!a_sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    if (    a_sk->sk_err
        ||  (a_sk->sk_shutdown & SEND_SHUTDOWN))
    {
        goto fail_release;
    }

    // send msg to pci device
    err = ipnoise_pci_setsockopt(
        a_sk,
        a_level,
        a_optname,
        a_optval,
        a_optlen
    );

out_release:
    pdebug(20, "hostos_setsockopt, err: '%d'\n", err);
    release_sock(a_sk);

out:
    return err;

fail:
    goto out;

fail_release:
    goto out_release;
}

static int32_t ipnoise_pci_getsockopt(
    struct sock     *a_sk,
    int             a_level,
    int             a_optname,
    char __user     *a_optval,
    int __user      *a_optlen)
{
    struct hostos_sock  *hostos     = NULL;
    IPNoisePacket       *packet     = NULL;
    long                timeo       = 0;
    int32_t             res         = 0;
    int32_t             error       = 0;
    int32_t             err         = -1;
    int32_t             copied      = 0;
    int32_t             notcopied   = 0;
    int32_t             len         = 0;
    uint32_t            size        = 0;

    if (!a_sk){
        perr("sock->sk is NULL\n");
        goto out;
    }

    hostos = hostos_sk(a_sk);
    if (!hostos){
        perr("hostos_sk(sk) is NULL\n");
        goto out;
    }

    // copy len
    size = min(
        (uint32_t)sizeof(len),
        (uint32_t)sizeof(*a_optlen)
    );
    notcopied = copy_from_user(
        &len,
        a_optlen,
        size
    );
    if (notcopied){
        err = -EINTR;
        goto fail;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->fd         = hostos->peer.fd;
    packet->type       = IPNOISE_PACKET_SOCKOPT_GET;
    packet->os_private = (void *)a_sk;
    packet->un.sockopt.level    = a_level;
    packet->un.sockopt.optname  = a_optname;
    packet->un.sockopt.optlen   = len;

    res = ipnoise_add_uplink(packet);
    if (0 > res){
        goto fail;
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    timeo = sock_sndtimeo(a_sk, 0);

    do {
        if (hostos->pcidev.last_packet){
            // already processed
            break;
        }
        if (!timeo){
            break;
        }
    } while (!ipnoise_wait_for_events(a_sk, &error, &timeo));

    // get result code
    if (error){
        err = error;
    } else if (!timeo && !hostos->pcidev.last_packet){
        err = -EAGAIN;
    } else {
        err = hostos->pcidev.last_packet->res;
        size = min(
            (uint32_t)hostos->pcidev.last_packet->data_size,
            (uint32_t)len
        );
        if (0 < size){
            notcopied = copy_to_user(
                a_optval,
                hostos->pcidev.last_packet->data,
                size
            );
            copied = size - notcopied;
        }
        // update copied len
        size = min(
            (uint32_t)sizeof(*a_optlen),
            (uint32_t)sizeof(copied)
        );
        notcopied = copy_to_user(
            a_optlen,
            &copied,
            size
        );
        if (notcopied){
            perr("copy_to_user failed\n");
            err = -EINTR;
        } else {
            err = 0;
        }
    }

out:
    ipnoise_free_last_packet(&hostos->pcidev);
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

static int32_t hostos_getsockopt(
    struct sock     *a_sk,
    int             a_level,
    int             a_optname,
    char __user     *a_optval,
    int __user      *a_optlen)
{
    int32_t err = -ESHUTDOWN;

    // check sk
    if (!a_sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    if (    a_sk->sk_err
        ||  (a_sk->sk_shutdown & SEND_SHUTDOWN))
    {
        goto fail_release;
    }

    // send msg to pci device
    err = ipnoise_pci_getsockopt(
        a_sk,
        a_level,
        a_optname,
        a_optval,
        a_optlen
    );

out_release:
    pdebug(20, "hostos_getsockopt, ret: '%d'\n", err);
    release_sock(a_sk);

out:
    return err;

fail:
    goto out;

fail_release:
    goto out_release;
}

// wait for a packet..
static int32_t ipnoise_wait_for_events(
    struct sock     *sk,
    int             *error,
    long            *timeo_p)
{
    DEFINE_WAIT(wait);
    *error = 0;

    prepare_to_wait(sk_sleep(sk), &wait, TASK_INTERRUPTIBLE);

    // handle signals
    if (signal_pending(current)){
        goto interrupted;
    }

    release_sock(sk);
    *timeo_p = schedule_timeout(*timeo_p);
    lock_sock(sk);

    // handle signals
    if (signal_pending(current)){
        goto interrupted;
    }

out:
    finish_wait(sk_sleep(sk), &wait);
    return *error;

interrupted:
    *error = sock_intr_errno(*timeo_p);
    goto out;
}

static int32_t is_listen(struct socket *sock)
{
    struct hostos_sock  *hostos = NULL;
    struct sock         *sk     = NULL;
    int is_listen               = 0;

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto out;
    }

    hostos = hostos_sk(sk);
    if (    hostos
        &&  hostos->state == HOSTOS_STATE_LISTEN)
    {
        is_listen = 1;
    }

out:
    return is_listen;
}

static int32_t is_shutdown(struct socket *sock)
{
    struct hostos_sock  *hostos = NULL;
    struct sock         *sk     = NULL;
    int is_shutdown             = 0;

    sk = sock->sk;
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

static int32_t is_connected(struct socket *sock)
{
    struct hostos_sock  *hostos = NULL;
    struct sock         *sk     = NULL;
    int is_connected            = 0;

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
        goto fail;
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
fail:
    goto out;
}

static int32_t get_uplink_free_size()
{
    int32_t ret = 0;

    // TODO XXX remove hardcode do true calculation here

    // calculate max uplink dma data size
    ret = UPLINK_DMA_SIZE - 10 * sizeof(IPNoisePacket);

    return ret;
}

static void ipnoise_flush()
{
    IPNoiseRegsInfo ri_cmd;

    // request irq
    CLEAR_REG_INFO(&ri_cmd);
    ri_cmd.cmd_id    = IPNOISE_CMD_SET_IRQ_LEVEL;
    ri_cmd.cmd_reg_0 = 1;

    ipnoise_process_command(&ri_cmd);
}

// must free packet if error
static int32_t ipnoise_add_uplink(IPNoisePacket *packet)
{
    int err = 0;
    list_add_tail(&packet->list, &ipnoise.uplink.list);
    return err;
}

// don't forget lock sk, before call this
static int32_t ipnoise_pci_close(struct sock *sk)
{
    struct hostos_sock  *hostos = NULL;
    IPNoisePacket       *packet = NULL;
    long timeo;
    int32_t res     = 0;
    int32_t error   = 0;
    int32_t err     = -1;

    if (!sk){
        goto out;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        goto out;
    }

    if (hostos->peer.fd < 0){
        goto out;
    }

    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->type    = IPNOISE_PACKET_CLOSE;
    packet->fd      = hostos->peer.fd;

    res = ipnoise_add_uplink(packet);
    if (res < 0){
        goto fail;
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    timeo = sock_sndtimeo(sk, 0);

    do {
        if (hostos->pcidev.last_packet){
            // already processed
            break;
        }
        if (!timeo){
            break;
        }
    } while (!ipnoise_wait_for_events(sk, &error, &timeo));

    // get result code
    if (error){
        err = error;
    } else if (!timeo && !hostos->pcidev.last_packet){
        err = -EAGAIN;
    } else {
        hostos->peer.fd = -1;
        err = hostos->pcidev.last_packet->res;
    }

out:
    ipnoise_free_last_packet(&hostos->pcidev);

    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

static int32_t ipnoise_pci_recvmsg(
    struct socket   *sock,
    struct msghdr   *msg,
    size_t          total_len,
    int             flags)
{
    struct sock         *sk             = NULL;
    struct hostos_sock  *hostos         = NULL;
    IPNoisePacket       *packet         = NULL;
    IPNoisePacket       *last_packet    = NULL;
    long                timeo           = 0;
    int32_t             error           = 0;
    int32_t             res             = 0;
    int32_t             err             = -1;

    int32_t i, non_block = sock->file->f_flags & O_NONBLOCK;

    pdebug(20, "ipnoise_pci_recvmsg:\n"
        "  non_block:   '%d'\n"
        "  total_len:   '%lld'\n",
        non_block,
        (uint64_t)total_len
    );

    pdebug(20, "ipnoise_pci_recvmsg, dump of iovs:\n");
    for (i = 0; i < msg->msg_iovlen; i++){
        struct iovec *iov = msg->msg_iov + i;
        pdebug(20, "iov: 0x%x:\n"
            "  iov_base: 0x%x\n"
            "  iov_len:  0x%lld\n",
            (uint32_t)iov,
            (uint32_t)iov->iov_base,
            (uint64_t)iov->iov_len
        );
    }

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
        goto fail;
    }

    if (0 > hostos->peer.fd){
        perr("0 > hostos->peer.fd\n");
        goto fail;
    }

    // msg flags
    {
        packet = kzalloc(sizeof(*packet), GFP_KERNEL);
        if (!packet){
            perr("cannot allocate memory, was needed %d byte(s)\n",
                sizeof(*packet));
            goto fail;
        }

        packet->fd              = hostos->peer.fd;
        packet->type            = IPNOISE_PACKET_RECVMSG;
        packet->un.recvmsg.type = IPNOISE_MSG_TYPE_FLAGS;
        packet->data            = NULL;
        packet->data_size       = sizeof(flags);

        packet->data = kzalloc(packet->data_size, GFP_KERNEL);
        if (!packet->data){
            perr("cannot allocate memory, was needed %d byte(s)\n",
                packet->data_size);
            goto fail;
        }
        memcpy(packet->data, &flags, packet->data_size);

        res = ipnoise_add_uplink(packet);
        if (res < 0){
            perr("ipnoise_add_uplink failed, res: '%d'\n", res);
            goto fail;
        }
    }

    // msg name
    if (msg->msg_namelen > 0){
        packet = kzalloc(sizeof(*packet), GFP_KERNEL);
        if (!packet){
            perr("cannot allocate memory, was needed %d byte(s)\n",
                sizeof(*packet));
            goto fail;
        }

        packet->fd              = hostos->peer.fd;
        packet->type            = IPNOISE_PACKET_RECVMSG;
        packet->un.recvmsg.type = IPNOISE_MSG_TYPE_NAME;
        packet->data            = NULL;
        packet->data_size       = msg->msg_namelen;

        packet->data = kzalloc(packet->data_size, GFP_KERNEL);
        if (!packet->data){
            perr("cannot allocate memory, was needed %d byte(s)\n",
                packet->data_size);
            goto fail;
        }
        memcpy(packet->data, msg->msg_name, packet->data_size);

        res = ipnoise_add_uplink(packet);
        if (res < 0){
            perr("ipnoise_add_uplink failed, res: '%d'\n", res);
            goto fail;
        }
    }

    // msg iov
    {
        packet = kzalloc(sizeof(*packet), GFP_KERNEL);
        if (!packet){
            perr("cannot allocate memory, was needed %d byte(s)\n",
                sizeof(*packet));
            goto fail;
        }

        packet->fd                              = hostos->peer.fd;
        packet->type                            = IPNOISE_PACKET_RECVMSG;
        packet->un.recvmsg.type                 = IPNOISE_MSG_TYPE_IOV;
        packet->un.recvmsg.un.msg_iov.iov_len   = total_len;

        res = ipnoise_add_uplink(packet);
        if (res < 0){
            perr("ipnoise_add_uplink failed, res: '%d'\n", res);
            goto fail;
        }
    }

    // end
    {
        packet = kzalloc(sizeof(*packet), GFP_KERNEL);
        if (!packet){
            perr("cannot allocate memory, was needed %d byte(s)\n",
                sizeof(*packet));
            goto fail;
        }

        packet->fd              = hostos->peer.fd;
        packet->type            = IPNOISE_PACKET_RECVMSG;
        packet->un.recvmsg.type = IPNOISE_MSG_TYPE_END;
        packet->non_block       = non_block;

        res = ipnoise_add_uplink(packet);
        if (res < 0){
            perr("ipnoise_add_uplink failed, res: '%d'\n", res);
            goto fail;
        }
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    timeo = sock_rcvtimeo(sk, non_block);

    pdebug(20, "recvmsg"
        " non_block: '%d',"
        " timeo: '%ld'\n",
        non_block,
        timeo
    );

    do {
        if (hostos->pcidev.last_packet){
            // already processed
            break;
        }
        // TODO remove this
        // problems with openssh when recvmsg is not sync
        if (!timeo){
           break;
        }
    } while (!ipnoise_wait_for_events(sk, &error, &timeo));

    last_packet = hostos->pcidev.last_packet;

    // get data if exist
    if (    last_packet
        &&  last_packet->data_size > 0
        &&  last_packet->data)
    {
        res = memcpy_toiovec(
            msg->msg_iov,
            last_packet->data,
            last_packet->data_size
        );

        pdebug(20, "memcpy_toiovec '%d' byte(s),"
            " res: '%d'\n",
            last_packet->data_size,
            res
        );

        if (50 <= g_debug_level){
            print_hex_dump_bytes(
                "dump of recv ",
                0,
                last_packet->data,
                last_packet->data_size
            );
        }
    }

    // copy addr if exist
    if (    last_packet
        &&  last_packet->un.recvmsg.un.msg_end.addrlen > 0
        &&  msg->msg_name
        &&  msg->msg_namelen > 0)
    {
        memcpy(
            msg->msg_name,
            &last_packet->un.recvmsg.un.msg_end.addr,
            min(
                (uint32_t)last_packet->un.recvmsg.un.msg_end.addrlen,
                (uint32_t)msg->msg_namelen
            )
        );
    }

    // get result code
    if (error){
        err = error;
    } else if (!timeo && !hostos->pcidev.last_packet){
        err = -EAGAIN;
    } else {
        err = last_packet->res;
    }

out:
    ipnoise_free_last_packet(&hostos->pcidev);
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

static int32_t hostos_recvmsg(
    struct kiocb    *iocb,
    struct socket   *sock,
    struct msghdr   *msg,
    size_t          total_len,
    int             flags)
{
    struct sock         *sk     = NULL;
    struct hostos_sock  *hostos = NULL;
    int copied = 0;

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    lock_sock(sk);

    hostos = hostos_sk(sk);
    if (!hostos){
        copied = 0;
        goto fail_release;
    }

    if (is_shutdown(sk->sk_socket)){
        // socket have closing state..
        copied = 0;
        goto out_release;
    }

    if (!is_connected(sk->sk_socket)){
        copied = -ENOTCONN;
        goto fail_release;
    }

    copied = ipnoise_pci_recvmsg(
        sock,
        msg,
        total_len,
        flags
    );

out_release:
    release_sock(sk);

out:
    pdebug(20, "hostos_recvmsg, copied: '%d'\n", copied);
    return copied;

fail:
    if (copied >= 0){
        copied = -1;
    }
    goto out;

fail_release:
    if (copied >= 0){
        copied = -1;
    }
    goto out_release;
}

// don't forget lock sk, before call this
static int32_t ipnoise_pci_bind(struct socket *sock)
{
    long                timeo   = 0;
    int32_t             error   = 0;
    int32_t             res     = 0;
    int32_t             err     = -1;
    struct sock         *sk     = NULL;
    struct hostos_sock  *hostos = NULL;
    IPNoisePacket       *packet = NULL;

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
        goto fail;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->fd                  = hostos->peer.fd;
    packet->type                = IPNOISE_PACKET_BIND;
    packet->un.bind.addrlen     = min(
        (uint32_t)sizeof(packet->un.bind.addr),
        (uint32_t)hostos->peer.saddrlen
    );
    memcpy(
        &packet->un.bind.addr,
        &hostos->peer.saddr,
        packet->un.bind.addrlen
    );

    res = ipnoise_add_uplink(packet);
    if (res < 0){
        goto fail;
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    timeo = sock_sndtimeo(sk, 0);

    do {
        if (hostos->pcidev.last_packet){
            // already processed
            break;
        }
        if (!timeo){
            break;
        }
    } while (!ipnoise_wait_for_events(sk, &error, &timeo));

    if (error){
        err = error;
    } else if (!timeo && !hostos->pcidev.last_packet){
        err = -EAGAIN;
    } else {
        err = hostos->pcidev.last_packet->res;
    }

out:
    ipnoise_free_last_packet(&hostos->pcidev);
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

static int32_t hostos_bind(
    struct socket       *sock,
    struct sockaddr     *addr,
    int                  addr_len)
{
    struct sock         *sk     = NULL;
    struct hostos_sock  *hostos = NULL;
    int err = -1;

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
        goto fail;
    }

    lock_sock(sk);

    pdebug(20, "bind fd: '%d'\n",
        hostos->peer.fd
    );

    // setup destination address
    hostos->peer.saddrlen = min(
        (uint32_t)sizeof(hostos->peer.saddr),
        (uint32_t)addr_len
    );
    memcpy(
        &hostos->peer.saddr,
        addr,
        hostos->peer.saddrlen
    );

    err = ipnoise_pci_bind(sock);

    release_sock(sk);

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// don't forget lock sk, before call this
static int32_t ipnoise_pci_connect(struct socket *sock)
{
    struct sock         *sk     = NULL;
    struct hostos_sock  *hostos = NULL;
    IPNoisePacket       *packet = NULL;
    long                timeo   = 0;
    int32_t             error   = 0;
    int32_t             res     = 0;
    int32_t             err     = -EIO;

    int32_t non_block = sock->file->f_flags & O_NONBLOCK;

    sk = sock->sk;
    if (!sk){
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
        goto fail;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->fd   = hostos->peer.fd;
    packet->type = IPNOISE_PACKET_CONNECT;
    memcpy(
        &packet->un.connect.daddr,
        &hostos->peer.daddr,
        hostos->peer.daddrlen
    );
    packet->un.connect.daddrlen = hostos->peer.daddrlen;
    packet->non_block           = non_block;

    res = ipnoise_add_uplink(packet);
    if (res < 0){
        goto fail;
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    non_block   = 0;
    timeo       = sock_sndtimeo(sk, non_block);

    // store last socket error
    sk->sk_err = EAGAIN;

    do {
        if (hostos->pcidev.last_packet){
            // already processed
            break;
        }
        if (!timeo){
            break;
        }
    } while (!ipnoise_wait_for_events(sk, &error, &timeo));

    if (error){
        err = error;
    } else if (!timeo && !hostos->pcidev.last_packet){
        err = -EAGAIN;
    } else {
        err = hostos->pcidev.last_packet->res;
    }

out:
    ipnoise_free_last_packet(&hostos->pcidev);
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

static int32_t hostos_connect(
    struct socket       *sock,
    struct sockaddr     *addr,
    int32_t              addrlen,
    int32_t              flags)
{
    int                 err     = -EIO;
    struct hostos_sock  *hostos = NULL;
    struct sock         *sk     = NULL;

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    if (SOCK_STREAM != sock->type){
        err = -ENOTCONN;
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
        err = -EIO;
        goto fail;
    }

    if (is_shutdown(sock)){
        // socket have closing state..
        err = -ESHUTDOWN;
        goto out;
    }

    lock_sock(sk);

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

        // setup destination address
        hostos->peer.daddrlen = min(
            (uint32_t)sizeof(hostos->peer.daddr),
            (uint32_t)addrlen
        );
        memcpy(
            &hostos->peer.daddr,
            addr,
            hostos->peer.daddrlen
        );
        hostos->peer.daddr.sa_family = AF_INET;

        // connect
        err = ipnoise_pci_connect(sock);
    } while (0);

    if (!err){
        _set_state_change(sock, HOSTOS_STATE_CONNECTED);
    }

    // store last socket error
    sk->sk_err = (err < 0) ? -err : err;

    release_sock(sk);
out:
    return err;

fail:
    if (err >= 0){
        err = -EIO;
    }
    goto out;
}

static int32_t hostos_release(
    struct socket *a_sock)
{
    int         err = -1;
    struct sock *sk = NULL;

    sk = a_sock->sk;
    if (!sk){
        goto fail;
    }

    lock_sock(sk);
    err = ipnoise_pci_close(sk);
    if (!err){
        _set_state_change(a_sock, HOSTOS_STATE_SHUTDOWN);
    }
    release_sock(sk);

out:
    return err;
fail:
    goto out;
}

// don't forget lock sk, before call this
static int32_t ipnoise_pci_listen(
    struct socket   *sock,
    int32_t         backlog)
{
    long                timeo   = 0;
    int32_t             error   = 0;
    int32_t             res     = 0;
    int32_t             err     = -1;
    struct sock         *sk     = NULL;
    struct hostos_sock  *hostos = NULL;
    IPNoisePacket       *packet = NULL;

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto out;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
        goto out;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->fd                  = hostos->peer.fd;
    packet->type                = IPNOISE_PACKET_LISTEN;
    packet->un.listen.backlog   = backlog;

    res = ipnoise_add_uplink(packet);
    if (res < 0){
        goto fail;
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    timeo = sock_sndtimeo(sk, 0);

    do {
        if (hostos->pcidev.last_packet){
            // already processed
            break;
        }
        if (!timeo){
            break;
        }
    } while (!ipnoise_wait_for_events(sk, &error, &timeo));

    if (error){
        err = error;
    } else if (!timeo && !hostos->pcidev.last_packet){
        err = -EAGAIN;
    } else {
        err = hostos->pcidev.last_packet->res;
    }

out:
    ipnoise_free_last_packet(&hostos->pcidev);
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

static int32_t hostos_listen(
    struct socket   *sock,
    int             backlog)
{
    struct sock *sk = NULL;
    int         err = -1;

    if (sock->type != SOCK_STREAM){
        err = -ENOTCONN;
        goto out;
    }

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    lock_sock(sk);

    err = ipnoise_pci_listen(sock, backlog);
    if (!err){
        _set_state_change(sock, HOSTOS_STATE_LISTEN);
    }

    release_sock(sk);

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

static int32_t ipnoise_pci_accept(
    struct socket   *sock,
    struct socket   *newsock,
    int32_t         flags,
    struct sockaddr *addr,
    int32_t         *addrlen)
{
    struct sock         *sk     = NULL;
    struct hostos_sock  *hostos = NULL;
    IPNoisePacket       *packet = NULL;
    long                timeo   = 0;
    int32_t             error   = 0;
    int32_t             res     = 0;
    int32_t             err     = -1;

    int32_t non_block = sock->file->f_flags & O_NONBLOCK;

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
        goto fail;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->fd          = hostos->peer.fd;
    packet->type        = IPNOISE_PACKET_ACCEPT;
    packet->non_block   = non_block;

    res = ipnoise_add_uplink(packet);
    if (res < 0){
        goto fail;
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    non_block   = 0;
    timeo       = sock_sndtimeo(sk, non_block);

    do {
        if (hostos->pcidev.last_packet){
            // already processed
            break;
        }
        if (!timeo){
            break;
        }
    } while (!ipnoise_wait_for_events(sk, &error, &timeo));

    if (error){
        *addrlen    = 0;
        err         = error;
    } else if (!timeo && !hostos->pcidev.last_packet){
        *addrlen    = 0;
        err         = -EAGAIN;
    } else {
        err = hostos->pcidev.last_packet->res;
        *addrlen = min(
            (uint32_t)sizeof(*addr),
            (uint32_t)hostos->pcidev.last_packet->un.accept.addrlen
        );
        memcpy(
            addr,
            &hostos->pcidev.last_packet->un.accept.addr,
            *addrlen
        );
    }

out:
    ipnoise_free_last_packet(&hostos->pcidev);
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

static int32_t hostos_accept(
    struct socket   *sock,
    struct socket   *newsock,
    int             flags)
{
    struct sock         *sk         = NULL;
    struct hostos_sock  *hostos     = NULL;
    struct sock         *new_sk     = NULL;
    struct hostos_sock  *new_hostos = NULL;

    struct sockaddr addr;
    int32_t         addrlen;

    int32_t err     = -EINVAL;
    int32_t new_fd  = -1;

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
        goto out;
    }

    if (is_shutdown(sock)){
        // socket have closing state..
        err = -ESHUTDOWN;
        goto fail;
    }

    if (!is_listen(sock)){
        perr("attempt to accept from socket"
            " with hostos->state != HOSTOS_STATE_LISTEN\n");
        goto fail;
    }

    lock_sock(sk);

    // accept new host os descriptor
    new_fd = ipnoise_pci_accept(
        sock,
        newsock,
        flags,
        &addr,
        &addrlen
    );

    if (new_fd < 0){
        err = new_fd;
        goto fail_release;
    }

    // we have new socket accepted
    err = _ipnoise_hostos_socket(
        sock_net(sk),           // net
        newsock,                // new socket
        sk->sk_protocol,        // some protocol
        new_fd                  // new host os descriptor
    );

    if (err){
        goto fail_release;
    }

    new_sk                    = newsock->sk;
    new_hostos                = hostos_sk(new_sk);
    new_hostos->peer.daddrlen = min(
        (uint32_t)sizeof(new_hostos->peer.daddr),
        (uint32_t)addrlen
    );
    memcpy(
        &new_hostos->peer.daddr,
        &addr,
        new_hostos->peer.daddrlen
    );
    new_hostos->peer.saddrlen = min(
        (uint32_t)sizeof(new_hostos->peer.saddr),
        (uint32_t)hostos->peer.saddrlen
    );
    memcpy(
        &new_hostos->peer.saddr,
        &hostos->peer.saddr,
        new_hostos->peer.saddrlen
    );

    // mark as connected
    _set_state_change(newsock, HOSTOS_STATE_CONNECTED);

    // all ok
    err = new_fd;

out_release:
    release_sock(sk);

out:
    pdebug(20, "hostos_accept, err: '%d'\n", err);
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;

fail_release:
    if (err >= 0){
        err = -1;
    }
    goto out_release;
}

static int32_t hostos_getname(
    struct socket   *sock,
    struct sockaddr *addr,
    int             *addr_len,
    int             is_peer)
{
    int                 len     = 0;
    int                 err     = 0;
    struct sock         *sk     = NULL;
    struct hostos_sock  *hostos = NULL;

    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
        goto fail;
    }

    if (is_peer){
        len = min(
            (uint32_t)sizeof(*addr),
            (uint32_t)hostos->peer.daddrlen
        );
        memcpy(addr, &hostos->peer.daddr, len);
        *addr_len = len;
    } else {
        len = min(
            (uint32_t)sizeof(*addr),
            (uint32_t)hostos->peer.saddrlen
        );
        memcpy(addr, &hostos->peer.saddr, len);
        *addr_len = len;
    }

out:
    return err;
fail:
    goto out;
}

static int32_t ipnoise_pci_sendmsg(
    struct socket   *sock,
    struct msghdr   *msg,
    size_t          total_len)
{
    struct sock         *sk         = NULL;
    struct hostos_sock  *hostos     = NULL;
    IPNoisePacket       *packet     = NULL;
    int32_t             iov_id      = 0;
    struct iovec        *iov        = NULL;
    long                timeo       = 0;
    int32_t             res         = 0;
    int32_t             error       = 0;
    int32_t             err         = -1;
    int32_t             size        = 0;
    int32_t             non_block   = 0;

    // get non block state
    non_block = sock->file->f_flags & O_NONBLOCK;

    // get sock
    sk = sock->sk;
    if (!sk){
        perr("sk is NULL\n");
        goto out;
    }

    // get hostos sock
    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
        goto out;
    }

    // check perr fd
    if (hostos->peer.fd < 0){
        err = -ENOTCONN;
        goto out;
    }

    // msg data
    {
        if (1 < msg->msg_iovlen){
            pdebug(20, "iovs dump {\n");
            for (iov_id = 0;
                iov_id < msg->msg_iovlen;
                iov_id++)
            {
                iov = &msg->msg_iov[iov_id];
                pdebug(20, "iov_len: '%d'\n", iov->iov_len);
            }
            pdebug(20, "iovs dump }\n");
        }

        for (iov_id = 0; iov_id < msg->msg_iovlen; iov_id++){
            int32_t         notcopied   = 0;
            int32_t         iov_len     = 0;
            unsigned char   *iov_base   = NULL;

            iov         = &msg->msg_iov[iov_id];
            iov_len     = iov->iov_len;
            iov_base    = iov->iov_base;

            // allocate packet
            packet = kzalloc(sizeof(*packet), GFP_KERNEL);
            if (!packet){
                perr("cannot allocate memory, was needed %d"
                    " byte(s)\n",
                    sizeof(*packet)
                );
                goto fail;
            }

            packet->fd              = hostos->peer.fd;
            packet->type            = IPNOISE_PACKET_SENDMSG;
            packet->un.sendmsg.type = IPNOISE_MSG_TYPE_IOV;
            packet->un.sendmsg.un.msg_iov.id = iov_id;
            packet->data                     = NULL;
            packet->data_size                = min(
                (uint32_t)get_uplink_free_size(),
                (uint32_t)iov_len
            );

            if (0 < packet->data_size){
                packet->data = kzalloc(
                    packet->data_size,
                    GFP_KERNEL
                );
                if (!packet->data){
                    perr("cannot allocate memory,"
                        " was needed %d byte(s)\n",
                        packet->data_size
                    );
                    goto fail;
                }

                notcopied = copy_from_user(
                    packet->data,
                    iov_base,
                    packet->data_size
                );
                packet->data_size -= notcopied;
            }

            res = ipnoise_add_uplink(packet);
            if (0 > res){
                perr("ipnoise_add_uplink failed,"
                    " res: '%d'\n",
                    res
                );
                goto fail;
            }
        }
    }

    // msg name
    if (0 < msg->msg_namelen){
        size   = sizeof(*packet);
        packet = kzalloc(size, GFP_KERNEL);
        if (!packet){
            perr("cannot allocate memory,"
                " was needed %d byte(s)\n",
                size
            );
            goto fail;
        }

        packet->fd              = hostos->peer.fd;
        packet->type            = IPNOISE_PACKET_SENDMSG;
        packet->un.sendmsg.type = IPNOISE_MSG_TYPE_NAME;
        packet->data            = NULL;
        packet->data_size       = msg->msg_namelen;

        packet->data = kzalloc(packet->data_size, GFP_KERNEL);
        if (!packet->data){
            perr("cannot allocate memory,"
                " was needed '%d' byte(s)\n",
                packet->data_size
            );
            goto fail;
        }
        memcpy(
            packet->data,
            msg->msg_name,
            packet->data_size
        );

        res = ipnoise_add_uplink(packet);
        if (0 > res){
            perr("ipnoise_add_uplink failed,"
                " res: '%d'\n",
                res
            );
            goto fail;
        }
    }

    // end
    {
        size   = sizeof(*packet);
        packet = kzalloc(size, GFP_KERNEL);
        if (!packet){
            perr("cannot allocate memory,"
                " was needed %d byte(s)\n",
                size
            );
            goto fail;
        }

        packet->fd              = hostos->peer.fd;
        packet->type            = IPNOISE_PACKET_SENDMSG;
        packet->un.sendmsg.type = IPNOISE_MSG_TYPE_END;
        packet->non_block       = non_block;

        res = ipnoise_add_uplink(packet);
        if (0 > res){
            perr("ipnoise_add_uplink failed,"
                " res: '%d'\n",
                res
            );
            goto fail;
        }
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // If space is not available at the sending socket
    // to hold the message to be transmitted
    // and the socket file descriptor does not have
    // O_NONBLOCK set, the sendmsg() function shall block
    // until space is available. If space is not available
    // at the sending socket to hold the message
    // to be transmitted and the socket file descriptor
    // does have O_NONBLOCK set, the sendmsg() function
    // shall fail.
    non_block = 0; // so every time block here
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/sendmsg.html

    // get timeout
    timeo = sock_sndtimeo(sk, non_block);

    pdebug(20, "sendmsg"
        " non_block: '%d',"
        " timeo: '%ld'\n",
        non_block,
        timeo
    );

    do {
        if (hostos->pcidev.last_packet){
            // already processed
            break;
        }
        if (!timeo){
           break;
        }
    } while (!ipnoise_wait_for_events(sk, &error, &timeo));

    // get result code
    if (error){
        err = error;
    } else if (!timeo && !hostos->pcidev.last_packet){
        err = -EINPROGRESS;
    } else {
        IPNoisePacket *answer = hostos->pcidev.last_packet;
        err = answer->res;
        if (0 && 0 < answer->data_size){
            // update iovs info
            int32_t iovs_count = 0;
            int32_t *iovs      = (int32_t *)answer->data;
            // calculate answers count
            iovs_count = answer->data_size / sizeof(int32_t);
            // update iovs base and length
            for (iov_id = 0; iov_id < iovs_count; iov_id++){
                struct iovec *iov_req = &msg->msg_iov[iov_id];
                int32_t      wrote    = 0;
                // calculate how much was wrote
                wrote = iov_req->iov_len - iovs[iov_id];
                // store new iov length and update base
                iov_req->iov_len  =  iovs[iov_id];
                iov_req->iov_base += wrote;
            }
        }
    }

out:
    ipnoise_free_last_packet(&hostos->pcidev);
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

static int32_t hostos_sendmsg(
    struct kiocb    *iocb,
    struct socket   *sock,
    struct msghdr   *msg,
    size_t          total_len)
{
    struct sock *sk    = NULL;
    int32_t     copied = -1;

    // check sk
    sk = sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto fail;
    }

    if (is_shutdown(sock)){
        // socket have closing state..
        copied = 0;
        perr("Attempt to sendmsg in socket"
            " what have 'shutdown' state\n");
        goto fail;
    }

    if (!is_connected(sock)){
        copied = -ENOTCONN;
        perr("Attempt to sendmsg in socket"
            " what have 'not-connected' state (%d)\n",
            sock->state);
        goto fail;
    }

    lock_sock(sk);

    copied = -EPIPE;
    if (sk->sk_err || (sk->sk_shutdown & SEND_SHUTDOWN)){
        goto fail_release;
    }

    // send msg to pci device
    copied = ipnoise_pci_sendmsg(sock, msg, total_len);

out_release:
    pdebug(20, "hostos_sendmsg, copied: '%d'\n", copied);
    release_sock(sk);

out:
    return copied;

fail:
    if (copied >= 0){
        copied = -1;
    }
    goto out;

fail_release:
    if (copied >= 0){
        copied = -1;
    }
    goto out_release;
}

static uint32_t hostos_poll(
    struct file                 *a_file,
    struct socket               *a_sock,
    struct poll_table_struct    *a_wait)
{
    struct sock         *sk         = NULL;
    struct hostos_sock  *hostos     = NULL;
    IPNoisePacket       *packet     = NULL;
    uint32_t            poll_mask   = 0;
    int32_t             res         = 0;

    sk = a_sock->sk;
    if (!sk){
        perr("sock->sk is NULL\n");
        goto out;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos is NULL\n");
    }

    if (is_shutdown(a_sock)){
        // socket have closing state..
        poll_mask |= POLLERR;
        goto out;
    }

    hostos->poll_wait = poll_requested_events(a_wait);
    pdebug(20, "after poll_requested_events,"
        " poll_wait: 0x'%x'\n",
        hostos->poll_wait
    );

//    if (!hostos->poll_mask){
        sock_poll_wait(a_file, sk_sleep(sk), a_wait);
//    }

    poll_mask           = hostos->poll_mask;
    //hostos->poll_mask   = 0;

    if (0){
        // reset poll events
        IPNoiseRegsInfo ri_cmd;
        CLEAR_REG_INFO(&ri_cmd);
        ri_cmd.cmd_id       = IPNOISE_CMD_SET_POLL_EVENTS;
        ri_cmd.cmd_reg_0    = hostos->peer.fd;
        ri_cmd.cmd_reg_1    = 0;
        ipnoise_process_command(&ri_cmd);

    }

    if (0){
        // prepare packet for pci device
        packet = kzalloc(sizeof(*packet), GFP_KERNEL);
        if (!packet){
            goto fail;
        }

        packet->fd              = hostos->peer.fd;
        packet->type            = IPNOISE_PACKET_POLL;
        packet->un.poll.events  = 0; // clear poll events

        res = ipnoise_add_uplink(packet);
        if (res < 0){
            goto fail;
        }

        // send requests to pci device and sleep
        ipnoise_flush();
    }

    pdebug(20, "after hostos_poll:"
        " host_os->peer.fd: '%d',"
        " poll_mask: '%d'",
        hostos->peer.fd,
        poll_mask
    );

out:
    return poll_mask;
fail:
    goto out;
}

static int32_t hostos_ioctl(
    struct socket   *sock,
    unsigned int    cmd,
    unsigned long   arg)
{
    return 0;
}

// don't forget lock sk, before call this
static int32_t ipnoise_pci_socket(
    struct sock     *sk,
    int32_t         pf_family,
    int32_t         type,
    int32_t         protocol,
    int32_t         fd)
{
    struct hostos_sock  *hostos = NULL;
    IPNoisePacket       *packet = NULL;
    long                timeo   = 0;
    int32_t             res     = 0;
    int32_t             error   = 0;
    int32_t             err     = -1;

    if (!sk){
        perr("sock->sk is NULL\n");
        goto out;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        perr("hostos_sk(sk) is NULL\n");
        goto out;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->fd                          = fd;
    packet->type                        = IPNOISE_PACKET_OPEN_SOCKET;
    packet->os_private                  = (void *)sk;
    packet->un.open_socket.pf_family    = pf_family;
    packet->un.open_socket.type         = type;
    packet->un.open_socket.protocol     = protocol;

    res = ipnoise_add_uplink(packet);
    if (res < 0){
        goto fail;
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    timeo = sock_sndtimeo(sk, 0);

    do {
        if (hostos->pcidev.last_packet){
            // already processed
            break;
        }
        if (!timeo){
            break;
        }
    } while (!ipnoise_wait_for_events(sk, &error, &timeo));

    // get result code
    if (error){
        err = error;
    } else if (!timeo && !hostos->pcidev.last_packet){
        err = -EAGAIN;
    } else {
        err = hostos->pcidev.last_packet->res;
        hostos->peer.fd = err;
    }

out:
    ipnoise_free_last_packet(&hostos->pcidev);
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

const struct proto_ops hostos_proto_ops = {
    .family         = PF_HOSTOS,
    .owner          = THIS_MODULE,
    .recvmsg        = hostos_recvmsg,
    .bind           = hostos_bind,
    .connect        = hostos_connect,
    .release        = hostos_release,
    .listen         = hostos_listen,
    .accept         = hostos_accept,
    .getname        = hostos_getname,
    .socketpair     = sock_no_socketpair,
    .sendmsg        = hostos_sendmsg,
    .poll           = hostos_poll,

    .shutdown       = sock_no_shutdown,
    .setsockopt     = sock_common_setsockopt,
    .getsockopt     = sock_common_getsockopt,
    .ioctl          = hostos_ioctl, // need implement <morik>
    .mmap           = sock_no_mmap,
};

static struct pci_driver ipnoise_driver = {
    .name       = DRV_NAME,
    .probe      = ipnoise_pci_init_one,
    .remove     = ipnoise_pci_remove_one,
    .id_table   = ipnoise_pci_tbl,
    .suspend    = ipnoise_pm_suspend,
    .resume     = ipnoise_pm_resume,
};

// create an hostos socket.
static int32_t _ipnoise_hostos_socket(
    struct net      *net,       // net
    struct socket   *sock,      // socket
    int             protocol,   // protocol
    int             fd)         // Host OS descriptor or -1 if need new
{
    int                 err     = 0;
    struct sock         *sk     = NULL;
    struct hostos_sock  *hostos = NULL;

    // check what we are have support this sock type
    err = -ESOCKTNOSUPPORT;

    if (    sock->type != SOCK_STREAM
        &&  sock->type != SOCK_DGRAM)
    {
        perr("unsupported sock type: '%d' for AF_HOSTOS\n",
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
            perr("failed to allocate PF_HOSTOS TCP socket\n");
            goto fail;
        }
        sock->state = HOSTOS_STATE_UNCONNECTED;
    } else if (IPPROTO_UDP == protocol){
        // good,  now try to allocate UDP socket
        sk = sk_alloc(net, PF_HOSTOS, GFP_KERNEL, &hostos_udp_prot);
        if (sk == NULL){
            err = -ENOBUFS;
            perr("failed to allocate PF_HOSTOS UDP socket\n");
            goto fail;
        }
        sock->state = HOSTOS_STATE_CONNECTED;
    } else {
        err = -ESOCKTNOSUPPORT;
        perr("unsupported protocol: '%d' for PF_HOSTOS\n",
            protocol
        );
        goto fail;
    }

    // lock
    lock_sock(sk);

    // get hostos socket and init states
    hostos = hostos_sk(sk);

    // good, now init socket data
    sock_init_data(sock, sk);

    sk->sk_protocol     = protocol;
    sock->ops           = &hostos_proto_ops;

    // trying to create/setup (for accept) HOSTOS socket
    hostos->peer.fd = ipnoise_pci_socket(
        sk,
        PF_INET,
        sock->type,
        protocol,
        fd
    );

    if (hostos->peer.fd < 0){
        err = -ENETUNREACH;
        goto fail_release;
    }

    // all ok
    err = 0;

    // set states unconnected
    _set_state_change(sock, sock->state);

out_release:
    // release
    release_sock(sk);

out:
    return err;

fail:
    //if (sk != NULL){
    //    sk_common_release(sk);
    //    sk = NULL;
    //}
    goto out;

fail_release:
    goto out_release;
}

static int32_t ipnoise_hostos_socket(
    struct net      *net,
    struct socket   *sock,
    int             protocol,
    int             kern)
{
    int32_t err = -1;
    err =_ipnoise_hostos_socket(net, sock, protocol, -1);
    pdebug(20, "ipnoise_hostos_socket, err: '%d'\n", err);
    return err;
}

static void __exit ipnoise_cleanup_module(void)
{
    if (ipnoise.have_pci){
        pci_unregister_driver(&ipnoise_driver);
        ipnoise.have_pci = 0;
    }
}

static int32_t __init ipnoise_init_module(void)
{
    int res;
    int err = -ENODEV;

    pinfo("%s ver. %s\n", DRV_NAME, DRV_VERSION);

    // init global structure
    memset(&ipnoise, 0x00, sizeof(ipnoise));

    // init uplink, downlink queues
    INIT_LIST_HEAD(&ipnoise.uplink.list);
    INIT_LIST_HEAD(&ipnoise.downlink.list);

    // register pci driver
    res = pci_register_driver(&ipnoise_driver);
    if (res){
        perr("cannot register pci driver\n");
        goto fail;
    }

    ipnoise.have_pci = 1;

    // all ok
    err = 0;

out:
    return err;
fail:
    if (ipnoise.have_pci){
        pci_unregister_driver(&ipnoise_driver);
        ipnoise.have_pci = 0;
    }
    goto out;
}

module_init(ipnoise_init_module);
module_exit(ipnoise_cleanup_module);

