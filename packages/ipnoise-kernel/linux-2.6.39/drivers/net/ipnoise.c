#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

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

static IPNoise ipnoise;

void ipnoise_free_last_packet(IPNoisePCIDev *pcidev)
{
    if (!pcidev){
        goto out;
    }
    if (!pcidev->last_packet){
        goto out;
    }
    if (pcidev->last_packet->data){
        kfree(pcidev->last_packet->data);
        pcidev->last_packet->data = NULL;
    }
    pcidev->last_packet->data_size = 0;
    kfree(pcidev->last_packet);
    pcidev->last_packet = NULL;

out:
    return;
}

/*
 *  Change socket state without lock.
 *  Socket must be locked already
 */
void _set_state_change(struct socket *sock, int new_state)
{
    struct hostos_sock  *hostos = NULL;
    struct sock         *sk     = NULL;

    pr_info("_set_state_change, new_state: '%d'\n", new_state);

    sk = sock->sk;
    if (!sk){
        pr_err("sock->sk is NULL\n");
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
            pr_err("Unknown hostos socket state: '%d'\n", new_state);
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

    sk      = (struct sock *)packet->os_private;
    hostos  = hostos_sk(sk);

    hostos->poll_mask   = packet->poll_revents;
    sk->sk_err          = packet->last_sock_err;

#ifdef IPNOISE_DEBUG
    ipnoise_dump_packet("ipnoise_downlink_process_packet", packet);
#endif

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
            do_process      = 1;
            sk_state_change = 1;
            break;

        default:
            pr_err("unsupported packet type: '%d'\n", packet->type);
            break;
    }

    if (!do_process){
        goto out;
    }

    // copy packet
    size = min(sizeof(*hostos->pcidev.last_packet), sizeof(*packet));
    hostos->pcidev.last_packet = kzalloc(size, GFP_KERNEL);
    if (!hostos->pcidev.last_packet){
        pr_err("cannot allocate memory, was needed: '%d' byte(s)\n",
            size);
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
            pr_err("cannot allocate memory, was needed '%d' byte(s)\n",
            hostos->pcidev.last_packet->data_size);
            goto fail;
        }
        memcpy(
            hostos->pcidev.last_packet->data,
            packet->data,
            hostos->pcidev.last_packet->data_size
        );
    }

out:
    pr_err("ipnoise_downlink_process_packet, sk->sk_err: '%d'\n",
        sk->sk_err);

    if (sk_state_change){
        // wake up anyone sleeping in poll
        sk->sk_state_change(sk);
    }

    if (sk_data_ready){
        // wake up anyone sleeping in poll
        sk->sk_data_ready(
            sk,
            hostos->pcidev.last_packet->data_size
        );
    }

    return;

fail:
    goto out;
}

