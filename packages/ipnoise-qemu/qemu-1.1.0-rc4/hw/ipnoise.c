/*
 * QEMU IPNoise support
 *
 * Copyright (c) 2012 Roman E. Chechnev
 *
 */

#define pr_fmt(fmt) "qemu: " fmt

#include <errno.h>
#include "ipnoise.h"

static const MemoryRegionOps ipnoise_io_ops = {
    .read       = ipnoise_ioport_read,
    .write      = ipnoise_ioport_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static const VMStateDescription vmstate_pci_ipnoise = {
    .name                       = "ipnoise",
    .version_id                 = 3,
    .minimum_version_id         = 2,
    .minimum_version_id_old     = 2,
    .fields                     = (VMStateField [])
    {
        VMSTATE_PCI_DEVICE(pci_dev, PCIIPNoiseState),
        VMSTATE_STRUCT(
            state, PCIIPNoiseState, 0, vmstate_ipnoise, IPNoiseState
        ),
        VMSTATE_END_OF_LIST()
    }
};

static Property ipnoise_properties[] = {
    DEFINE_PROP_END_OF_LIST(),
};

static uint64_t ipnoise_ioport_read(
    void                *opaque,
    target_phys_addr_t  addr,
    unsigned            size)
{
    IPNoiseState    *s          = opaque;
    uint64_t        res         = 0;
    uint32_t        reg_addr    = addr & (IPNOISE_IOPORT_SIZE - 1);

    // pr_debug("ipnoise_ioport_read addr=0x%08x size: %u\n", addr, size);

    switch (reg_addr){
        case IPNOISE_REG_CR:
            res = s->reg_cr;
            break;
        case IPNOISE_REG_CMD_ID:
            res = s->reg_cmd_id;
            break;
        case IPNOISE_REG_CMD_0:
            res = s->reg_cmd_0;
            break;
        case IPNOISE_REG_CMD_1:
            res = s->reg_cmd_1;
            break;
        case IPNOISE_REG_CMD_2:
            res = s->reg_cmd_2;
            break;
        case IPNOISE_REG_CMD_3:
            res = s->reg_cmd_3;
            break;
        case IPNOISE_REG_CMD_4:
            res = s->reg_cmd_4;
            break;

        default:
            pr_err("ipnoise_ioport_read, unsupported register"
                " address: '%u'\n",
                reg_addr
            );
            break;
    }

    return res;
}

static void ipnoise_reset_cmd_regs(IPNoiseState *s)
{
    s->reg_cr       = 0;
    s->reg_cmd_id   = 0;
    s->reg_cmd_0    = 0;
    s->reg_cmd_1    = 0;
    s->reg_cmd_2    = 0;
    s->reg_cmd_3    = 0;
    s->reg_cmd_4    = 0;
}

static void ipnoise_ioport_write(
    void                *opaque,
    target_phys_addr_t  addr,
    uint64_t            data,
    unsigned            size)
{
    IPNoiseState *s     = opaque;
    uint32_t reg_addr   = addr & (IPNOISE_IOPORT_SIZE - 1);
    uint32_t val        = data;

    // pr_debug("ipnoise_ioport_write addr=0x%08x size: %u\n", addr, size);

    switch (reg_addr){
        case IPNOISE_REG_CR:
            // read only register
            goto read_only;
        case IPNOISE_REG_CMD_ID:
            s->reg_cmd_id = val;
            ipnoise_process_command(s);
            break;
        case IPNOISE_REG_CMD_0:
            s->reg_cmd_0 = val;
            break;
        case IPNOISE_REG_CMD_1:
            s->reg_cmd_1 = val;
            break;
        case IPNOISE_REG_CMD_2:
            s->reg_cmd_2 = val;
            break;
        case IPNOISE_REG_CMD_3:
            s->reg_cmd_3 = val;
            break;
        case IPNOISE_REG_CMD_4:
            s->reg_cmd_4 = val;
            break;
       default:
            pr_err("ipnoise_ioport_write, unsupported register"
                " address: '%u'\n",
                reg_addr
            );
            break;
    }

out:
    return;

read_only:
    pr_err("attempt to write in read only register, addr: '%d'\n",
        reg_addr);
    goto out;
}

static int pci_ipnoise_uninit(PCIDevice *dev)
{
    PCIIPNoiseState *d = DO_UPCAST(PCIIPNoiseState, pci_dev, dev);
    IPNoiseState    *s = &d->state;

    while (!list_empty(&s->fds.list)){
        // ok, list is not empty
        struct list_head    *first      = NULL;
        IPNoiseFdInfo       *fd_info    = NULL;
        first   = s->fds.list.next;
        fd_info = list_entry(first, IPNoiseFdInfo, list);

        ipnoise_delete_fd_info(s, fd_info);
    }

    memory_region_destroy(&d->state.mmio);
    memory_region_destroy(&d->io_bar);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}

static void ipnoise_cr_dma(IPNoiseState *s)
{
    // check what downlink DMA ready
    if (s->downlink_dma_addr && s->downlink_dma_size){
        ipnoise_set_cr_flags(s, IPNOISE_CR_DOWNLINK_DMA_READY);
    } else {
        ipnoise_clear_cr_flags(s, IPNOISE_CR_DOWNLINK_DMA_READY);
    }

    // check what uplink DMA ready
    if (s->uplink_dma_addr && s->uplink_dma_size){
        ipnoise_set_cr_flags(s, IPNOISE_CR_UPLINK_DMA_READY);
    } else {
        ipnoise_clear_cr_flags(s, IPNOISE_CR_UPLINK_DMA_READY);
    }
}

static void ipnoise_s_reset(IPNoiseState *s)
{
    pr_debug("ipnoise_s_reset\n");

    // reset control register
    ipnoise_cr(s, 0);

    // reset all cmd registers
    ipnoise_reset_cmd_regs(s);

    // update DMA flags
    ipnoise_cr_dma(s);

    // mark what self test was passed
    ipnoise_set_cr_flags(s, IPNOISE_CR_SELF_TEST_PASSED);
}

static void ipnoise_cr(
    IPNoiseState    *s,
    uint32_t        value)
{
    s->reg_cr = value;
}

static void ipnoise_set_cr_flags(
    IPNoiseState    *s,
    uint32_t        flags)
{
    s->reg_cr |= flags;
}

static void ipnoise_clear_cr_flags(
    IPNoiseState    *s,
    uint32_t        flags)
{
    s->reg_cr &= ~flags;
}

static void ipnoise_free_packet(
    IPNoisePacket *packet)
{
    if (!packet){
        goto out;
    }

    if (packet->data){
        g_free(packet->data);
        packet->data = NULL;
    }

    g_free(packet);
    packet = NULL;

out:
    return;
}

static IPNoisePacket * _ipnoise_create_packet(
    IPNoisePacket   *packet,
    int32_t         do_clone_data)
{
    IPNoisePacket *new_packet = NULL;

    // allocate new packet
    new_packet = g_malloc0(sizeof(*new_packet));
    if (!new_packet){
        pr_err("cannot allocate memory, was needed: '%d' byte(s)\n",
            sizeof(*new_packet));
        goto fail;
    }

    if (!packet){
        goto out;
    }

    // copy old packet
    memcpy(
        new_packet,
        packet,
        min(sizeof(*new_packet), sizeof(*packet))
    );

    if (    do_clone_data
        &&  new_packet->data_size > 0)
    {
        new_packet->data = g_malloc0(new_packet->data_size);
        if (!new_packet->data){
            pr_err("cannot allocate memory, was needed: '%d' byte(s)\n",
                new_packet->data_size);
            goto fail;
        }
        // copy data
        memcpy(new_packet->data, packet->data, new_packet->data_size);
    } else {
        new_packet->data        = NULL;
        new_packet->data_size   = 0;
    }

out:
    return new_packet;

fail:
    if (new_packet){
        ipnoise_free_packet(new_packet);
        new_packet = NULL;
    }
    goto out;
}

static IPNoisePacket * ipnoise_create_packet(
    IPNoisePacket *packet)
{
    IPNoisePacket *new_packet = NULL;

    new_packet = _ipnoise_create_packet(packet, 0);
    if (!new_packet){
        ipnoise_dump_packet("cannot create packet", packet);
        goto fail;
    }

out:
    return new_packet;

fail:
    goto out;
}

void ipnoise_clear_buff(IPNoiseBuff *buff)
{
    if (!buff){
        goto out;
    }

    // name
    if (buff->name){
        g_free(buff->name);
        buff->name = NULL;
    }
    buff->namelen = 0;

    // data
    if (buff->data){
        g_free(buff->data);
        buff->data = NULL;
    }
    buff->data_size = 0;

out:
    return;
}

void ipnoise_free_waiting_packet(IPNoiseFdInfo *fd_info)
{
    if (fd_info->waiting_packet){
        ipnoise_free_packet(fd_info->waiting_packet);
        fd_info->waiting_packet = NULL;
    }
}

void ipnoise_clear_fd_info(IPNoiseFdInfo *fd_info)
{
    ipnoise_free_waiting_packet(fd_info);
    ipnoise_clear_buff(&fd_info->send);
    ipnoise_clear_buff(&fd_info->recv);
}

void ipnoise_clear_poll_revents(
    IPNoiseFdInfo   *fd_info,
    uint32_t        events)
{
    fd_info->poll_revents &= ~events;
}

void ipnoise_set_poll_revents(
    IPNoiseFdInfo   *fd_info,
    uint32_t        events)
{
    fd_info->poll_revents |= events;
}

void ipnoise_clear_poll_wevents(
    IPNoiseFdInfo   *fd_info,
    uint32_t        events)
{
    fd_info->poll_wevents &= ~events;
}

void ipnoise_set_poll_wevents(
    IPNoiseFdInfo   *fd_info,
    uint32_t        events)
{
    fd_info->poll_wevents |= events;
}

static void ipnoise_set_waiting_packet(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *packet)
{
    if (fd_info->waiting_packet){
        pr_warn("fd_info->waiting_packet not empty\n");
        ipnoise_free_packet(fd_info->waiting_packet);
        fd_info->waiting_packet = NULL;
    }

    // copy request packet
    fd_info->waiting_packet = ipnoise_create_packet(packet);
}

void ipnoise_wait(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet)
{
    uint32_t poll_wevents = fd_info->poll_wevents;

    if (waiting_packet){
        // set waiting packet
        ipnoise_set_waiting_packet(fd_info, waiting_packet);
    }

    qemu_set_fd_handler2(
        fd_info->fd,
        NULL,
        (poll_wevents & IPNOISE_POLLIN)  ? ipnoise_read_cb  : NULL,
        (poll_wevents & IPNOISE_POLLOUT) ? ipnoise_write_cb : NULL,
        fd_info
    );
}

void ipnoise_wait_rd_wr(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet)
{
    fd_info->poll_wevents |= (IPNOISE_POLLIN | IPNOISE_POLLOUT);
    ipnoise_wait(fd_info, waiting_packet);
}

void ipnoise_call_rd(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet)
{
    ipnoise_set_waiting_packet(fd_info, waiting_packet);
    ipnoise_read_cb(fd_info);
}

void ipnoise_call_wr(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet)
{
    ipnoise_set_waiting_packet(fd_info, waiting_packet);
    ipnoise_write_cb(fd_info);
}

void ipnoise_wait_rd(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet)
{
    fd_info->poll_wevents |= (IPNOISE_POLLIN);
    ipnoise_wait(fd_info, waiting_packet);
}

void ipnoise_wait_wr(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet)
{
    fd_info->poll_wevents |= (IPNOISE_POLLOUT);
    ipnoise_wait(fd_info, waiting_packet);
}

void ipnoise_no_wait_rd_wr(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet)
{
    uint32_t new = IPNOISE_POLLIN | IPNOISE_POLLOUT;
    fd_info->poll_wevents &= ~new;
    ipnoise_wait(fd_info, waiting_packet);
}

void ipnoise_no_wait_rd(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet)
{
    uint32_t new = IPNOISE_POLLIN;
    fd_info->poll_wevents &= ~new;
    ipnoise_wait(fd_info, waiting_packet);
}

void ipnoise_no_wait_wr(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet)
{
    uint32_t new = IPNOISE_POLLOUT;
    fd_info->poll_wevents &= ~new;
    ipnoise_wait(fd_info, waiting_packet);
}

static void ipnoise_flush_downlink(IPNoiseState *s)
{
    unsigned char *buffer       = NULL;
    unsigned char *buffer_ptr   = NULL;

    // attempt to flush downlink queue
    if (s->reg_cr & IPNOISE_CR_DLDD){
        // downlink already dirty
        goto out;
    }

    if (s->irq_level){
        goto out;
    }

    if (list_empty(&s->downlink.list)){
        goto out;
    }

    // allocate buffer
    buffer = g_malloc0(s->downlink_dma_size);
    if (!buffer){
        pr_err("cannot allocate memory, was needed: '%d' byte(s)\n",
            s->downlink_dma_size);
        goto fail;
    }

    buffer_ptr = buffer;

    // ok, now we can flush downlink DMA
    while (!list_empty(&s->downlink.list)){
        struct list_head    *first          = NULL;
        IPNoisePacket       *packet         = NULL;
        uint32_t            packet_size     = 0;
        unsigned char       *packet_data    = NULL;

        // ok, list is not empty
        first       = s->downlink.list.next;
        packet      = list_entry(first, IPNoisePacket, list);
        packet_size = sizeof(*packet) + packet->data_size;

        // check what we have DMA space for this packet
        if ((buffer_ptr + packet_size) > (buffer + s->downlink_dma_size)){
            // there are no space for this packet
            pr_warn("not enough downlink DMA size:\n"
                " buffer:               0x'%x'\n"
                " buffer_ptr:           0x'%x'\n"
                " packet_size:          '%d'\n"
                " s->downlink_dma_size: '%d'\n",
                (unsigned int)buffer,
                (unsigned int)buffer_ptr,
                packet_size,
                s->downlink_dma_size
            );
            break;
        }

        // remove from queue
        list_del(first);

        // store packet data pointer and remove it from packet
        packet_data     = packet->data;
        packet->data    = NULL;

        // copy packet header to downlink DMA
        memcpy(buffer_ptr, packet, sizeof(*packet));
        buffer_ptr += sizeof(*packet);

        if (packet_data){
            // copy packet data to downlink DMA
            if (packet->data_size > 0){
                memcpy(buffer_ptr, packet_data, packet->data_size);
                buffer_ptr += packet->data_size;
            }
        }

#ifdef IPNOISE_DEBUG
        ipnoise_dump_packet("ipnoise flush downlink", packet);
#endif

        // free packet
        ipnoise_free_packet(packet);

        // mark downlink DMA dirty
        ipnoise_set_cr_flags(s, IPNOISE_CR_DLDD);
    }

    // write DMA
    pci_dma_write(
        s->pci_dev,
        s->downlink_dma_addr,
        (void *)buffer,
        s->downlink_dma_size
    );

    // free buffer
    g_free(buffer);

out:
    if (s->reg_cr & IPNOISE_CR_DLDD){
        if (!s->irq_level){
            // downlink data dirty, request IRQ
            s->irq_level = 1;
            qemu_set_irq(s->irq, s->irq_level);
        }
    }

    return;

fail:
    goto out;
}

static void ipnoise_flush(IPNoiseState *s)
{
    ipnoise_flush_uplink(s);
    ipnoise_flush_downlink(s);
}

static void ipnoise_add_downlink(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *packet)
{
    IPNoiseState *s = (IPNoiseState *)fd_info->s;
    packet->os_private      = fd_info->os_private;
    packet->poll_revents    = fd_info->poll_revents;
    list_add_tail(&packet->list, &s->downlink.list);
}

static int32_t ipnoise_normalize_error(int32_t err)
{
    int negative = 0;

    if (err < 0){
        negative    = 1;
        err         = -err;
    }

#ifdef _WIN32
    switch (err){
        case 0:
            break;
        case WSAEWOULDBLOCK:
            err = EWOULDBLOCK;
            break;
        case WSAEINPROGRESS:
            err = EINPROGRESS;
            break;
        case WSAEALREADY:
            err = EALREADY;
            break;
        case WSAENOTSOCK:
            err = ENOTSOCK;
            break;
        case WSAEDESTADDRREQ:
            err = EDESTADDRREQ;
            break;
        case WSAEMSGSIZE:
            err = EMSGSIZE;
            break;
        case WSAEPROTOTYPE:
            err = EPROTOTYPE;
            break;
        case WSAENOPROTOOPT:
            err = ENOPROTOOPT;
            break;
        case WSAEPROTONOSUPPORT:
            err = EPROTONOSUPPORT;
            break;
//        case WSAESOCKTNOSUPPORT:
//            err = ESOCKTNOSUPPORT;
//            break;
        case WSAEOPNOTSUPP:
            err = EOPNOTSUPP;
            break;
//        case WSAEPFNOSUPPORT:
//            err = EPFNOSUPPORT;
//            break;
        case WSAEAFNOSUPPORT:
            err = EAFNOSUPPORT;
            break;
        case WSAEADDRINUSE:
            err = EADDRINUSE;
            break;
        case WSAEADDRNOTAVAIL:
            err = EADDRNOTAVAIL;
            break;
        case WSAENETDOWN:
            err = ENETDOWN;
            break;
        case WSAENETUNREACH:
            err = ENETUNREACH;
            break;
        case WSAENETRESET:
            err = ENETRESET;
            break;
        case WSAECONNABORTED:
            err = ECONNABORTED;
            break;
        case WSAECONNRESET:
            err = ECONNRESET;
            break;
        case WSAENOBUFS:
            err = ENOBUFS;
            break;
        case WSAEISCONN:
            err = EISCONN;
            break;
        case WSAENOTCONN:
            err = ENOTCONN;
            break;
//        case WSAESHUTDOWN:
//            err = ESHUTDOWN;
//            break;
//        case WSAETOOMANYREFS:
//            err = TOOMANYREFS;
//            break;
        case WSAETIMEDOUT:
            err = ETIMEDOUT;
            break;
        case WSAECONNREFUSED:
            err = ECONNREFUSED;
            break;
        case WSAELOOP:
            err = ELOOP;
            break;
        case WSAENAMETOOLONG:
            err = ENAMETOOLONG;
            break;
//        case WSAEHOSTDOWN:
//            err = EHOSTDOWN;
//            break;
        case WSAEHOSTUNREACH:
            err = EHOSTUNREACH;
            break;
        case WSAENOTEMPTY:
            err = ENOTEMPTY;
            break;
//        case WSAEPROCLIM:
//            err = EPROCLIM;
//            break;
//        case WSAEUSERS:
//            err = EUSERS;
//            break;
//        case WSAEDQUOT:
//            err = EDQUOT;
//            break;
//        case WSAESTALE:
//            err = ESTALE;
//            break;
//        case WSAEREMOTE:
//            err = EREMOTE;
//            break;
        default:
            pr_err("Cannot translate win32 error: '%d'\n", err);
            break;
    }
#endif

    if (negative){
        err = -err;
    }

    return err;
}

static int32_t ipnoise_socket_error(int32_t res)
{
    int32_t err = 0;
    int32_t ret = 0;

    err = socket_error();
    err = ipnoise_normalize_error(err);

    ret = (res < 0) ? -err : res;
    pr_debug("ipnoise_socket_error ret: '%d', err: '%d'\n", ret, err);
    return ret;
}

static int32_t ipnoise_lsocket_error(int32_t fd)
{
    // get last socket error
    int32_t     res, err           = 0;
    int32_t     last_sock_err      = 0;
    socklen_t   last_sock_err_size = sizeof(last_sock_err);

    res = getsockopt(
        fd,
        SOL_SOCKET,
        SO_ERROR,
        (void *)&last_sock_err,
        &last_sock_err_size
    );

    if (!res){
        err = ipnoise_normalize_error(last_sock_err);
    }

    pr_debug("ipnoise_lsocket_error, fd: '%d', err: '%d'\n",
        fd, err);
    return err;
}

static void ipnoise_read_cb(void *opaque)
{
    IPNoiseFdInfo   *fd_info        = (IPNoiseFdInfo *)opaque;
    IPNoiseState    *s              = (IPNoiseState *)fd_info->s;
    IPNoisePacket   *new_packet     = NULL;
    IPNoisePacket   *waiting_packet = fd_info->waiting_packet;

    int32_t res = 0;
    int32_t fd  = fd_info->fd;

    pr_debug("ipnoise_read_cb fd: '%d', waiting_packet: 0x'%x'\n",
        fd,
        (unsigned int)waiting_packet
    );

    // add information about poll event
    ipnoise_set_poll_revents(fd_info, IPNOISE_POLLIN);

    // allocate new packet
    new_packet = ipnoise_create_packet(waiting_packet);
    if (!new_packet){
        ipnoise_dump_packet(
            "cannot allocate new packet from: ",
            fd_info->waiting_packet
        );
        goto fail;
    }

    // get last socket error
    new_packet->last_sock_err = ipnoise_lsocket_error(fd);

    // get wait packet
    waiting_packet = fd_info->waiting_packet;

    if (!waiting_packet){
        // there are no waiting packet
        new_packet->type       = IPNOISE_PACKET_POLL;
        new_packet->un.poll.fd = fd;

        // add answer in downlink queue
        ipnoise_add_downlink(fd_info, new_packet);
        goto out;
    }

    pr_debug("ipnoise_read_cb fd: '%d',"
        " fd_info->waiting_packet_type: 0x'%x'\n",
        fd,
        (unsigned int)fd_info->waiting_packet->type
    );

    switch (waiting_packet->type){
        case IPNOISE_PACKET_ACCEPT:
            // mark event as handled
            ipnoise_clear_poll_revents(fd_info, IPNOISE_POLLIN);
            {
                struct sockaddr addr;
                socklen_t       addrlen = sizeof(addr);
                do {
                    res = qemu_accept(fd, &addr, &addrlen);
                    res = ipnoise_socket_error(res);
                } while (res < 0 && EINTR == res);

                pr_debug("after accept, fd: '%d', res: '%d'\n",
                    fd, res);

                // store result
                new_packet->last_sock_err = ipnoise_lsocket_error(fd);
                new_packet->un.accept.res       = res;
                new_packet->un.accept.addrlen   = min(
                    sizeof(new_packet->un.accept.addr),
                    (uint32_t)addrlen
                );
                memcpy(
                    &new_packet->un.accept.addr,
                    &addr,
                    new_packet->un.accept.addrlen
                );

                // add answer in downlink queue
                ipnoise_add_downlink(fd_info, new_packet);
            }
            break;

        case IPNOISE_PACKET_CONNECT:
            // mark event as handled
            ipnoise_clear_poll_revents(fd_info, IPNOISE_POLLIN);
            {
                do {
                    int32_t tmp;
                    res = qemu_recv(
                        fd,
                        &tmp,
                        0,              // read size
                        0               // flags
                    );
                    res = ipnoise_socket_error(res);
                } while (res < 0 && EINTR == res);

                new_packet->last_sock_err = ipnoise_lsocket_error(fd);

                // add answer in downlink queue
                ipnoise_add_downlink(fd_info, new_packet);
            }
            break;

        case IPNOISE_PACKET_RECVMSG:
            // mark event as handled
            ipnoise_clear_poll_revents(fd_info, IPNOISE_POLLIN);
            {
                int32_t         flags       = 0;
                int32_t         *addrlen    = NULL;
                struct sockaddr *addr       = NULL;

                addr = &new_packet->un.recvmsg.un.msg_end.addr;
                addrlen  = &new_packet->un.recvmsg.un.msg_end.addrlen;
                *addrlen = sizeof(*addr);

                do {
                    res = recvfrom(
                        fd,
                        fd_info->recv.data,
                        fd_info->recv.data_size,
                        flags,
                        addr,
                        (socklen_t *)addrlen
                    );
                    res = ipnoise_socket_error(res);
                } while (res < 0 && EINTR == res);

                pr_debug("after recv fd: '%d',"
                    " res: '%d', addrlen: '%d'\n",
                    fd_info->fd,
                    res,
                    new_packet->un.recvmsg.un.msg_end.addrlen
                );

                // setup end packet
                {
                    new_packet->last_sock_err =
                        ipnoise_lsocket_error(fd);
                    new_packet->un.recvmsg.un.msg_end.res = res;
                    if (res > 0){
                        new_packet->data_size = res;
                    }
                    new_packet->data = fd_info->recv.data;
                    // fd_info->recv.data will be free after packet
                    // will be processed,
                    // so mark it as free in fd_info
                    fd_info->recv.data          = NULL;
                    fd_info->recv.data_size     = 0;

                    // add answer in downlink queue
                    ipnoise_add_downlink(fd_info, new_packet);
                }
            }
            break;

        default:
            pr_err("unsupported waiting_packet type: 0x'%x'\n",
                waiting_packet->type);
            break;
    }

out:
    // flush answers
    ipnoise_flush_downlink(s);

    if (fd_info->poll_revents & IPNOISE_POLLIN){
        // we have pending event, so disable read callback
        ipnoise_no_wait_rd(fd_info, NULL);
    } else {
        ipnoise_wait_rd(fd_info, NULL);
    }

    // clear fd_info
    ipnoise_clear_fd_info(fd_info);

    return;

fail:
    if (new_packet){
        ipnoise_free_packet(new_packet);
        new_packet = NULL;
    }
    goto out;
}

static void ipnoise_write_cb(void *opaque)
{
    IPNoiseFdInfo   *fd_info        = (IPNoiseFdInfo *)opaque;
    IPNoiseState    *s              = (IPNoiseState *)fd_info->s;
    IPNoisePacket   *new_packet     = NULL;
    IPNoisePacket   *waiting_packet = fd_info->waiting_packet;

    int32_t res = 0;
    int32_t fd  = fd_info->fd;

    // disable write callback
    ipnoise_no_wait_wr(fd_info, NULL);

    pr_debug("ipnoise_write_cb, fd: '%d',"
        " fd_info->waiting_packet: 0x'%x'\n",
        fd,
        (unsigned int)fd_info->waiting_packet
    );

    // add information about poll event
    ipnoise_set_poll_revents(fd_info, IPNOISE_POLLOUT);

    // allocate new packet
    new_packet = ipnoise_create_packet(waiting_packet);
    if (!new_packet){
        ipnoise_dump_packet(
            "cannot allocate new packet from: ",
            fd_info->waiting_packet
        );
        goto fail;
    }

    // get last socket error
    new_packet->last_sock_err = ipnoise_lsocket_error(fd);

    if (!waiting_packet){
        // there are no waiting packet
        new_packet->type       = IPNOISE_PACKET_POLL;
        new_packet->un.poll.fd = fd;

        // add answer in downlink queue
        ipnoise_add_downlink(fd_info, new_packet);
        goto out;
    }

    switch (waiting_packet->type){
        case IPNOISE_PACKET_CONNECT:
            new_packet->last_sock_err = 0;
            // add answer in downlink queue
            ipnoise_add_downlink(fd_info, new_packet);
            break;

        case IPNOISE_PACKET_SENDMSG:
            {
                int32_t flags = 0;

                do {
                    if (fd_info->send.namelen > 0){
                        res = sendto(
                            fd,
                            (const void *)fd_info->send.data,
                            (int32_t)fd_info->send.data_size,
                            flags,
                            (struct sockaddr *)fd_info->send.name,
                            (socklen_t)fd_info->send.namelen
                        );
                    } else {
                        res = send(
                            fd,
                            (const void *)fd_info->send.data,
                            (int32_t)fd_info->send.data_size,
                            flags
                        );
                    }
                    res = ipnoise_socket_error(res);
                } while (res < 0 && EINTR == res);

                pr_debug("send to fd: '%d',"
                    " '%u' byte(s),"
                    " flags: '%d',"
                    " res: '%d'\n",
                    fd,
                    fd_info->send.data_size,
                    flags,
                    res
                );

                // clear send buffer
                ipnoise_clear_buff(&fd_info->send);

                // add result
                new_packet->last_sock_err = ipnoise_lsocket_error(fd);
                new_packet->un.sendmsg.un.msg_end.res = res;

                // add answer in downlink queue
                ipnoise_add_downlink(fd_info, new_packet);
            }
            break;

        default:
            pr_err("unsupported waiting_packet type: 0x'%x'\n",
                waiting_packet->type);
            break;
    }

out:
    // flush answers
    ipnoise_flush_downlink(s);

    // clear fd_info
    ipnoise_clear_fd_info(fd_info);
    return;

fail:
    if (new_packet){
        ipnoise_free_packet(new_packet);
        new_packet = NULL;
    }
    goto out;
}

static IPNoiseFdInfo * ipnoise_get_fd_info(
    IPNoiseState    *s,
    int32_t         fd)
{
    IPNoiseFdInfo       *ret    = NULL;
    struct list_head    *cur    = NULL;
    struct list_head    *tmp    = NULL;

    list_for_each_safe(cur, tmp, &s->fds.list){
        IPNoiseFdInfo *fd_info = NULL;
        fd_info = list_entry(cur, IPNoiseFdInfo, list);

        if (fd_info->fd == fd){
            ret = fd_info;
            break;
        }
    }

    return ret;
}

static void ipnoise_delete_fd_info(
    IPNoiseState    *s,
    IPNoiseFdInfo   *fd_info)
{
    if (fd_info){
        list_del(&fd_info->list);
        ipnoise_clear_fd_info(fd_info);
        g_free(fd_info);
    }
}

static IPNoiseFdInfo * _ipnoise_get_create_fd_info(
    IPNoiseState    *s,
    int32_t         fd,
    int32_t         do_clear)
{

    IPNoiseFdInfo *fd_info = NULL;

    fd_info = ipnoise_get_fd_info(s, fd);
    if (!fd_info){
        fd_info = g_malloc0(sizeof(*fd_info));
        list_add_tail(&fd_info->list, &s->fds.list);
    } else {
        pr_warn("_ipnoise_get_create_fd_info, fd: '%d' already exist,"
            " do_clear: %d\n",
            fd,
            do_clear
        );
    }

    if (do_clear){
        struct list_head list;
        // backup list head
        memcpy(&list, &fd_info->list, sizeof(list));
        // clear
        memset(fd_info, 0x00, sizeof(*fd_info));
        // restore list head
        memcpy(&fd_info->list, &list, sizeof(fd_info->list));
    }

    fd_info->fd = fd;
    fd_info->s  = s;

    return fd_info;
}

static IPNoiseFdInfo * ipnoise_create_fd_info(
    IPNoiseState    *s,
    int32_t         fd)
{
    return _ipnoise_get_create_fd_info(s, fd, 1);
}

// ---------------------- cmd handlers ----------------------

static void ipnoise_hostos_open_socket(
    IPNoiseState    *s,
    IPNoisePacket   *packet)
{
    int32_t     fd          = packet->un.open_socket.fd;
    uint32_t    pf_family   = packet->un.open_socket.pf_family;
    uint32_t    type        = packet->un.open_socket.type;
    uint32_t    protocol    = packet->un.open_socket.protocol;
    void        *os_private = packet->os_private;

    IPNoisePacket   *new_packet = NULL;
    IPNoiseFdInfo   *fd_info    = NULL;

    // allocate new packet
    new_packet = ipnoise_create_packet(packet);
    if (!new_packet){
        ipnoise_dump_packet("cannot allocate new packet from: ",
            packet);
        goto fail;
    }

    if (packet->un.open_socket.fd < 0){
        // if fd not exist - create new
        fd = qemu_socket(pf_family, type, protocol);
    }

    new_packet->last_sock_err = ipnoise_lsocket_error(fd);
    new_packet->un.open_socket.fd = fd;

    if (fd >= 0){
        // create fd info
        fd_info = ipnoise_create_fd_info(s, fd);
        fd_info->os_private = os_private;
        socket_set_nonblock(fd);

        // add callbacks
        ipnoise_wait_rd_wr(fd_info, NULL);
    }

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

out:
    ipnoise_flush_downlink(s);
    return;

fail:
    goto out;
}

static void ipnoise_hostos_close(
    IPNoiseState    *s,
    IPNoisePacket   *packet)
{
    int32_t         res, fd         = packet->un.close.fd;
    IPNoisePacket   *new_packet     = NULL;
    IPNoiseFdInfo   *fd_info        = NULL;

    if (fd < 0){
        // TODO XXX report error here (to downlink)
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(s, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        goto fail;
    }

    // remove callbacks
    ipnoise_no_wait_rd_wr(fd_info, NULL);

    // allocate new packet
    new_packet = ipnoise_create_packet(packet);
    if (!new_packet){
        ipnoise_dump_packet("cannot allocate new packet from: ",
            packet);
        goto fail;
    }

    do {
        // close socket
        res = closesocket(fd);
        res = ipnoise_socket_error(res);
    } while (res < 0 && EINTR == res);

    pr_debug("close fd: '%d', res: '%d'\n", fd, res);

    // clear poll events
    fd_info->poll_revents = 0;

    new_packet->last_sock_err = ipnoise_lsocket_error(fd);

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

out:
    ipnoise_flush_downlink(s);

    // delete fd info
    ipnoise_delete_fd_info(s, fd_info);

    return;

fail:
    goto out;
}

static void ipnoise_hostos_sendmsg(
    IPNoiseState    *s,
    IPNoisePacket   *packet)
{
    int32_t         fd          = packet->un.sendmsg.fd;
    IPNoiseFdInfo   *fd_info    = NULL;
    uint32_t        type        = packet->un.sendmsg.type;

    if (fd < 0){
        // TODO XXX report error here (to downlink)
        pr_err("ipnoise_hostos_sendmsg invalid fd: '%d'\n", fd);
        goto fail;
    }

    // search fd info
    fd_info = ipnoise_get_fd_info(s, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        pr_err("attempt to sendmsg in closed fd: '%d'\n", fd);
        goto fail;
    }

    switch (type){
        case IPNOISE_MSG_TYPE_NAME:
            // msg name
            if (packet->data_size > 0){
                fd_info->send.namelen = packet->data_size;
                if (fd_info->send.name){
                    pr_err("fd_info->send.name not free\n");
                    g_free(fd_info->send.name);
                    fd_info->send.name = NULL;
                }
                fd_info->send.name = g_malloc0(
                    fd_info->send.namelen
                );
                if (!fd_info->send.name){
                    pr_err("cannot allocate memory,"
                        " was needed: '%d' byte(s)\n",
                        fd_info->send.namelen
                    );
                    goto fail;
                }
                memcpy(
                    fd_info->send.name,
                    packet->data,
                    fd_info->send.namelen
                );
            }
            break;
        case IPNOISE_MSG_TYPE_IOV:
            // msg content
            {
                fd_info->send.data_size = packet->data_size;
                if (!fd_info->send.data_size){
                    break;
                }
                if (fd_info->send.data){
                    pr_err("fd_info->send.data not free\n");
                    g_free(fd_info->send.data);
                    fd_info->send.data = NULL;
                }
                fd_info->send.data = g_malloc0(
                    fd_info->send.data_size
                );
                if (!fd_info->send.data){
                    pr_err("cannot allocate memory,"
                        " was needed: '%d' byte(s)\n",
                        fd_info->send.data_size
                    );
                    goto fail;
                }
                // copy data
                memcpy(
                    fd_info->send.data,
                    packet->data,
                    fd_info->send.data_size
                );
            }
            break;

        case IPNOISE_MSG_TYPE_END:
            // msg end
            if (packet->non_block){
                // call write immediately
                ipnoise_call_wr(fd_info, packet);
            } else {
                // waiting write events (write)
                ipnoise_wait_wr(fd_info, packet);
            }
            break;

        default:
            pr_err("unknown sendmsg packet type: '%d'\n", type);
            break;
    }

out:
    return;
fail:
    goto out;
}

static void ipnoise_hostos_recvmsg(
    IPNoiseState    *s,
    IPNoisePacket   *packet)
{
    int32_t         fd          = packet->un.recvmsg.fd;
    IPNoiseFdInfo   *fd_info    = NULL;
    uint32_t        type        = packet->un.recvmsg.type;

    if (fd < 0){
        // TODO XXX report error here (to downlink)
        goto fail;
    }

    // search fd info
    fd_info = ipnoise_get_fd_info(s, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        pr_err("attempt to recvmsg from closed fd: '%d'\n", fd);
        goto fail;
    }

    switch (type){
        case IPNOISE_MSG_TYPE_NAME:
            // msg name
            if (packet->data_size > 0){
                fd_info->recv.namelen = packet->data_size;
                if (fd_info->recv.name){
                    pr_err("fd_info->recv.name not free\n");
                    g_free(fd_info->recv.name);
                    fd_info->recv.name = NULL;
                }
                fd_info->recv.name = g_malloc0(
                    fd_info->recv.namelen
                );
                if (!fd_info->recv.name){
                    pr_err("cannot allocate memory,"
                        " was needed: '%d' byte(s)\n",
                        fd_info->recv.namelen
                    );
                    goto fail;
                }
                memcpy(
                    fd_info->recv.name,
                    packet->data,
                    fd_info->recv.namelen
                );
            }
            break;

        case IPNOISE_MSG_TYPE_IOV:
            // msg iov
            {
                int32_t max_downlink_dma_data_size = 0;

                // calculate max downlink dma data size
                max_downlink_dma_data_size = s->downlink_dma_size
                    - 10 * sizeof(*packet);

                fd_info->recv.data_size = min(
                    max_downlink_dma_data_size,
                    packet->un.recvmsg.un.msg_iov.iov_len
                );

                if (fd_info->recv.data){
                    pr_err("fd_info->recv.data not free\n");
                    g_free(fd_info->recv.data);
                    fd_info->recv.data = NULL;
                }

                fd_info->recv.data = g_malloc0(
                    fd_info->recv.data_size
                );
                if (!fd_info->recv.data){
                    pr_err("cannot allocate memory,"
                        " was needed: '%d' byte(s)\n",
                        fd_info->recv.data_size
                    );
                    goto fail;
                }
            }
            break;

        case IPNOISE_MSG_TYPE_END:
            if (packet->non_block){
                // call read immediately
                ipnoise_call_rd(fd_info, packet);
            } else {
                // waiting read events (read)
                ipnoise_wait_rd(fd_info, packet);
            }
            break;

        default:
            pr_err("unknown recvmsg packet type: '%d'\n", type);
            break;
    }

out:
    return;
fail:
    goto out;
}

static void ipnoise_hostos_listen(
    IPNoiseState    *s,
    IPNoisePacket   *packet)
{
    int32_t         res, fd     = packet->un.listen.fd;
    int32_t         backlog     = packet->un.listen.backlog;
    IPNoiseFdInfo   *fd_info    = NULL;
    IPNoisePacket   *new_packet = NULL;

    if (fd < 0){
        // TODO XXX report error here (to downlink)
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(s, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        pr_err("cannot get/allocate fd_info\n");
        goto fail;
    }

    do {
        res = listen(fd, backlog);
        res = ipnoise_socket_error(res);
    } while (res < 0 && EINTR == res);

    if (!res){
        ipnoise_wait_rd(fd_info, NULL);
    }

    // allocate new packet
    new_packet = ipnoise_create_packet(packet);
    if (!new_packet){
        ipnoise_dump_packet("cannot allocate new packet from: ",
            packet);
        goto fail;
    }

    // store result
    new_packet->last_sock_err = ipnoise_lsocket_error(fd);

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

out:
    ipnoise_flush_downlink(s);
    return;

fail:
    goto out;
}

static void ipnoise_hostos_bind(
    IPNoiseState    *s,
    IPNoisePacket   *packet)
{
    int32_t         res, fd     = packet->un.bind.fd;
    struct sockaddr *addr       = &packet->un.bind.addr;
    int32_t         addrlen     = packet->un.bind.addrlen;
    IPNoiseFdInfo   *fd_info    = NULL;
    IPNoisePacket   *new_packet = NULL;

    if (fd < 0){
        // TODO XXX report error here (to downlink)
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(s, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        pr_err("cannot get/allocate fd_info\n");
        goto fail;
    }

    do {
        // enable reuse
        int on = 1;
        res = setsockopt(
            fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            (char *)&on,
            sizeof(on)
        );
        res = ipnoise_socket_error(res);
    } while (res < 0 && EINTR == res);

    do {
        res = bind(fd, addr, addrlen);
        res = ipnoise_socket_error(res);
    } while (res < 0 && EINTR == res);

    // allocate new packet
    new_packet = ipnoise_create_packet(packet);
    if (!new_packet){
        ipnoise_dump_packet("cannot allocate new packet from: ",
            packet);
        goto fail;
    }

    // store result
    new_packet->last_sock_err = ipnoise_lsocket_error(fd);

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

out:
    ipnoise_flush_downlink(s);
    return;

fail:
    goto out;
}

static void ipnoise_hostos_accept(
    IPNoiseState    *s,
    IPNoisePacket   *packet)
{
    int32_t         fd          = packet->un.accept.fd;
    IPNoiseFdInfo   *fd_info    = NULL;

    if (fd < 0){
        // TODO XXX report error here (to downlink)
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(s, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        pr_err("cannot get/allocate fd_info\n");
        goto fail;
    }

    if (packet->non_block){
        // call read immediately
        ipnoise_call_rd(fd_info, packet);
    } else {
        // waiting read events (accept)
        ipnoise_wait_rd(fd_info, packet);
    }

out:
    return;

fail:
    goto out;
}

static void ipnoise_hostos_connect(
    IPNoiseState    *s,
    IPNoisePacket   *packet)
{
    int32_t                 res, fd     = packet->un.connect.fd;
    struct sockaddr         *daddr      = &packet->un.connect.daddr;
    int32_t                 daddrlen    = packet->un.connect.daddrlen;
    IPNoiseFdInfo           *fd_info    = NULL;
    static IPNoisePacket    *new_packet = NULL;

    if (fd < 0){
        // TODO XXX report error here (to downlink)
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(s, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        pr_err("cannot get/allocate fd_info\n");
        goto fail;
    }

    do {
        res = connect(fd, daddr, daddrlen);
        res = ipnoise_socket_error(res);
    } while (res < 0 && EINTR == res);

    if (res >= 0){
        // we are connected
    } else if (-EINPROGRESS == res || -EALREADY == res){
        // all ok
        if (!packet->non_block){
            // wait read or write events (connect result)
            ipnoise_wait_rd_wr(fd_info, packet);
            goto out;
        }
    } else {
        // we have error
    }

    // create new packet
    new_packet = ipnoise_create_packet(packet);
    if (!new_packet){
        pr_err("cannot allocate new packet\n");
        goto fail;
    }

    // get last socket error
    new_packet->last_sock_err = ipnoise_lsocket_error(fd);

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

    ipnoise_flush_downlink(s);

    // wait read or write events (for poll)
    ipnoise_wait_rd_wr(fd_info, NULL);

out:
    return;

fail:
    goto out;
}

static void ipnoise_process_packet(
    IPNoiseState    *s,
    IPNoisePacket   *packet)
{
#ifdef IPNOISE_DEBUG
    ipnoise_dump_packet("ipnoise_process_packet", packet);
#endif
    switch (packet->type){
        case IPNOISE_PACKET_FREE:
            ipnoise_dump_packet(
                "attempt to process empty uplink packet",
                packet
            );
            break;

        case IPNOISE_PACKET_OPEN_SOCKET:
            ipnoise_hostos_open_socket(s, packet);
            break;

        case IPNOISE_PACKET_CLOSE:
            ipnoise_hostos_close(s, packet);
            break;

        case IPNOISE_PACKET_CONNECT:
            ipnoise_hostos_connect(s, packet);
            break;

        case IPNOISE_PACKET_SENDMSG:
            ipnoise_hostos_sendmsg(s, packet);
            break;

        case IPNOISE_PACKET_RECVMSG:
            ipnoise_hostos_recvmsg(s, packet);
            break;

        case IPNOISE_PACKET_BIND:
            ipnoise_hostos_bind(s, packet);
            break;

        case IPNOISE_PACKET_LISTEN:
            ipnoise_hostos_listen(s, packet);
            break;

        case IPNOISE_PACKET_ACCEPT:
            ipnoise_hostos_accept(s, packet);
            break;

        default:
            pr_err("unsupported packet type: '%d'\n", packet->type);
            break;
    }

    return;
}

static void ipnoise_flush_uplink(IPNoiseState *s)
{
    if (s->reg_cr & IPNOISE_CR_ULDD){
        unsigned char *buffer       = NULL;
        unsigned char *buffer_ptr   = NULL;

        // allocate buffer
        buffer = g_malloc0(s->uplink_dma_size);
        if (!buffer){
            pr_err("cannot allocate memory, was needed: '%d' byte(s)\n",
                s->downlink_dma_size);
            goto fail;
        }
        buffer_ptr = buffer;

        // read uplink dma
        pci_dma_read(
            s->pci_dev,
            s->uplink_dma_addr,
            buffer,
            s->uplink_dma_size
        );

        while (buffer_ptr < (buffer + s->uplink_dma_size)){
            IPNoisePacket *packet = (IPNoisePacket *)buffer_ptr;

            if (IPNOISE_PACKET_FREE == packet->type){
                // no more packets available
                break;
            }

            // read data if exist
            if (packet->data_size > 0){
                packet->data = buffer_ptr + sizeof(*packet);
            } else {
                packet->data = NULL;
            }

            // process packet
            ipnoise_process_packet(s, packet);

            // seek to next packet
            buffer_ptr += sizeof(*packet) + packet->data_size;
        };

        // mark uplink us free
        ipnoise_clear_cr_flags(s, IPNOISE_CR_ULDD);

        // free buffer
        g_free(buffer);
    }

out:
    return;

fail:
    goto out;
}

static void ipnoise_process_command(IPNoiseState *s)
{
    uint32_t cmd_id = s->reg_cmd_id;

    switch (cmd_id){
        case IPNOISE_CMD_RESET:
            ipnoise_s_reset(s);
            break;

        case IPNOISE_CMD_SET_ULDD:
            ipnoise_set_cr_flags(s, IPNOISE_CR_ULDD);
            break;

        case IPNOISE_CMD_CLEAR_DLDD:
            ipnoise_clear_cr_flags(s, IPNOISE_CR_DLDD);
            break;

        case IPNOISE_CMD_SET_IRQ_LEVEL:
            if (s->irq_level != s->reg_cmd_0){
                s->irq_level = s->reg_cmd_0;
                qemu_set_irq(s->irq, s->irq_level);
                if (!s->irq_level){
                    ipnoise_flush(s);
                }
            }
            break;

        case IPNOISE_CMD_SET_DOWNLINK_DMA_ADDR:
            s->downlink_dma_addr = s->reg_cmd_0;
            // update DMA flags
            ipnoise_cr_dma(s);
            break;

        case IPNOISE_CMD_SET_DOWNLINK_DMA_SIZE:
            s->downlink_dma_size = s->reg_cmd_0;
            // update DMA flags
            ipnoise_cr_dma(s);
            break;

        case IPNOISE_CMD_SET_UPLINK_DMA_ADDR:
            s->uplink_dma_addr = s->reg_cmd_0;
            // update DMA flags
            ipnoise_cr_dma(s);
            break;

        case IPNOISE_CMD_SET_UPLINK_DMA_SIZE:
            s->uplink_dma_size = s->reg_cmd_0;
            // update DMA flags
            ipnoise_cr_dma(s);
            break;

        default:
            pr_err("ipnoise_process_command"
                ", unsupported command ID: '%d'\n",
                cmd_id
            );
            break;
    }

}

void ipnoise_h_reset(void *opaque)
{
    IPNoiseState *s = opaque;
    ipnoise_s_reset(s);
}

const VMStateDescription vmstate_ipnoise = {
    .name                   = "ipnoise",
    .version_id             = 3,
    .minimum_version_id     = 2,
    .minimum_version_id_old = 2,
    .fields                 = (VMStateField []) {
        VMSTATE_END_OF_LIST()
    }
};

static inline int64_t ipnoise_get_next_poll_time(int64_t time_ms)
{
    return qemu_get_clock_ns(vm_clock) + time_ms*1e6;
}

static void ipnoise_poll_timer(void *opaque)
{
    IPNoiseState *s = opaque;

    qemu_del_timer(s->poll_timer);

//    pr_debug("timer before call ipnoise_flush_downlink {\n");
//    ipnoise_flush_downlink(s);
//    pr_debug("timer after call ipnoise_flush_downlink }\n");

    qemu_mod_timer(s->poll_timer, ipnoise_get_next_poll_time(5000));
}

static int pci_ipnoise_init(PCIDevice *pci_dev)
{
    PCIIPNoiseState *d = DO_UPCAST(PCIIPNoiseState, pci_dev, pci_dev);
    IPNoiseState    *s = &d->state;
    uint8_t         *pci_conf = pci_dev->config;

#ifdef _WIN32
    WSADATA Data;
    WSAStartup(MAKEWORD(2,0), &Data);
#endif

    INIT_LIST_HEAD(&s->downlink.list);
    INIT_LIST_HEAD(&s->fds.list);

    pci_set_word(pci_conf + PCI_STATUS,
        PCI_STATUS_FAST_BACK | PCI_STATUS_DEVSEL_MEDIUM);

    pci_set_word(pci_conf + PCI_SUBSYSTEM_VENDOR_ID,    0x00);
    pci_set_word(pci_conf + PCI_SUBSYSTEM_ID,           0x00);

    pci_conf[PCI_INTERRUPT_PIN] = 1; // interrupt pin A
    pci_conf[PCI_MIN_GNT]       = 0x06;
    pci_conf[PCI_MAX_LAT]       = 0xff;

    memory_region_init_io(
        &d->io_bar, &ipnoise_io_ops, s, "ipnoise-io",
        IPNOISE_IOPORT_SIZE
    );

    pci_register_bar(
        pci_dev,
        0,
        PCI_BASE_ADDRESS_SPACE_IO,
        &d->io_bar
    );

    pci_register_bar(pci_dev, 1, 0, &s->mmio);

    s->irq     = pci_dev->irq[0];
    s->pci_dev = pci_dev;

    s->poll_timer = qemu_new_timer_ns(vm_clock, ipnoise_poll_timer, s);
    qemu_mod_timer(s->poll_timer, ipnoise_get_next_poll_time(5000));

    return 0;
}

static void pci_reset(DeviceState *dev)
{
    PCIIPNoiseState *d = DO_UPCAST(PCIIPNoiseState, pci_dev.qdev, dev);

    ipnoise_h_reset(&d->state);
}

static void ipnoise_class_init(ObjectClass *klass, void *data)
{
    DeviceClass     *dc = DEVICE_CLASS(klass);
    PCIDeviceClass  *k = PCI_DEVICE_CLASS(klass);

    k->init         = pci_ipnoise_init;
    k->exit         = pci_ipnoise_uninit;
    k->vendor_id    = PCI_VENDOR_ID_IPNOISE;
    k->device_id    = PCI_DEVICE_ID_HOSTOS;
    k->revision     = 0x00;
    k->class_id     = PCI_CLASS_SYSTEM_OTHER;
    dc->reset       = pci_reset;
    dc->vmsd        = &vmstate_pci_ipnoise;
    dc->props       = ipnoise_properties;
}

static TypeInfo ipnoise_info = {
    .name          = "ipnoise",
    .parent        = TYPE_PCI_DEVICE,
    .instance_size = sizeof(PCIIPNoiseState),
    .class_init    = ipnoise_class_init
};

static void pci_ipnoise_register_types(void)
{
    type_register_static(&ipnoise_info);
}

type_init(pci_ipnoise_register_types)