static irqreturn_t ipnoise_interrupt(int irq, void *ctx)
{
    int             need_more_irq   = 0;
    uint32_t        cr              = 0;
    IPNoise         *ipnoise        = ctx;
    unsigned char   *dma_uplink_ptr = NULL;
    IPNoiseRegsInfo ri_cmd;

    // read control register
    cr = ipnoise_read_reg(IPNOISE_REG_CR);

    pr_debug("ipnoise_interrupt: cr: '%x'\n", cr);

#ifdef IPNOISE_DEBUG
    ipnoise_dump_queue("downlink before", &ipnoise->downlink);
    ipnoise_dump_queue("uplink before", &ipnoise->uplink);
#endif

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

            if (packet_data){
                // copy packet data to uplink DMA
                if (packet->data_size > 0){
                    memcpy(dma_uplink_ptr, packet_data, packet->data_size);
                    dma_uplink_ptr += packet->data_size;
                }
                kfree(packet_data);
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

#ifdef IPNOISE_DEBUG
    ipnoise_dump_queue("downlink after",    &ipnoise->downlink);
    ipnoise_dump_queue("uplink after",      &ipnoise->uplink);
#endif

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

    pr_debug("IRQ_HANDLED\n");
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

static int __devinit ipnoise_probe_pci(
    struct pci_dev              *pdev,
    const struct pci_device_id  *ent)
{
    int             res             = 0;
    unsigned long   ioaddr          = 0;
    int             err             = -ENODEV;
    struct resource *r              = NULL;
    int             device_enabled  = 0;

    if (ipnoise.pdev){
        pr_err("PCI device already was inited,"
            " only one device is supported\n"
        );
        goto out;
    }

    // store pci device
    ipnoise.pdev = pdev;

    // check that IRQ is assigned
    if (!ipnoise.pdev->irq) {
        pr_err("IRQ not assigned\n");
        goto fail;
    }

    // check that DMA is supported
    res = pci_dma_supported(ipnoise.pdev, IPNOISE_DMA_MASK);
    if (!res){
        pr_err("DMA not supported\n");
        goto fail;
    }

    // try to enable device
    res = pci_enable_device(ipnoise.pdev);
    if (res < 0) {
        err = res;
        pr_err("cannot enable device, err: '%d'\n", err);
        goto fail;
    } else {
        // don't forget
        device_enabled = 1;
    }

    // set master
    pci_set_master(ipnoise.pdev);

    // start resource
    ioaddr = pci_resource_start(ipnoise.pdev, 0);
    if (!ioaddr) {
        pr_err("pci resourse start failed\n");
        goto fail;
    }

    // trying request IO region
    r = request_region(
        ioaddr,
        IPNOISE_TOTAL_SIZE,
        "ipnoise_probe_pci"
    );
    if (!r){
        pr_err("cannot request PCI IO ports window\n");
        err = -EBUSY;
        goto fail;
    }

    // trying request irq
    res = request_irq(
        ipnoise.pdev->irq,
        ipnoise_interrupt,
        IRQF_SHARED,
        DRV_NAME,
        &ipnoise
    );
    if (res){
        pr_err("request irq failed\n");
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
        pr_err("cannot allocate DMA buffer\n");
        err = -ENOMEM;
        goto fail;
    }

    // store DMA ptrs
    ipnoise.dma_downlink    = ipnoise.dma_buffer;
    ipnoise.dma_uplink      = ipnoise.dma_buffer + DOWNLINK_DMA_SIZE;

    // trying to init device
    res = ipnoise_init_device();
    if (res){
        err = res;
        pr_err("device init failed\n");
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -ENODEV;
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
    if (device_enabled){
        pci_disable_device(ipnoise.pdev);
    }
    ipnoise.pdev = NULL;
    goto out;
}

static int ipnoise_init_device()
{
    uint32_t res;
    int err = -ENODEV;

    IPNoiseRegsInfo ri_cmd;

    // reset chip
    CLEAR_REG_INFO(&ri_cmd);
    ri_cmd.cmd_id = IPNOISE_CMD_RESET;
    ipnoise_process_command(&ri_cmd);

    // read control register
    res = ipnoise_read_reg(IPNOISE_REG_CR);
    if (res != IPNOISE_CR_SELF_TEST_PASSED){
        pr_err("device self test failed\n");
        goto fail;
    }

    // ------------------------- DOWNLINK -----------------------

    // send information about downlink DMA address
    CLEAR_REG_INFO(&ri_cmd);
    ri_cmd.cmd_id       = IPNOISE_CMD_SET_DOWNLINK_DMA_ADDR;
    ri_cmd.cmd_reg_0    = ipnoise.dma_buffer_addr;
    ipnoise_process_command(&ri_cmd);

    // send information about downlink DMA size
    CLEAR_REG_INFO(&ri_cmd);
    ri_cmd.cmd_id    = IPNOISE_CMD_SET_DOWNLINK_DMA_SIZE;
    ri_cmd.cmd_reg_0 = DOWNLINK_DMA_SIZE;
    ipnoise_process_command(&ri_cmd);

    // read control register
    res = ipnoise_read_reg(IPNOISE_REG_CR);
    if (!(res & IPNOISE_CR_DOWNLINK_DMA_READY)){
        pr_err("device init failed, failed setup DMA downlink\n");
        goto fail;
    }

    // ------------------------- UPLINK -------------------------

    // send information about uplink DMA address
    CLEAR_REG_INFO(&ri_cmd);
    ri_cmd.cmd_id       = IPNOISE_CMD_SET_UPLINK_DMA_ADDR;
    ri_cmd.cmd_reg_0    = ipnoise.dma_buffer_addr + DOWNLINK_DMA_SIZE;
    ipnoise_process_command(&ri_cmd);

    // send information about uplink DMA size
    CLEAR_REG_INFO(&ri_cmd);
    ri_cmd.cmd_id    = IPNOISE_CMD_SET_UPLINK_DMA_SIZE;
    ri_cmd.cmd_reg_0 = UPLINK_DMA_SIZE;
    ipnoise_process_command(&ri_cmd);

    // read control register
    res = ipnoise_read_reg(IPNOISE_REG_CR);
    if (!(res & IPNOISE_CR_UPLINK_DMA_READY)){
        pr_err("device init failed, failed setup DMA uplink\n");
        goto fail;
    }

    // all ok
    err = 0;

    pr_info("device was inited successfull\n");

out:
    return err;
fail:
    if (err >= 0){
        err = -ENODEV;
    }
    goto out;
}

static int ipnoise_pm_suspend(
    struct pci_dev  *pdev,
    pm_message_t    state)
{
    pci_save_state(pdev);
    return 0;
}

static int ipnoise_pm_resume(struct pci_dev *pdev)
{
    return 0;
}

static void __devexit ipnoise_remove_one(struct pci_dev *pdev)
{
    pci_disable_device(pdev);
    pci_set_drvdata(pdev, NULL);
    ipnoise.pdev = NULL;
}

int hostos_setsockopt(
    struct sock     *sk,
    int             level,
	int             optname,
    char __user     *optval,
    unsigned int    optlen)
{
    pr_warn("hostos_setsockopt\n");
    return -1;
}

int hostos_getsockopt(
    struct sock     *sk,
    int             level,
    int             optname,
    char __user     *optval,
    int __user      *option)
{
    pr_warn("hostos_getsockopt\n");
    return -1;
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

// wait for a packet..
int ipnoise_wait_for_events(
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

int is_listen(struct socket *sock)
{
    struct hostos_sock  *hostos = NULL;
    struct sock         *sk     = NULL;
    int is_listen               = 0;

    sk = sock->sk;
    if (!sk){
        pr_err("sock->sk is NULL\n");
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

int is_shutdown(struct socket *sock)
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

int is_connected(struct socket *sock)
{
    struct hostos_sock  *hostos = NULL;
    struct sock         *sk     = NULL;
    int is_connected            = 0;

    sk = sock->sk;
    if (!sk){
        pr_err("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
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
static int ipnoise_add_uplink(IPNoisePacket *packet)
{
    int err = 0;
    list_add_tail(&packet->list, &ipnoise.uplink.list);
    return err;
}

// don't forget lock sk, before call this
static int ipnoise_pci_close(struct sock *sk)
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

    packet->type            = IPNOISE_PACKET_CLOSE;
    packet->un.close.fd     = hostos->peer.fd;

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
        err = -hostos->pcidev.last_packet->last_sock_err;
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

int ipnoise_pci_recvmsg(
    struct socket   *sock,
    struct msghdr   *msg,
    size_t          total_len,
    int             flags)
{
    struct sock         *sk                         = NULL;
    struct hostos_sock  *hostos                     = NULL;
    IPNoisePacket       *packet                     = NULL;
    IPNoisePacket       *last_packet                = NULL;
    long                timeo                       = 0;
    int32_t             error                       = 0;
    int32_t             res                         = 0;
    int32_t             err                         = -1;

    int32_t non_block = sock->file->f_flags & O_NONBLOCK;

    sk = sock->sk;
    if (!sk){
        pr_err("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
        goto fail;
    }

    if (hostos->peer.fd < 0){
        pr_err("hostos->peer.fd < 0\n");
        goto fail;
    }

    // msg name
    if (msg->msg_namelen > 0){
        packet = kzalloc(sizeof(*packet), GFP_KERNEL);
        if (!packet){
            pr_err("cannot allocate memory, was needed %d byte(s)\n",
                sizeof(*packet));
            goto fail;
        }

        packet->type            = IPNOISE_PACKET_RECVMSG;
        packet->un.recvmsg.type = IPNOISE_MSG_TYPE_NAME;
        packet->un.recvmsg.fd   = hostos->peer.fd;
        packet->data            = NULL;
        packet->data_size       = msg->msg_namelen;

        packet->data = kzalloc(packet->data_size, GFP_KERNEL);
        if (!packet->data){
            pr_err("cannot allocate memory, was needed %d byte(s)\n",
                packet->data_size);
            goto fail;
        }
        memcpy(packet->data, msg->msg_name, packet->data_size);

        res = ipnoise_add_uplink(packet);
        if (res < 0){
            pr_err("ipnoise_add_uplink failed, res: '%d'\n", res);
            goto fail;
        }
    }

    // msg iov
    {
        packet = kzalloc(sizeof(*packet), GFP_KERNEL);
        if (!packet){
            pr_err("cannot allocate memory, was needed %d byte(s)\n",
                sizeof(*packet));
            goto fail;
        }

        packet->type                            = IPNOISE_PACKET_RECVMSG;
        packet->un.recvmsg.type                 = IPNOISE_MSG_TYPE_IOV;
        packet->un.recvmsg.fd                   = hostos->peer.fd;
        packet->un.recvmsg.un.msg_iov.iov_len   = total_len;

        res = ipnoise_add_uplink(packet);
        if (res < 0){
            pr_err("ipnoise_add_uplink failed, res: '%d'\n", res);
            goto fail;
        }
    }

    // end
    {
        packet = kzalloc(sizeof(*packet), GFP_KERNEL);
        if (!packet){
            pr_err("cannot allocate memory, was needed %d byte(s)\n",
                sizeof(*packet));
            goto fail;
        }

        packet->type                        = IPNOISE_PACKET_RECVMSG;
        packet->un.recvmsg.type             = IPNOISE_MSG_TYPE_END;
        packet->un.recvmsg.fd               = hostos->peer.fd;
        packet->un.recvmsg.un.msg_end.res   = -1;
        packet->non_block                   = non_block;

        res = ipnoise_add_uplink(packet);
        if (res < 0){
            pr_err("ipnoise_add_uplink failed, res: '%d'\n", res);
            goto fail;
        }
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    timeo = sock_rcvtimeo(sk, non_block);

    do {
        if (hostos->pcidev.last_packet){
            // already processed
            break;
        }
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
                last_packet->un.recvmsg.un.msg_end.addrlen,
                msg->msg_namelen
            )
        );
    }

    // get result code
    if (error){
        err = error;
    } else if (!timeo && !hostos->pcidev.last_packet){
        err = -EAGAIN;
    } else {
        err = last_packet->un.recvmsg.un.msg_end.res;
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

int hostos_recvmsg(
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
        pr_err("sock->sk is NULL\n");
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

    copied = ipnoise_pci_recvmsg(sock, msg, total_len, flags);

out_release:
    release_sock(sk);

out:
    pr_debug("hostos_recvmsg, copied: '%d'\n", copied);
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
int ipnoise_pci_bind(struct socket *sock)
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
        pr_err("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
        goto fail;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->type                = IPNOISE_PACKET_BIND;
    packet->un.bind.fd          = hostos->peer.fd;
    packet->un.bind.addrlen     = min(
        sizeof(packet->un.bind.addr),
        hostos->peer.saddrlen
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
        err = -hostos->pcidev.last_packet->last_sock_err;
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

int hostos_bind(
    struct socket       *sock,
    struct sockaddr     *addr,
    int                  addr_len)
{
    struct sock         *sk     = NULL;
    struct hostos_sock  *hostos = NULL;
    int err = -1;

    sk = sock->sk;
    if (!sk){
        pr_err("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
        goto fail;
    }

    lock_sock(sk);

    // setup destination address
    hostos->peer.saddrlen = min(
        sizeof(hostos->peer.saddr),
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
int ipnoise_pci_connect(struct socket *sock)
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
        pr_err("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
        goto fail;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->type            = IPNOISE_PACKET_CONNECT;
    packet->un.connect.fd   = hostos->peer.fd;
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
    timeo = sock_sndtimeo(sk, non_block);

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
        err = -hostos->pcidev.last_packet->last_sock_err;
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

int hostos_connect(
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
        pr_err("sock->sk is NULL\n");
        goto fail;
    }

    if (SOCK_STREAM != sock->type){
        err = -ENOTCONN;
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
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
            sizeof(hostos->peer.daddr),
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

int hostos_release(struct socket *sock)
{
    int         err = -1;
    struct sock *sk = NULL;

    sk = sock->sk;
    if (!sk){
        pr_err("sock->sk is NULL\n");
        goto fail;
    }

    lock_sock(sk);
    err = ipnoise_pci_close(sk);
    if (!err){
        _set_state_change(sock, HOSTOS_STATE_SHUTDOWN);
    }
    release_sock(sk);

out:
    return err;
fail:
    goto out;
}

// don't forget lock sk, before call this
int32_t ipnoise_pci_listen(
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
        pr_err("sock->sk is NULL\n");
        goto out;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
        goto out;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->type                = IPNOISE_PACKET_LISTEN;
    packet->un.listen.fd        = hostos->peer.fd;
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
        err = -hostos->pcidev.last_packet->last_sock_err;
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

int hostos_listen(struct socket *sock, int backlog)
{
    struct sock *sk = NULL;
    int         err = -1;

    if (sock->type != SOCK_STREAM){
        err = -ENOTCONN;
        goto out;
    }

    sk = sock->sk;
    if (!sk){
        pr_err("sock->sk is NULL\n");
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

int ipnoise_pci_accept(
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
        pr_err("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
        goto fail;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->type                    = IPNOISE_PACKET_ACCEPT;
    packet->un.accept.fd            = hostos->peer.fd;
    packet->non_block               = non_block;

    res = ipnoise_add_uplink(packet);
    if (res < 0){
        goto fail;
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    timeo = sock_sndtimeo(sk, non_block);

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
        err = hostos->pcidev.last_packet->un.accept.res;
        *addrlen = min(
            sizeof(*addr),
            hostos->pcidev.last_packet->un.accept.addrlen
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

int hostos_accept(
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
        pr_err("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
        goto out;
    }

    if (is_shutdown(sock)){
        // socket have closing state..
        err = -ESHUTDOWN;
        goto fail;
    }

    if (!is_listen(sock)){
        pr_err("attempt to accept from socket"
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
        sizeof(new_hostos->peer.daddr),
        (uint32_t)addrlen
    );
    memcpy(
        &new_hostos->peer.daddr,
        &addr,
        new_hostos->peer.daddrlen
    );
    new_hostos->peer.saddrlen = min(
        sizeof(new_hostos->peer.saddr),
        hostos->peer.saddrlen
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
    pr_debug("hostos_accept, err: '%d'\n", err);
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

int hostos_getname(
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
        pr_err("sock->sk is NULL\n");
        goto fail;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
        goto fail;
    }

    if (is_peer){
        len = min(sizeof(*addr), hostos->peer.daddrlen);
        memcpy(addr, &hostos->peer.daddr, len);
        *addr_len = len;
    } else {
        len = min(sizeof(*addr), hostos->peer.saddrlen);
        memcpy(addr, &hostos->peer.saddr, len);
        *addr_len = len;
    }

out:
    return err;
fail:
    goto out;
}

int32_t ipnoise_pci_sendmsg(
    struct socket   *sock,
    struct msghdr   *msg,
    size_t          total_len)
{
    struct sock         *sk                         = NULL;
    struct hostos_sock  *hostos                     = NULL;
    IPNoisePacket       *packet                     = NULL;
    int32_t             max_uplink_dma_data_size    = 0;
    int32_t             iov_id                      = 0;
    struct iovec        *iov                        = NULL;
    long                timeo                       = 0;
    int32_t             res                         = 0;
    int32_t             error                       = 0;
    int32_t             err                         = -1;

    int32_t non_block = sock->file->f_flags & O_NONBLOCK;

    sk = sock->sk;
    if (!sk){
        pr_err("sk is NULL\n");
        goto out;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
        goto out;
    }

    if (hostos->peer.fd < 0){
        goto out;
    }

    // calculate max uplink dma data size
    max_uplink_dma_data_size = UPLINK_DMA_SIZE
        - 10 * sizeof(*packet);

    // msg data
    {
        // search first iov with data for send
        for (iov_id = 0; iov_id < msg->msg_iovlen; iov_id++){

            iov = &msg->msg_iov[iov_id];
            if (iov->iov_len > 0){
                // we have found iov with data
                break;
            }

            // iov with data yet not found
            iov = NULL;
        }

        if (!iov){
            // nothing to send
            err = -1;
            goto out;
        }

        {
            int32_t         notcopied   = 0;
            int32_t         iov_len     = 0;
            unsigned char   *iov_base   = NULL;

            iov_len  = iov->iov_len;
            iov_base = iov->iov_base;

            packet = kzalloc(sizeof(*packet), GFP_KERNEL);
            if (!packet){
                pr_err("cannot allocate memory, was needed %d"
                    " byte(s)\n",
                    sizeof(*packet));
                goto fail;
            }

            packet->type                = IPNOISE_PACKET_SENDMSG;
            packet->un.sendmsg.type     = IPNOISE_MSG_TYPE_IOV;
            packet->un.sendmsg.fd       = hostos->peer.fd;
            packet->data                = NULL;
            packet->data_size           = min(
                max_uplink_dma_data_size,
                iov_len
            );

            packet->data = kzalloc(packet->data_size, GFP_KERNEL);
            if (!packet->data){
                pr_err("cannot allocate memory, was needed %d byte(s)\n",
                    packet->data_size);
                goto fail;
            }

           notcopied = copy_from_user(
               packet->data,
               iov_base,
               packet->data_size
           );
           packet->data_size -= notcopied;

           res = ipnoise_add_uplink(packet);
           if (res < 0){
               pr_err("ipnoise_add_uplink failed, res: '%d'\n", res);
               goto fail;
           }
        }
    }

    // msg name
    if (msg->msg_namelen > 0){
        packet = kzalloc(sizeof(*packet), GFP_KERNEL);
        if (!packet){
            pr_err("cannot allocate memory, was needed %d byte(s)\n",
                sizeof(*packet));
            goto fail;
        }

        packet->type            = IPNOISE_PACKET_SENDMSG;
        packet->un.sendmsg.type = IPNOISE_MSG_TYPE_NAME;
        packet->un.sendmsg.fd   = hostos->peer.fd;
        packet->data            = NULL;
        packet->data_size       = msg->msg_namelen;

        packet->data = kzalloc(packet->data_size, GFP_KERNEL);
        if (!packet->data){
            pr_err("cannot allocate memory, was needed %d byte(s)\n",
                packet->data_size);
            goto fail;
        }
        memcpy(packet->data, msg->msg_name, packet->data_size);

        res = ipnoise_add_uplink(packet);
        if (res < 0){
            pr_err("ipnoise_add_uplink failed, res: '%d'\n", res);
            goto fail;
        }
    }

    // end
    {
        packet = kzalloc(sizeof(*packet), GFP_KERNEL);
        if (!packet){
            pr_err("cannot allocate memory, was needed %d byte(s)\n",
                sizeof(*packet));
            goto fail;
        }

        packet->type                        = IPNOISE_PACKET_SENDMSG;
        packet->un.sendmsg.type             = IPNOISE_MSG_TYPE_END;
        packet->un.sendmsg.fd               = hostos->peer.fd;
        packet->un.sendmsg.un.msg_end.res   = -1;
        packet->non_block                   = non_block;

        res = ipnoise_add_uplink(packet);
        if (res < 0){
            pr_err("ipnoise_add_uplink failed, res: '%d'\n", res);
            goto fail;
        }
    }

    // send requests to pci device and sleep
    ipnoise_flush();

    // get timeout
    timeo = sock_sndtimeo(sk, non_block);

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
        err = hostos->pcidev.last_packet->un.sendmsg.un.msg_end.res;
        if (err > 0){
            iov->iov_len    -= err;
            iov->iov_base   += err;
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

int hostos_sendmsg(
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
        pr_err("sock->sk is NULL\n");
        goto fail;
    }

    if (is_shutdown(sock)){
        // socket have closing state..
        copied = 0;
        pr_err("Attempt to sendmsg in socket"
            " what have 'shutdown' state\n");
        goto fail;
    }

    if (!is_connected(sock)){
        copied = -ENOTCONN;
        pr_err("Attempt to sendmsg in socket"
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
    pr_debug("hostos_sendmsg, copied: '%d'\n", copied);
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

unsigned int hostos_poll(
    struct file                 *file,
    struct socket               *sock,
    struct poll_table_struct    *wait)
{
    unsigned int mask           = 0;
    struct sock         *sk     = NULL;
    struct hostos_sock  *hostos = NULL;

    sk = sock->sk;
    if (!sk){
        pr_err("sock->sk is NULL\n");
        goto out;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos is NULL\n");
    }

    if (is_shutdown(sock)){
        // socket have closing state..
        mask |= POLLERR;
       goto out;
    }

    pr_debug("before poll_wait, hostos->poll_mask: 0x'%x'\n",
        (unsigned int)hostos->poll_mask);

    poll_wait(file, sk_sleep(sk), wait);

out:
    pr_debug("after poll_wait, hostos->poll_mask: 0x'%x'\n",
        (unsigned int)hostos->poll_mask);
    return hostos->poll_mask;
}

int hostos_shutdown(struct socket *sock, int flags)
{
    return -EINVAL;
}

int hostos_ioctl(
    struct socket   *sock,
    unsigned int    cmd,
    unsigned long   arg)
{
    return 0;
}

const struct proto_ops hostos_proto_ops = {
    .family         = PF_HOSTOS,
    .owner          = THIS_MODULE,
    .recvmsg        = hostos_recvmsg,           // done
    .bind           = hostos_bind,              // done
    .connect        = hostos_connect,           // done
    .release        = hostos_release,           // done
    .listen         = hostos_listen,            // done
    .accept         = hostos_accept,            // done
    .getname        = hostos_getname,           // done
    .socketpair     = sock_no_socketpair,       // done
    .sendmsg        = hostos_sendmsg,           // done
    .poll           = hostos_poll,

    .shutdown       = hostos_shutdown,
    .setsockopt     = sock_common_setsockopt,
    .getsockopt     = sock_common_getsockopt,
    .ioctl          = hostos_ioctl, // need implement <morik>
    .mmap           = sock_no_mmap,
};

// don't forget lock sk, before call this
static int ipnoise_pci_socket(
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
        pr_err("sock->sk is NULL\n");
        goto out;
    }

    hostos = hostos_sk(sk);
    if (!hostos){
        pr_err("hostos_sk(sk) is NULL\n");
        goto out;
    }

    // prepare packet for pci device
    packet = kzalloc(sizeof(*packet), GFP_KERNEL);
    if (!packet){
        goto fail;
    }

    packet->type                        = IPNOISE_PACKET_OPEN_SOCKET;
    packet->os_private                  = (void *)sk;
    packet->un.open_socket.pf_family    = pf_family;
    packet->un.open_socket.type         = type;
    packet->un.open_socket.protocol     = protocol;
    packet->un.open_socket.fd           = fd;

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
        err = hostos->pcidev.last_packet->un.open_socket.fd;
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

// create an hostos socket.
static int _ipnoise_hostos_socket(
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
        pr_err("Unsupported sock type: '%d' for AF_HOSTOS\n",
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
            pr_err("Failed to allocate PF_HOSTOS TCP socket\n");
            goto fail;
        }
        sock->state = HOSTOS_STATE_UNCONNECTED;
    } else if (IPPROTO_UDP == protocol){
        // good,  now try to allocate UDP socket
        sk = sk_alloc(net, PF_HOSTOS, GFP_KERNEL, &hostos_udp_prot);
        if (sk == NULL){
            err = -ENOBUFS;
            pr_err("Failed to allocate PF_HOSTOS UDP socket\n");
            goto fail;
        }
        sock->state = HOSTOS_STATE_CONNECTED;
    } else {
        err = -ESOCKTNOSUPPORT;
        pr_err("Unsupported protocol: '%d' for PF_HOSTOS\n",
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

static int ipnoise_hostos_socket(
    struct net      *net,
    struct socket   *sock,
    int             protocol,
    int             kern)
{
    return _ipnoise_hostos_socket(net, sock, protocol, -1);
}

struct net_proto_family hostos_family_ops = {
    .family  = PF_HOSTOS,
    .create  = ipnoise_hostos_socket,
    .owner   = THIS_MODULE
};

static struct pci_driver ipnoise_driver = {
    .name       = DRV_NAME,
    .probe      = ipnoise_probe_pci,
    .remove     = __devexit_p(ipnoise_remove_one),
    .id_table   = ipnoise_pci_tbl,
    .suspend    = ipnoise_pm_suspend,
    .resume     = ipnoise_pm_resume,
};

static void __exit ipnoise_cleanup_module(void)
{
    if (ipnoise.have_pci){
        pci_unregister_driver(&ipnoise_driver);
        ipnoise.have_pci = 0;
    }
}


static int __init ipnoise_init_module(void)
{
    int res;
    int err = -ENODEV;

    pr_info("%s ver. %s\n", DRV_NAME, DRV_VERSION);

    // init global structure
    memset(&ipnoise, 0x00, sizeof(ipnoise));

    // init uplink, downlink queues
    INIT_LIST_HEAD(&ipnoise.uplink.list);
    INIT_LIST_HEAD(&ipnoise.downlink.list);

    // register pci driver
    res = pci_register_driver(&ipnoise_driver);
    if (res){
        pr_err("cannot register pci driver\n");
        goto fail;
    }

    ipnoise.have_pci = 1;

    // register hostos tcp proto
    err = proto_register(&hostos_tcp_prot, 1);
    if (err){
        pr_err("cannot register tcp proto\n");
        goto fail;
    }

    // register hostos udp proto
    err = proto_register(&hostos_udp_prot, 1);
    if (err){
        pr_err("cannot register udp proto\n");
        goto fail;
    }

    // register hostos sock
    (void)sock_register(&hostos_family_ops);

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

