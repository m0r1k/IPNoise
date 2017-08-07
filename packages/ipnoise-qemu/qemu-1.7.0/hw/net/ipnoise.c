/*
 * QEMU IPNoise support
 *
 * Copyright (c) 2012 Roman E. Chechnev
 *
 */

#define pr_fmt(fmt) "qemu: " fmt

#include <errno.h>
#include "ipnoise.h"

#define TYPE_IPNOISE    "ipnoise"

#define IPNOISE(obj) \
     OBJECT_CHECK(IPNoiseState, (obj), TYPE_IPNOISE)

static int32_t  g_debug_level         = 0;
static int32_t  g_memory_alloc_count  = 0;

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
    .fields                     = (VMStateField []){
        VMSTATE_END_OF_LIST()
    }
};

static Property ipnoise_properties[] = {
    DEFINE_PROP_END_OF_LIST(),
};

static void * ipnoise_malloc0(
    size_t a_size)
{
    void *ret = NULL;
    assert(a_size);
    ret = g_malloc0(a_size);
    g_memory_alloc_count++;
    pdebug(50, "request malloc:\n"
        "  size:                            '%d'\n"
        "  ptr:                             0x%x\n"
        "  g_memory_alloc_count (before):   '%d'\n"
        "  g_memory_alloc_count (after):    '%d'\n",
        a_size,
        (uint32_t)ret,
        (g_memory_alloc_count - 1),
        g_memory_alloc_count
    );
    return ret;
}

static void * ipnoise_realloc(
    void    *a_ptr,
    size_t  a_new_size)
{
    void *ret = NULL;
    if (!a_ptr){
        g_memory_alloc_count++;
    }
    ret = g_realloc(a_ptr, a_new_size);
    pdebug(50, "request re-alloc:\n"
        "  new_size:                        '%d'\n"
        "  ptr (old):                       0x%x\n"
        "  ptr (new):                       0x%x\n"
        "  g_memory_alloc_count (before):   '%d'\n"
        "  g_memory_alloc_count (after):    '%d'\n",
        a_new_size,
        (uint32_t)a_ptr,
        (uint32_t)ret,
        (g_memory_alloc_count - 1),
        g_memory_alloc_count
    );

    return ret;
}

static void ipnoise_free(
    void *a_ptr)
{
    assert(a_ptr);
    g_free(a_ptr);
    g_memory_alloc_count--;
    pdebug(50, "request free:\n"
        "  ptr:                             0x%x\n"
        "  g_memory_alloc_count (before):   '%d'\n"
        "  g_memory_alloc_count (after):    '%d'\n",
        (uint32_t)a_ptr,
        (g_memory_alloc_count + 1),
        g_memory_alloc_count
    );
}

static uint64_t ipnoise_ioport_read(
    void                *a_opaque,
    hwaddr              a_addr,
    unsigned            a_size)
{
    IPNoiseState *state   = (IPNoiseState *)a_opaque;
    uint64_t     res      = 0;
    uint32_t     reg_addr = a_addr & (IPNOISE_IOPORT_SIZE - 1);

    pdebug(100, "ipnoise_ioport_read"
        " addr=0x%08lx size: %u\n",
        (long unsigned int)a_addr,
        (uint32_t)a_size
    );

    switch (reg_addr){
        case IPNOISE_REG_CR:
            res = state->reg_cr;
            break;
        case IPNOISE_REG_CMD_ID:
            res = state->reg_cmd_id;
            break;
        case IPNOISE_REG_CMD_0:
            res = state->reg_cmd_0;
            break;
        case IPNOISE_REG_CMD_1:
            res = state->reg_cmd_1;
            break;
        case IPNOISE_REG_CMD_2:
            res = state->reg_cmd_2;
            break;
        case IPNOISE_REG_CMD_3:
            res = state->reg_cmd_3;
            break;
        case IPNOISE_REG_CMD_4:
            res = state->reg_cmd_4;
            break;

        default:
            perr("ipnoise_ioport_read, unsupported register"
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
    void        *a_opaque,
    hwaddr      a_addr,
    uint64_t    a_data,
    unsigned    a_size)
{
    IPNoiseState *state   = a_opaque;
    uint32_t     reg_addr = a_addr & (IPNOISE_IOPORT_SIZE - 1);
    uint32_t     val      = a_data;

    pdebug(100, "ipnoise_ioport_write addr=0x%04x size: %u\n",
       (unsigned int)a_addr,
       a_size
    );

    switch (reg_addr){
        case IPNOISE_REG_CR:
            pdebug(100, "reg_addr IPNOISE_REG_CR\n");
            // read only register
            goto read_only;
        case IPNOISE_REG_CMD_ID:
            pdebug(100, "reg_addr IPNOISE_REG_CMD_ID\n");
            state->reg_cmd_id = val;
            ipnoise_process_command(state);
            break;
        case IPNOISE_REG_CMD_0:
            pdebug(100, "reg_addr IPNOISE_REG_CMD_0\n");
            state->reg_cmd_0 = val;
            break;
        case IPNOISE_REG_CMD_1:
            pdebug(100, "reg_addr IPNOISE_REG_CMD_1\n");
            state->reg_cmd_1 = val;
            break;
        case IPNOISE_REG_CMD_2:
            pdebug(100, "reg_addr IPNOISE_REG_CMD_2\n");
            state->reg_cmd_2 = val;
            break;
        case IPNOISE_REG_CMD_3:
            pdebug(100, "reg_addr IPNOISE_REG_CMD_3\n");
            state->reg_cmd_3 = val;
            break;
        case IPNOISE_REG_CMD_4:
            pdebug(100, "reg_addr IPNOISE_REG_CMD_4\n");
            state->reg_cmd_4 = val;
            break;
       default:
            perr("ipnoise_ioport_write,"
                " unsupported register address: '%u'\n",
                reg_addr
            );
            break;
    }

out:
    return;

read_only:
    perr("attempt to write in read only register,"
        " addr: '%d'\n",
        reg_addr
    );
    goto out;
}

static void pci_ipnoise_uninit(
    PCIDevice *a_dev)
{
    IPNoiseState *state = IPNOISE(a_dev);

    timer_del(state->poll_timer);
    timer_free(state->poll_timer);

    _ipnoise_free_all_fds(state);

    memory_region_destroy(&state->mmio);
    memory_region_destroy(&state->mmio_bar);

#ifdef _WIN32
    WSACleanup();
#endif
}

static void ipnoise_cr_dma(
    IPNoiseState *a_state)
{
    IPNoiseState *state = a_state;

    // check what downlink DMA ready
    if (    state->dma_downlink_addr
        &&  state->dma_downlink_size)
    {
        ipnoise_set_cr_flags(
            state,
            IPNOISE_CR_DOWNLINK_DMA_READY
        );
    } else {
        ipnoise_clear_cr_flags(
            state,
            IPNOISE_CR_DOWNLINK_DMA_READY
        );
    }

    // check what uplink DMA ready
    if (    state->dma_uplink_addr
        &&  state->dma_uplink_size)
    {
        ipnoise_set_cr_flags(
            state,
            IPNOISE_CR_UPLINK_DMA_READY
        );
    } else {
        ipnoise_clear_cr_flags(
            state,
            IPNOISE_CR_UPLINK_DMA_READY
        );
    }
}

static void _ipnoise_free_all_fds(
    IPNoiseState *a_state)
{
    struct list_head    *first      = NULL;
    IPNoiseFdInfo       *fd_info    = NULL;
    IPNoiseState        *state      = a_state;

    while (!list_empty(&state->fds.list)){
        // ok, list is not empty
        first   = state->fds.list.next;
        fd_info = list_entry(first, IPNoiseFdInfo, list);

        // disable callbacks
        ipnoise_no_wait_rd_wr(fd_info, NULL);

        // close socket
        if (0 <= fd_info->fd){
            closesocket(fd_info->fd);
            fd_info->fd = -1;
        }

        // delete fd info
        ipnoise_delete_fd_info(state, fd_info);
    }

    INIT_LIST_HEAD(&state->fds.list);
}

/*
static void _ipnoise_free_uplink(
    IPNoiseState *a_state)
{
    struct list_head    *first  = NULL;
    IPNoisePacket       *packet = NULL;
    IPNoiseState        *state  = a_state;

    while (!list_empty(&state->uplink.list)){
        // ok, list is not empty
        first   = state->uplink.list.next;
        packet  = list_entry(first, IPNoisePacket, list);

        // remove from queue
        list_del(first);

        // free packet
        ipnoise_free_packet(packet);
    }
}
*/

static void _ipnoise_free_downlink(
    IPNoiseState *a_state)
{
    struct list_head    *first  = NULL;
    IPNoisePacket       *packet = NULL;
    IPNoiseState        *state  = a_state;

    while (!list_empty(&state->downlink.list)){
        // ok, list is not empty
        first   = state->downlink.list.next;
        packet  = list_entry(first, IPNoisePacket, list);

        // remove from queue
        list_del(first);

        // free packet
        ipnoise_free_packet(packet);
    }

    INIT_LIST_HEAD(&state->downlink.list);
}

static void _ipnoise_generic_reset(
    IPNoiseState *a_state)
{
    IPNoiseState *state = a_state;

    // clear test-passed flag
    ipnoise_clear_cr_flags(
        state,
        IPNOISE_CR_SELF_TEST_PASSED
    );

    // disable irq
    state->irq_level = 0;

    // del timer's events
    timer_del(state->poll_timer);

    {
        // disable events and close sockets
        _ipnoise_free_all_fds(state);
    }

    {
        // free downlink
        _ipnoise_free_downlink(state);

        // mark uplink as free
        ipnoise_clear_cr_flags(state, IPNOISE_CR_DLDD);
    }

    // reset control register
    // ipnoise_cr(state, 0);

    // reset all cmd registers
    ipnoise_reset_cmd_regs(state);

    {
        // free uplink
        // _ipnoise_free_uplink(state);

        // mark uplink us free
        ipnoise_clear_cr_flags(state, IPNOISE_CR_ULDD);
    }

    {
        // reset info about dma
        state->dma_downlink_addr  = 0;
        state->dma_downlink_size  = 0;
        state->dma_uplink_addr    = 0;
        state->dma_uplink_size    = 0;

        // update DMA flags
        ipnoise_cr_dma(state);
    }
}

static void ipnoise_s_reset(
    IPNoiseState *a_state)
{
    IPNoiseState    *state      = a_state;
    uint32_t        reg_cmd_0   = state->reg_cmd_0;

    pdebug(1, "ipnoise_s_reset\n");

    // do generic reset (it will clear registers too)
    _ipnoise_generic_reset(state);

    // check what size of IPNoisePacket structure is equal
    if (reg_cmd_0 == sizeof(IPNoisePacket)){
        // mark what self test was passed
        pdebug(1, "setup IPNOISE_CR_SELF_TEST_PASSED\n");
        ipnoise_set_cr_flags(
            state,
            IPNOISE_CR_SELF_TEST_PASSED
        );
    } else {
        perr("size of struct IPNoisePacket"
            " in qemu driver: '%d' is not equal"
            " size in OS driver: '%d'"
            " or driver forgot setup reg_cmd_0 while reset\n",
            sizeof(IPNoisePacket),
            reg_cmd_0
        );
        ipnoise_clear_cr_flags(
            state,
            IPNOISE_CR_SELF_TEST_PASSED
        );
    }
}

//static void ipnoise_cr(
//    IPNoiseState    *a_state,
//    uint32_t        a_value)
//{
//    IPNoiseState *state = a_state;
//    state->reg_cr = a_value;
//}

static void ipnoise_set_cr_flags(
    IPNoiseState    *a_state,
    uint32_t        a_flags)
{
    IPNoiseState *state = a_state;
    state->reg_cr |= a_flags;
}

static void ipnoise_clear_cr_flags(
    IPNoiseState    *a_state,
    uint32_t        a_flags)
{
    IPNoiseState *state = a_state;
    state->reg_cr &= ~a_flags;
}

static void ipnoise_free_packet(
    IPNoisePacket *a_packet)
{
    if (!a_packet){
        goto out;
    }

    if (a_packet->data){
        ipnoise_free(a_packet->data);
        a_packet->data = NULL;
    }

    ipnoise_free(a_packet);
    a_packet = NULL;

out:
    return;
}

static IPNoisePacket * _ipnoise_create_packet(
    IPNoisePacket   *a_packet,
    int32_t         a_do_clone_data)
{
    IPNoisePacket *new_packet = NULL;

    // allocate new packet
    new_packet = ipnoise_malloc0(sizeof(*new_packet));
    if (!new_packet){
        perr("cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            sizeof(*new_packet)
        );
        goto fail;
    }

    if (!a_packet){
        goto out;
    }

    // copy old packet
    memcpy(
        new_packet,
        a_packet,
        min(sizeof(*new_packet), sizeof(*a_packet))
    );

    if (    a_do_clone_data
        &&  new_packet->data_size > 0)
    {
        new_packet->data = ipnoise_malloc0(new_packet->data_size);
        if (!new_packet->data){
            perr("cannot allocate memory,"
                " was needed: '%d' byte(s)\n",
                new_packet->data_size
            );
            goto fail;
        }
        // copy data
        memcpy(
            new_packet->data,
            a_packet->data,
            new_packet->data_size
        );
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
    IPNoisePacket *a_packet)
{
    IPNoisePacket *new_packet = NULL;

    new_packet = _ipnoise_create_packet(a_packet, 0);
    if (!new_packet){
        ipnoise_dump_packet("cannot create packet", a_packet);
        goto fail;
    }

out:
    return new_packet;

fail:
    goto out;
}

static void ipnoise_clear_msg(
    struct msghdr   *a_msg)
{
    struct iovec    *iov    = NULL;
    int32_t         iov_id  = 0;

    // free message name
    if (a_msg->msg_name){
        ipnoise_free(a_msg->msg_name);
        a_msg->msg_name = NULL;
    }
    a_msg->msg_namelen = 0;

    // free iovs
    for (iov_id = 0; iov_id < a_msg->msg_iovlen; iov_id++){
        iov = &a_msg->msg_iov[iov_id];
        if (iov->iov_base){
            ipnoise_free(iov->iov_base);
            iov->iov_base = NULL;
        }
        iov->iov_len = 0;
    }
    // free iovs array
    if (a_msg->msg_iov){
        ipnoise_free(a_msg->msg_iov);
        a_msg->msg_iov = NULL;
    }
    a_msg->msg_iovlen = 0;

    // free message control
    if (a_msg->msg_control){
        ipnoise_free(a_msg->msg_control);
        a_msg->msg_control = NULL;
    }
    a_msg->msg_controllen = 0;

    // clear flags
    a_msg->msg_flags = 0;
}

static void ipnoise_clear_buff(
    IPNoiseBuff *a_buff)
{
    if (!a_buff){
        goto out;
    }

    // name
    if (a_buff->name){
        ipnoise_free(a_buff->name);
        a_buff->name = NULL;
    }
    a_buff->namelen = 0;

    // data
    if (a_buff->data){
        ipnoise_free(a_buff->data);
        a_buff->data = NULL;
    }
    a_buff->data_size = 0;

out:
    return;
}

void ipnoise_free_waiting_packet(
    IPNoiseFdInfo *a_fd_info)
{
    if (a_fd_info->waiting_packet){
        ipnoise_free_packet(a_fd_info->waiting_packet);
        a_fd_info->waiting_packet = NULL;
    }
}

void ipnoise_clear_fd_info(
    IPNoiseFdInfo *a_fd_info)
{
    ipnoise_free_waiting_packet(a_fd_info);

    ipnoise_clear_msg(&a_fd_info->send);
    // ipnoise_clear_buff(&a_fd_info->send);
    ipnoise_clear_buff(&a_fd_info->recv);
}

void ipnoise_clear_poll_revents(
    IPNoiseFdInfo   *a_fd_info,
    uint32_t        a_events)
{
    a_fd_info->poll_revents &= ~a_events;
}

void ipnoise_set_poll_revents(
    IPNoiseFdInfo   *a_fd_info,
    uint32_t        a_events)
{
    a_fd_info->poll_revents |= a_events;
}

void ipnoise_clear_poll_wevents(
    IPNoiseFdInfo   *a_fd_info,
    uint32_t        a_events)
{
    a_fd_info->poll_wevents &= ~a_events;
}

void ipnoise_set_poll_wevents(
    IPNoiseFdInfo   *a_fd_info,
    uint32_t        a_events)
{
    a_fd_info->poll_wevents |= a_events;
}

static void ipnoise_set_waiting_packet(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_packet)
{
    if (a_fd_info->waiting_packet){
        pwarn("a_fd_info->waiting_packet not empty\n");
        ipnoise_free_packet(a_fd_info->waiting_packet);
        a_fd_info->waiting_packet = NULL;
    }

    // copy request packet
    a_fd_info->waiting_packet = ipnoise_create_packet(
        a_packet
    );
}

void ipnoise_setup_waiting(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet)
{
    uint32_t poll_wevents = a_fd_info->poll_wevents;

    pdebug(10, "ipnoise_setup_waiting | %s | %s |\n",
        (poll_wevents & IPNOISE_POLLIN)  ? "IPNOISE_POLLIN"  : "",
        (poll_wevents & IPNOISE_POLLOUT) ? "IPNOISE_POLLOUT" : ""
    );

    if (a_waiting_packet){
        // set waiting packet
        ipnoise_set_waiting_packet(
            a_fd_info,
            a_waiting_packet
        );
    }

    qemu_set_fd_handler2(
        a_fd_info->fd,
        NULL,
        (poll_wevents & IPNOISE_POLLIN)  ? ipnoise_read_cb  : NULL,
        (poll_wevents & IPNOISE_POLLOUT) ? ipnoise_write_cb : NULL,
        a_fd_info
    );
}

void ipnoise_wait_rd_wr(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet)
{
    pdebug(50, "ipnoise_wait_rd_wr\n");
    a_fd_info->poll_wevents |= (IPNOISE_POLLIN | IPNOISE_POLLOUT);
    ipnoise_setup_waiting(a_fd_info, a_waiting_packet);
}

void ipnoise_call_rd(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet)
{
    pdebug(50, "ipnoise_call_rd\n");
    ipnoise_set_waiting_packet(a_fd_info, a_waiting_packet);
    ipnoise_read_cb(a_fd_info);
}

void ipnoise_call_wr(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet)
{
    pdebug(50, "ipnoise_call_wr\n");
    ipnoise_set_waiting_packet(a_fd_info, a_waiting_packet);
    ipnoise_write_cb(a_fd_info);
}

void ipnoise_wait_rd(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet)
{
    pdebug(50, "ipnoise_wait_rd\n");
    a_fd_info->poll_wevents |= (IPNOISE_POLLIN);
    ipnoise_setup_waiting(a_fd_info, a_waiting_packet);
}

void ipnoise_wait_wr(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet)
{
    pdebug(50, "ipnoise_wait_wr\n");
    a_fd_info->poll_wevents |= (IPNOISE_POLLOUT);
    ipnoise_setup_waiting(a_fd_info, a_waiting_packet);
}

void ipnoise_no_wait_rd_wr(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet)
{
    pdebug(50, "ipnoise_no_wait_rd_wr\n");
    uint32_t new = IPNOISE_POLLIN | IPNOISE_POLLOUT;
    a_fd_info->poll_wevents &= ~new;
    ipnoise_setup_waiting(a_fd_info, a_waiting_packet);
}

void ipnoise_no_wait_rd(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet)
{
    pdebug(50, "ipnoise_no_wait_rd\n");
    uint32_t new = IPNOISE_POLLIN;
    a_fd_info->poll_wevents &= ~new;
    ipnoise_setup_waiting(a_fd_info, a_waiting_packet);
}

void ipnoise_no_wait_wr(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet)
{
    pdebug(50, "ipnoise_no_wait_wr\n");
    uint32_t new = IPNOISE_POLLOUT;
    a_fd_info->poll_wevents &= ~new;
    ipnoise_setup_waiting(a_fd_info, a_waiting_packet);
}

static void ipnoise_flush_downlink(
    IPNoiseState *a_state)
{
//    PCIDevice     *d              = PCI_DEVICE(s);
    unsigned char   *buffer         = NULL;
    unsigned char   *buffer_ptr     = NULL;
    IPNoiseState    *state          = a_state;

    // attempt to flush downlink queue
    if (state->reg_cr & IPNOISE_CR_DLDD){
        // downlink already dirty
        goto out;
    }

    if (state->irq_level){
        goto out;
    }

    if (list_empty(&state->downlink.list)){
        goto out;
    }

    // allocate buffer
    buffer = ipnoise_malloc0(state->dma_downlink_size);
    if (!buffer){
        perr("cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            state->dma_downlink_size
        );
        goto fail;
    }

    buffer_ptr = buffer;

    // ok, now we can flush downlink DMA
    while (!list_empty(&state->downlink.list)){
        struct list_head    *first          = NULL;
        IPNoisePacket       *packet         = NULL;
        uint32_t            packet_size     = 0;

        // ok, list is not empty
        first       = state->downlink.list.next;
        packet      = list_entry(first, IPNoisePacket, list);
        packet_size = sizeof(*packet) + packet->data_size;

        // check what we have DMA space for this packet
        if ((buffer_ptr + packet_size) >
            (buffer + state->dma_downlink_size))
        {
            // there are no space for this packet
            pwarn("not enough downlink DMA size:\n"
                " buffer:               0x'%x'\n"
                " buffer_ptr:           0x'%x'\n"
                " packet_size:          '%d'\n"
                " s->dma_downlink_size: '%d'\n",
                (unsigned int)buffer,
                (unsigned int)buffer_ptr,
                packet_size,
                state->dma_downlink_size
            );
            break;
        }

        // remove from queue
        list_del(first);

        // copy packet header to downlink DMA
        memcpy(buffer_ptr, packet, sizeof(*packet));
        buffer_ptr += sizeof(*packet);

        if (packet->data){
            // copy packet data to downlink DMA
            if (0 < packet->data_size){
                memcpy(buffer_ptr, packet->data, packet->data_size);
                buffer_ptr += packet->data_size;
            }
        }

        if (50 <= g_debug_level){
            ipnoise_dump_packet(
                "ipnoise flush downlink",
                packet
            );
        }

        // free packet
        ipnoise_free_packet(packet);

        // mark downlink DMA dirty
        ipnoise_set_cr_flags(state, IPNOISE_CR_DLDD);
    }

    // write DMA
    pci_dma_write(
        state->pci_dev,
        state->dma_downlink_addr,
        (void *)buffer,
        state->dma_downlink_size
    );

    // free buffer
    ipnoise_free(buffer);

out:
    if (state->reg_cr & IPNOISE_CR_DLDD){
        if (!state->irq_level){
            // downlink data dirty, request IRQ
            state->irq_level = 1;
            qemu_set_irq(state->irq, state->irq_level);
        }
    }

    return;

fail:
    goto out;
}

static void ipnoise_flush(
    IPNoiseState *state)
{
    ipnoise_flush_uplink(state);
    ipnoise_flush_downlink(state);
}

static void ipnoise_add_downlink(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_packet)
{
    IPNoiseState *state = (IPNoiseState *)a_fd_info->state;
    a_packet->os_private    = a_fd_info->os_private;
    a_packet->poll_revents  = a_fd_info->poll_revents;
    list_add_tail(&a_packet->list, &state->downlink.list);
}

static int32_t ipnoise_normalize_error(
    IPNoiseState    *a_state,
    int32_t         a_err)
{
    int32_t         negative  = 0;
    int32_t         err       = a_err;

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
            perr("Cannot translate win32 error: '%d'\n",
                err
            );
            break;
    }
#endif

    if (negative){
        err = -err;
    }

    return err;
}

static int32_t ipnoise_socket_error(
    IPNoiseState    *a_state,
    int32_t         a_res)
{
    int32_t         err     = 0;
    int32_t         ret     = 0;
    IPNoiseState    *state  = a_state;

    err = socket_error();

    pdebug(10, "ipnoise_socket_error before normalize,"
        " res: '%d', socket_error(): '%d'\n",
        a_res,
        err
    );

    err = ipnoise_normalize_error(
        state,
        err
    );
    ret = (a_res < 0) ? -err : a_res;

    pdebug(10, "ipnoise_socket_error after normalize,"
        " ret: '%d', res: '%d', err: '%d'\n",
        ret,
        a_res,
        err
    );

    return ret;
}

static int32_t ipnoise_lsocket_error(
    IPNoiseState    *a_state,
    int32_t         a_fd)
{
    // get last socket error
    int32_t         res, err           = 0;
    int32_t         last_sock_err      = 0;
    socklen_t       last_sock_err_size = sizeof(last_sock_err);
    IPNoiseState    *state             = a_state;

    res = getsockopt(
        a_fd,
        SOL_SOCKET,
        SO_ERROR,
        (void *)&last_sock_err,
        &last_sock_err_size
    );

    if (!res){
        err = ipnoise_normalize_error(
            state,
            last_sock_err
        );
    }

    pdebug(10, "ipnoise_lsocket_error:"
        " a_fd: '%d',"
        " err: '%d',"
        " res: '%d'"
        "\n",
        a_fd,
        err,
        res
    );

    return err;
}

static void ipnoise_read_cb(
    void *a_opaque)
{
    IPNoiseFdInfo *fd_info  = (IPNoiseFdInfo *)a_opaque;
    IPNoiseState  *state    = (IPNoiseState *)fd_info->state;
    IPNoisePacket *new_packet     = NULL;
    IPNoisePacket *waiting_packet = fd_info->waiting_packet;

    int32_t res = 0;
    int32_t fd  = fd_info->fd;

    pdebug(10, "ipnoise_read_cb fd: '%d',"
        " waiting_packet: 0x'%x'\n",
        fd,
        (unsigned int)waiting_packet
    );

    // add information about poll event
    ipnoise_clear_poll_revents(fd_info, IPNOISE_POLLIN);

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
    new_packet->last_sock_err = ipnoise_lsocket_error(
        state,
        fd
    );

    if (!waiting_packet){
        pdebug(15, "empty waiting packet in read cb\n");
        // tell poll, what new data is available
        ipnoise_set_poll_revents(
            fd_info,
            IPNOISE_POLLIN
        );
        // ipnoise_no_wait_rd(fd_info, NULL);
        // there are no waiting packet
        new_packet->type    = IPNOISE_PACKET_POLL;
        new_packet->fd      = fd;
        // add answer in downlink queue
        ipnoise_add_downlink(fd_info, new_packet);
        goto out;
    }

    pdebug(10, "ipnoise_read_cb fd: '%d',"
        " fd_info->waiting_packet_type: 0x'%x'\n",
        fd,
        (unsigned int)fd_info->waiting_packet->type
    );

    switch (waiting_packet->type){
        case IPNOISE_PACKET_ACCEPT:
            {
                struct sockaddr addr;
                socklen_t       addrlen = sizeof(addr);
                do {
                    res = qemu_accept(fd, &addr, &addrlen);
                    res = ipnoise_socket_error(
                        state,
                        res
                    );
                } while (res < 0 && -EINTR == res);

                // store result
                new_packet->res = res;

                pdebug(10, "after accept,"
                    " fd: '%d',"
                    " res: '%d'\n",
                    fd,
                    res
                );

                // store result
                new_packet->last_sock_err = ipnoise_lsocket_error(
                    state,
                    fd
                );
                new_packet->un.accept.addrlen = min(
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
//            {
//                do {
//                    int32_t tmp;
//                    res = qemu_recv(
//                        fd,
//                        &tmp,
//                        0,              // read size
//                        0               // flags
//                    );
//                    res = ipnoise_socket_error(
//                        state,
//                        res
//                    );
//                } while (res < 0 && -EINTR == res);

//                new_packet->last_sock_err = ipnoise_lsocket_error(
//                    state,
//                    fd
//                );

                // add answer in downlink queue
                ipnoise_add_downlink(fd_info, new_packet);
//            }
            break;

        case IPNOISE_PACKET_RECVMSG:
            {
                int32_t         *addrlen    = NULL;
                struct sockaddr *addr       = NULL;

                addr = &new_packet->un.recvmsg.un.msg_end.addr;
                addrlen  = &new_packet->un.recvmsg.un.msg_end.addrlen;
                *addrlen = sizeof(*addr);

                do {
                    pdebug(10, "attempt to recvfrom:\n"
                        " data_size:    '%d'\n"
                        " flags:        0x%x\n",
                        fd_info->recv.data_size,
                        fd_info->recv_flags
                    );
                    res = recvfrom(
                        fd,
                        fd_info->recv.data,
                        fd_info->recv.data_size,
                        fd_info->recv_flags,
                        addr,
                        (socklen_t *)addrlen
                    );
                    pdebug(10, "after recvfrom fd: '%d',"
                        " res: '%d'\n",
                        fd,
                        res
                    );
                    res = ipnoise_socket_error(
                        state,
                        res
                    );

                    if (    0  < res
                        &&  50 <= g_debug_level)
                    {
                        qemu_hexdump(
                            (char *)fd_info->recv.data,
                            stderr,
                            "",
                            fd_info->recv.data_size
                        );
                    }
                } while (res < 0 && -EINTR == res);

                if (    fd_info->recv_flags & MSG_PEEK
                    &&  -EAGAIN == res)
                {
                    ipnoise_clear_poll_revents(
                        fd_info,
                        IPNOISE_POLLOUT
                    );
                }

                // store result
                new_packet->res = res;

                pdebug(10, "after recv fd: '%d',"
                    " res: '%d', addrlen: '%d'\n",
                    fd_info->fd,
                    res,
                    new_packet->un.recvmsg.un.msg_end.addrlen
                );

                if (0 && !res){
                    // EOF so setup IPNOISE_POLLIN flag agin
                    ipnoise_set_poll_revents(
                        fd_info,
                        IPNOISE_POLLOUT
                    );
                }

                // setup end packet
                {
                    new_packet->last_sock_err = ipnoise_lsocket_error(
                        state,
                        fd
                    );
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
            //ipnoise_no_wait_rd(fd_info, NULL);
            {
                perr("unsupported waiting_packet type: 0x'%x'\n",
                    waiting_packet->type
                );
                // mark events as handled
                ipnoise_clear_poll_revents(
                    fd_info,
                    IPNOISE_POLLIN | IPNOISE_POLLOUT
                );
                new_packet->last_sock_err = -ENOTSUP;
                // add answer in downlink queue
                ipnoise_add_downlink(fd_info, new_packet);
            }
            break;
    }

out:
    // flush answers
    ipnoise_flush_downlink(state);

    // clear fd_info
    ipnoise_clear_fd_info(fd_info);

    if (!waiting_packet){
        // we have pending event, so disable read callback
        ipnoise_no_wait_rd(fd_info, NULL);
    } else {
        ipnoise_wait_rd(fd_info, NULL);
    }

    return;

fail:
    if (new_packet){
        ipnoise_free_packet(new_packet);
        new_packet = NULL;
    }
    goto out;
}

static void ipnoise_write_cb(
    void *a_opaque)
{
    IPNoiseFdInfo *fd_info  = (IPNoiseFdInfo *)a_opaque;
    IPNoiseState  *state    = (IPNoiseState *)fd_info->state;
    IPNoisePacket *new_packet     = NULL;
    IPNoisePacket *waiting_packet = fd_info->waiting_packet;

    int32_t res = 0;
    int32_t fd  = fd_info->fd;


    pdebug(10, "ipnoise_write_cb, fd: '%d',"
        " fd_info->waiting_packet: 0x'%x'\n",
        fd,
        (unsigned int)fd_info->waiting_packet
    );

    // clear information about event in OS's poll's buffer
    ipnoise_clear_poll_revents(fd_info, IPNOISE_POLLOUT);

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
    new_packet->last_sock_err = ipnoise_lsocket_error(
        state,
        fd
    );

    if (!waiting_packet){
        //ipnoise_no_wait_wr(fd_info, NULL);
        pdebug(15, "empty waiting packet in write cb\n");
        // inform poll about new write available
        ipnoise_set_poll_revents(fd_info, IPNOISE_POLLOUT);
        // there are no waiting packet
        new_packet->type          = IPNOISE_PACKET_POLL;
        new_packet->fd            = fd;
        // add answer in downlink queue
        ipnoise_add_downlink(fd_info, new_packet);
        // mark event as handled
        //ipnoise_clear_poll_revents(
        //    fd_info,
        //    IPNOISE_POLLOUT
        //);
        goto out;
    }

    switch (waiting_packet->type){
        case IPNOISE_PACKET_CONNECT:
            //ipnoise_no_wait_rd_wr(fd_info, NULL);
            // MORIK DEL ME new_packet->last_sock_err = 0;
            // add information about poll event
            //ipnoise_clear_poll_revents(fd_info, IPNOISE_POLLOUT);
            // add answer in downlink queue
            ipnoise_add_downlink(fd_info, new_packet);
            break;

        case IPNOISE_PACKET_SENDMSG:
            //ipnoise_no_wait_wr(fd_info, NULL);
            // add information about poll event
            //ipnoise_clear_poll_revents(fd_info, IPNOISE_POLLOUT);
            {
                do {
                    if (0 >= fd_info->send.msg_iovlen){
                        break;
                    }

                    struct  iovec *iov  = NULL;
                    int32_t iov_id      = 0;

                    pdebug(
                        15,
                        "--- IPNOISE_PACKET_SENDMSG ---\n"
                    );

                    for (iov_id = 0;
                        iov_id < fd_info->send.msg_iovlen;
                        iov_id++)
                    {
                        iov = &fd_info->send.msg_iov[iov_id];
                        pdebug(15, "before send"
                            " iov_base: 0x%x"
                            " iov_len: '%d'\n",
                            (uint32_t)iov->iov_base,
                            (int32_t)iov->iov_len
                        );
                    }
                } while (0);

                do {
                    res = sendmsg(
                        fd,
                        &fd_info->send,
                        fd_info->send_flags
                    );

                    res = ipnoise_socket_error(
                        state,
                        res
                    );
                } while (res < 0 && -EINTR == res);

                // store result
                new_packet->res = res;

                do {
                    if (0 >= fd_info->send.msg_iovlen){
                        break;
                    }

                    struct  iovec *iov  = NULL;
                    int32_t iov_id      = 0;

                    for (iov_id = 0;
                        iov_id < fd_info->send.msg_iovlen;
                        iov_id++)
                    {
                        iov = &fd_info->send.msg_iov[iov_id];
                        pdebug(15, "after send"
                            " iov_base: 0x%x"
                            " iov_len: '%d'\n",
                            (uint32_t)iov->iov_base,
                            (int32_t)iov->iov_len
                        );
                    }
                } while (0);

                pdebug(15, "after send to fd: '%d',"
                    " msg_iovlen: '%d',"
                    " flags: '%d',"
                    " res: '%d'\n",
                    fd,
                    fd_info->send.msg_iovlen,
                    fd_info->send_flags,
                    res
                );

                // clear send buffer
                //ipnoise_clear_buff(&fd_info->send);
                ipnoise_clear_msg(&fd_info->send);
                fd_info->send_flags = 0;

                // add result
                new_packet->last_sock_err = ipnoise_lsocket_error(
                    state,
                    fd
                );

                // add answer in downlink queue
                ipnoise_add_downlink(fd_info, new_packet);
            }
            ipnoise_wait_wr(fd_info, NULL);
            break;

        default:
            // ipnoise_no_wait_wr(fd_info, NULL);
            // add information about poll event
            // ipnoise_clear_poll_revents(fd_info, IPNOISE_POLLOUT);
            {
                perr("unsupported waiting_packet type: 0x'%x'\n",
                    waiting_packet->type
                );
                // inform about error
                ipnoise_set_poll_revents(
                    fd_info,
                    IPNOISE_POLLIN | IPNOISE_POLLOUT
                );
                new_packet->last_sock_err = -ENOTSUP;
                // add answer in downlink queue
                ipnoise_add_downlink(fd_info, new_packet);
            }
            break;
    }

out:
    // flush answers
    ipnoise_flush_downlink(state);

    // clear fd_info
    ipnoise_clear_fd_info(fd_info);

    if (!waiting_packet){
        // we have pending event, so disable read callback
        ipnoise_no_wait_wr(fd_info, NULL);
    } else {
        ipnoise_wait_wr(fd_info, NULL);
    }

    return;

fail:
    if (new_packet){
        ipnoise_free_packet(new_packet);
        new_packet = NULL;
    }
    goto out;
}

static IPNoiseFdInfo * ipnoise_get_fd_info(
    IPNoiseState    *a_state,
    int32_t         a_fd)
{
    IPNoiseFdInfo       *ret        = NULL;
    struct list_head    *cur        = NULL;
    struct list_head    *tmp        = NULL;
    IPNoiseFdInfo       *fd_info    = NULL;
    IPNoiseState        *state      = a_state;

    list_for_each_safe(cur, tmp, &state->fds.list){
        fd_info = list_entry(cur, IPNoiseFdInfo, list);
        if (fd_info->fd == a_fd){
            ret = fd_info;
            break;
        }
    }

    return ret;
}

static void ipnoise_delete_fd_info(
    IPNoiseState    *a_state,
    IPNoiseFdInfo   *a_fd_info)
{
    if (a_fd_info){
        list_del(&a_fd_info->list);
        ipnoise_clear_fd_info(a_fd_info);
        ipnoise_free(a_fd_info);
    }
}

static IPNoiseFdInfo * _ipnoise_get_create_fd_info(
    IPNoiseState    *a_state,
    int32_t         a_fd,
    int32_t         a_do_clear)
{
    IPNoiseFdInfo   *fd_info    = NULL;
    IPNoiseState    *state      = a_state;

    fd_info = ipnoise_get_fd_info(state, a_fd);
    if (!fd_info){
        fd_info = ipnoise_malloc0(sizeof(*fd_info));
        list_add_tail(&fd_info->list, &state->fds.list);
    } else {
        pwarn("_ipnoise_get_create_fd_info,"
            " fd: '%d' already exist,"
            " do_clear: %d\n",
            a_fd,
            a_do_clear
        );
    }

    if (a_do_clear){
        struct list_head list;
        // backup list head
        memcpy(&list, &fd_info->list, sizeof(list));
        // clear
        memset(fd_info, 0x00, sizeof(*fd_info));
        // restore list head
        memcpy(
            &fd_info->list,
            &list,
            sizeof(fd_info->list)
        );
    }

    fd_info->fd     = a_fd;
    fd_info->state  = state;

    return fd_info;
}

static IPNoiseFdInfo * ipnoise_create_fd_info(
    IPNoiseState    *a_state,
    int32_t         a_fd)
{
    IPNoiseState    *state  = a_state;
    IPNoiseFdInfo   *ret    = NULL;

    ret = _ipnoise_get_create_fd_info(state, a_fd, 1);

    return ret;
}

// ---------------------- cmd handlers ----------------------

static void ipnoise_hostos_open_socket(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t  fd          = a_packet->fd;
    uint32_t pf_family   = a_packet->un.open_socket.pf_family;
    uint32_t type        = a_packet->un.open_socket.type;
    uint32_t protocol    = a_packet->un.open_socket.protocol;
    void     *os_private = a_packet->os_private;

    IPNoiseState    *state      = a_state;
    IPNoisePacket   *new_packet = NULL;
    IPNoiseFdInfo   *fd_info    = NULL;

    pdebug(1, "request open socket\n");

    {
        struct list_head    *cur        = NULL;
        struct list_head    *tmp        = NULL;
        int32_t             count       = 0;

        pdebug(1, "dump &a_state->fds.list\n");
        list_for_each_safe(cur, tmp, &a_state->fds.list){
            fd_info = list_entry(cur, IPNoiseFdInfo, list);
            count++;
            pdebug(1, "  fds.list[%d] fd: '%d'\n",
                count,
                fd_info->fd
            );
        }
    }

    // allocate new packet
    new_packet = ipnoise_create_packet(a_packet);
    if (!new_packet){
        ipnoise_dump_packet(
            "cannot allocate new packet from: ",
            a_packet
        );
        goto fail;
    }

    if (0 > a_packet->fd){
        // if fd not exist - create new
        fd = qemu_socket(pf_family, type, protocol);
    }

    // store result
    new_packet->res = fd;

    if (0 <= fd){
        // store fd in answer
        new_packet->last_sock_err = fd;

        // create fd info
        fd_info = ipnoise_create_fd_info(state, fd);
        fd_info->os_private = os_private;
        qemu_set_nonblock(fd);
    } else {
        new_packet->last_sock_err = ipnoise_lsocket_error(
            state,
            fd
        );
    }

    if (0 <= a_packet->fd){
        // it is accept call, because fd was in packet,
        // so add events
        ipnoise_set_poll_revents(
            fd_info,
            IPNOISE_POLLIN | IPNOISE_POLLOUT
        );
    }

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

out:
    ipnoise_flush_downlink(state);

    return;

fail:
    goto out;
}

static void ipnoise_hostos_close(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t         res, fd         = a_packet->fd;
    IPNoisePacket   *new_packet     = NULL;
    IPNoiseFdInfo   *fd_info        = NULL;
    IPNoiseState    *state          = a_state;

    if (0 > fd){
        // TODO XXX report error here (to downlink)
        perr("attempt to close fd: '%d'\n", fd);
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(state, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        perr("cannot get fd info for fd: '%d'\n", fd);
        goto fail;
    }

    // remove callbacks
    ipnoise_no_wait_rd_wr(fd_info, NULL);

    // allocate new packet
    new_packet = ipnoise_create_packet(a_packet);
    if (!new_packet){
        ipnoise_dump_packet(
            "cannot allocate new packet from: ",
            a_packet
        );
        goto fail;
    }

    do {
        // close socket
        res = closesocket(fd);
        res = ipnoise_socket_error(
            state,
            res
        );
    } while (res < 0 && -EINTR == res);

    // store result
    new_packet->res = res;

    new_packet->last_sock_err = ipnoise_lsocket_error(
        state,
        fd
    );

    pdebug(10, "close fd: '%d', res: '%d'\n",
        fd,
        res
    );

    // clear poll events
    fd_info->poll_revents = 0;

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

out:
    ipnoise_flush_downlink(state);

    // delete fd info
    ipnoise_delete_fd_info(state, fd_info);

    return;

fail:
    goto out;
}

static struct iovec * ipnoise_alloc_iov(
    IPNoiseState    *a_state,
    struct msghdr   *a_msg)
{
    struct iovec    *msg_iov    = NULL;
    int32_t         size        = 0;

    // msg iov exist, resize
    size = sizeof(*a_msg->msg_iov) * (a_msg->msg_iovlen + 1);
    a_msg->msg_iov = ipnoise_realloc(
        a_msg->msg_iov,
        size
    );
    if (!a_msg->msg_iov){
        perr("cannot allocate memory,"
            " was needed: '%d' bytes\n",
            size
        );
        goto fail;
    }
    msg_iov = &a_msg->msg_iov[a_msg->msg_iovlen];
    memset(
        msg_iov,
        0x00,
        sizeof(*a_msg->msg_iov)
    );
    a_msg->msg_iovlen++;

    pdebug(30, "a_msg: 0x%x, was allocated new iov: 0x%x,"
        " a_msg->iov: 0x%x,"
        " a_msg->msg_iovlen: '%d'\n",
        (uint32_t)a_msg,
        (uint32_t)msg_iov,
        (uint32_t)a_msg->msg_iov,
        (int32_t)a_msg->msg_iovlen
    );

out:
    return msg_iov;
fail:
    goto out;
}

static void ipnoise_hostos_sendmsg(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t         fd          = a_packet->fd;
    IPNoiseFdInfo   *fd_info    = NULL;
    uint32_t        type        = a_packet->un.sendmsg.type;
    IPNoiseState    *state      = a_state;

    if (0 > fd){
        // TODO XXX report error here (to downlink)
        perr("attempt sendmsg to invalid fd: '%d'\n", fd);
        goto fail;
    }

    // search fd info
    fd_info = ipnoise_get_fd_info(state, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        perr("cannot get fd info for fd: '%d'\n", fd);
        goto fail;
    }

    switch (type){
        case IPNOISE_MSG_TYPE_FLAGS:
            if (0 < a_packet->data_size){
                memcpy(
                    &fd_info->send_flags,
                    a_packet->data,
                    min(
                        sizeof(fd_info->send_flags),
                        a_packet->data_size
                    )
                );
            }
            break;

        case IPNOISE_MSG_TYPE_NAME:
            // msg name
            if (a_packet->data_size > 0){
                fd_info->send.msg_namelen = a_packet->data_size;
                if (fd_info->send.msg_name){
                    perr("fd_info->send.name not free\n");
                    ipnoise_free(fd_info->send.msg_name);
                    fd_info->send.msg_name = NULL;
                }
                fd_info->send.msg_name = ipnoise_malloc0(
                    fd_info->send.msg_namelen
                );
                if (!fd_info->send.msg_name){
                    perr("cannot allocate memory,"
                        " was needed: '%d' byte(s)\n",
                        fd_info->send.msg_namelen
                    );
                    goto fail;
                }
                memcpy(
                    fd_info->send.msg_name,
                    a_packet->data,
                    fd_info->send.msg_namelen
                );
            }
            break;
        case IPNOISE_MSG_TYPE_IOV:
            // msg content
            {
                struct iovec *msg_iov = NULL;

                msg_iov = ipnoise_alloc_iov(
                    state,
                    &fd_info->send
                );
                if (!msg_iov){
                    perr("cannot allocate new msg_iov\n");
                    break;
                }
                // MORIK TODO MORIK

                msg_iov->iov_len = a_packet->data_size;
                if (0 < msg_iov->iov_len){
                    msg_iov->iov_base = ipnoise_malloc0(
                        msg_iov->iov_len
                    );
                }
                if (!msg_iov->iov_base){
                    perr("cannot allocate memory,"
                        " was needed: '%d' byte(s)\n",
                        msg_iov->iov_len
                    );
                    goto fail;
                }
                // copy data
                memcpy(
                    msg_iov->iov_base,
                    a_packet->data,
                    msg_iov->iov_len
                );
                // MORIK
//                ipnoise_hexdump(
//                    (unsigned char *)fd_info->send.data,
//                    (int32_t)fd_info->send.data_size
//                );
            }
            break;

        case IPNOISE_MSG_TYPE_END:
            // msg end
            if (a_packet->non_block){
                // call write immediately
                ipnoise_call_wr(fd_info, a_packet);
            } else {
                // waiting write events (write)
                ipnoise_wait_wr(fd_info, a_packet);
            }
            break;

        default:
            perr("unknown sendmsg packet type: '%d'\n", type);
            break;
    }

out:
    return;
fail:
    goto out;
}

static void ipnoise_hostos_recvmsg(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t         fd          = a_packet->fd;
    IPNoiseFdInfo   *fd_info    = NULL;
    uint32_t        type        = a_packet->un.recvmsg.type;
    IPNoiseState    *state      = a_state;

    if (0 > fd){
        // TODO XXX report error here (to downlink)
        perr("attempt to recv from fd: '%d'\n", fd);
        goto fail;
    }

    // search fd info
    fd_info = ipnoise_get_fd_info(state, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        perr("fd info not found for fd: '%d'\n", fd);
        goto fail;
    }

    switch (type){
        case IPNOISE_MSG_TYPE_FLAGS:
            // msg flags
            if (0 < a_packet->data_size){
                memcpy(
                    &fd_info->recv_flags,
                    a_packet->data,
                    min(
                        sizeof(fd_info->recv_flags),
                        a_packet->data_size
                    )
                );
            }
            break;

        case IPNOISE_MSG_TYPE_NAME:
            // msg name
            if (0 < a_packet->data_size){
                fd_info->recv.namelen = a_packet->data_size;
                if (fd_info->recv.name){
                    perr("fd_info->recv.name not free\n");
                    ipnoise_free(fd_info->recv.name);
                    fd_info->recv.name = NULL;
                }
                fd_info->recv.name = ipnoise_malloc0(
                    fd_info->recv.namelen
                );
                if (!fd_info->recv.name){
                    perr("cannot allocate memory,"
                        " was needed: '%d' byte(s)\n",
                        fd_info->recv.namelen
                    );
                    goto fail;
                }
                memcpy(
                    fd_info->recv.name,
                    a_packet->data,
                    fd_info->recv.namelen
                );
            }
            break;

        case IPNOISE_MSG_TYPE_IOV:
            // msg iov
            {
                uint32_t max_downlink_dma_data_size = 0;

                // calculate max downlink dma data size
                max_downlink_dma_data_size = state->dma_downlink_size
                    - 10 * sizeof(*a_packet);

                fd_info->recv.data_size = min(
                    max_downlink_dma_data_size,
                    a_packet->un.recvmsg.un.msg_iov.iov_len
                );

                if (fd_info->recv.data){
                    perr("fd_info->recv.data not free\n");
                    ipnoise_free(fd_info->recv.data);
                    fd_info->recv.data = NULL;
                }

                fd_info->recv.data = ipnoise_malloc0(
                    fd_info->recv.data_size
                );
                if (!fd_info->recv.data){
                    perr("cannot allocate memory,"
                        " was needed: '%d' byte(s)\n",
                        fd_info->recv.data_size
                    );
                    goto fail;
                }
            }
            break;

        case IPNOISE_MSG_TYPE_END:
            if (a_packet->non_block){
                // process immediately
                ipnoise_call_rd(fd_info, a_packet);
            } else {
                // waiting read events (read)
                ipnoise_wait_rd(fd_info, a_packet);
            }
            break;

        default:
            perr("unknown recvmsg packet type: '%d'\n",
                type
            );
            break;
    }

out:
    return;
fail:
    goto out;
}

static void ipnoise_hostos_listen(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t         res, fd     = a_packet->fd;
    int32_t         backlog     = a_packet->un.listen.backlog;
    IPNoiseFdInfo   *fd_info    = NULL;
    IPNoisePacket   *new_packet = NULL;
    IPNoiseState    *state      = a_state;

    if (0 > fd){
        // TODO XXX report error here (to downlink)
        perr("attempt to listen fd: '%d'\n", fd);
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(state, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        ipnoise_dump_packet(
            "cannot get/allocate fd_info\n",
            new_packet
        );
        goto fail;
    }

    do {
        res = listen(fd, backlog);
        res = ipnoise_socket_error(
            state,
            res
        );
    } while (res < 0 && -EINTR == res);

    if (!res){
        ipnoise_wait_rd(fd_info, NULL);
    }

    // allocate new packet
    new_packet = ipnoise_create_packet(a_packet);
    if (!new_packet){
        ipnoise_dump_packet("cannot allocate new packet from: ",
            a_packet
        );
        goto fail;
    }

    // store result
    new_packet->res = res;

    // store result
    new_packet->last_sock_err = ipnoise_lsocket_error(
        state,
        fd
    );

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

out:
    ipnoise_flush_downlink(state);

    // wait read or write events (for poll)
    ipnoise_wait_rd_wr(fd_info, NULL);

    return;

fail:
    goto out;
}

static void ipnoise_hostos_bind(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t         res, fd     = a_packet->fd;
    struct sockaddr *addr       = &a_packet->un.bind.addr;
    int32_t         addrlen     = a_packet->un.bind.addrlen;
    IPNoiseFdInfo   *fd_info    = NULL;
    IPNoisePacket   *new_packet = NULL;
    IPNoiseState    *state      = a_state;

    if (0 > fd){
        // TODO XXX report error here (to downlink)
        perr("attempt to bind fd: '%d'\n", fd);
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(state, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        ipnoise_dump_packet(
            "cannot get fd info while bind",
            a_packet
        );
        goto fail;
    }

    do {  // TODO XXX remove this block
        // enable reuse
        int on = 1;
        res = setsockopt(
            fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            (char *)&on,
            sizeof(on)
        );
        res = ipnoise_socket_error(
            state,
            res
        );
    } while (res < 0 && -EINTR == res);

    do {
        res = bind(fd, addr, addrlen);
        res = ipnoise_socket_error(
            state,
            res
        );
    } while (res < 0 && -EINTR == res);

    // allocate new packet
    new_packet = ipnoise_create_packet(a_packet);
    if (!new_packet){
        ipnoise_dump_packet(
            "cannot allocate new packet from: ",
            a_packet
        );
        goto fail;
    }

    // store result
    new_packet->res = res;

    // store result
    new_packet->last_sock_err = ipnoise_lsocket_error(
        state,
        fd
    );

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

out:
    ipnoise_flush_downlink(state);

    // wait read or write events (for poll)
    ipnoise_wait_rd_wr(fd_info, NULL);

    return;

fail:
    goto out;
}

static void ipnoise_hostos_accept(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t         fd          = a_packet->fd;
    IPNoiseFdInfo   *fd_info    = NULL;
    IPNoiseState    *state      = a_state;

    if (0 > fd){
        // TODO XXX report error here (to downlink)
        perr("attempt to accept from fd: '%d'\n", fd);
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(state, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        perr("cannot get/allocate fd_info\n");
        goto fail;
    }

    if (a_packet->non_block){
        // call read immediately
        ipnoise_call_rd(fd_info, a_packet);
    } else {
        // waiting read events (accept)
        ipnoise_wait_rd(fd_info, a_packet);
    }

out:
    return;

fail:
    goto out;
}

static void ipnoise_hostos_poll(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t         fd          = a_packet->fd;
    IPNoiseFdInfo   *fd_info    = NULL;
    IPNoiseState    *state      = a_state;

    if (0 > fd){
        // TODO XXX report error here (to downlink)
        perr("attempt to accept from fd: '%d'\n", fd);
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(state, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        perr("cannot get/allocate fd_info\n");
        goto fail;
    }

    fd_info->poll_revents = a_packet->un.poll.events;

out:
    return;

fail:
    goto out;
}

static void ipnoise_hostos_sockopt_set(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t         fd          = a_packet->fd;
    IPNoiseFdInfo   *fd_info    = NULL;
    IPNoiseState    *state      = a_state;
    IPNoisePacket   *new_packet = NULL;
    int32_t         res         = 0;

    if (0 > fd){
        // TODO XXX report error here (to downlink)
        perr("attempt to set sockopt for fd: '%d'\n", fd);
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(state, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        perr("cannot get fd_info for fd: '%d'\n",
            fd
        );
        goto fail;
    }

    pdebug(10, "before setsockopt:\n"
        "  fd:       '%d'\n"
        "  level:    '%d'\n"
        "  optname:  '%d'\n"
        "  optlen:   '%d'\n",
        fd,
        a_packet->un.sockopt.level,
        a_packet->un.sockopt.optname,
        a_packet->data_size
    );

    // allocate new packet
    new_packet = ipnoise_create_packet(a_packet);
    if (!new_packet){
        ipnoise_dump_packet(
            "cannot allocate new packet from: ",
            a_packet
        );
        goto fail;
    }

    do {
        res = qemu_setsockopt(
            fd,
            a_packet->un.sockopt.level,
            a_packet->un.sockopt.optname,
            a_packet->data,
            a_packet->data_size
        );
        res = ipnoise_socket_error(
            state,
            res
        );
    } while (res < 0 && -EINTR == res);

    // store result
    new_packet->res = res;

    pdebug(10, "after setsockopt:\n"
        "  fd:       '%d'\n"
        "  level:    '%d'\n"
        "  optname:  '%d'\n"
        "  optlen:   '%d'\n"
        "  res:      '%d'\n",
        fd,
        new_packet->un.sockopt.level,
        new_packet->un.sockopt.optname,
        new_packet->data_size,
        res
    );

    // store result
    new_packet->last_sock_err = ipnoise_lsocket_error(
        state,
        fd
    );

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

out:
    ipnoise_flush_downlink(state);

    // wait read or write events (for poll)
    ipnoise_wait_rd_wr(fd_info, NULL);

    return;

fail:
    goto out;
}

static void ipnoise_hostos_sockopt_get(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t         fd          = a_packet->fd;
    IPNoiseFdInfo   *fd_info    = NULL;
    IPNoiseState    *state      = a_state;
    IPNoisePacket   *new_packet = NULL;
    int32_t         size        = 0;
    int32_t         res         = 0;

    if (0 > fd){
        // TODO XXX report error here (to downlink)
        perr("attempt to get sockopt from fd: '%d'\n", fd);
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(state, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        perr("cannot get/allocate fd_info\n");
        goto fail;
    }

    // allocate new packet
    new_packet = ipnoise_create_packet(a_packet);
    if (!new_packet){
        ipnoise_dump_packet(
            "cannot allocate new packet from: ",
            a_packet
        );
        goto fail;
    }

    size = new_packet->un.sockopt.optlen;
    if (0 < size){
        new_packet->data = ipnoise_malloc0(size);
        if (new_packet->data){
            new_packet->data_size = size;
        } else {
            perr("cannot allocate memory"
                " was need '%d' byte(s)\n",
                size
            );
        }
    }

    pdebug(10, "before getsockopt:\n"
        " fd:           '%d'\n"
        " level:        '%d'\n"
        " optname:      '%d'\n"
        " data:         '%s'\n"
        " data_size:    0x%x\n",
        fd,
        a_packet->un.sockopt.level,
        a_packet->un.sockopt.optname,
        new_packet->data,
        new_packet->data_size
    );

    do {
        if (!new_packet->data){
            // memory allocate failed
            // or invalid (empty) option's data size
            res = -ENOMEM;
            break;
        }
        res = qemu_getsockopt(
            fd,
            a_packet->un.sockopt.level,
            a_packet->un.sockopt.optname,
            new_packet->data,
            &new_packet->data_size
        );
        new_packet->un.sockopt.optlen = new_packet->data_size;
        res = ipnoise_socket_error(
            state,
            res
        );
    } while (res < 0 && -EINTR == res);

    // store result
    new_packet->res = res;

    pdebug(10, "after getsockopt:\n"
        " fd:           '%d'\n"
        " res:          '%d'\n"
        " level:        '%d'\n"
        " optname:      '%d'\n"
        " data:         '%s'\n"
        " data_size:    0x%x\n",
        fd,
        res,
        a_packet->un.sockopt.level,
        a_packet->un.sockopt.optname,
        new_packet->data,
        new_packet->data_size
    );

    // store result
    new_packet->last_sock_err = ipnoise_lsocket_error(
        state,
        fd
    );

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

out:
    ipnoise_flush_downlink(state);

    // wait read or write events (for poll)
    ipnoise_wait_rd_wr(fd_info, NULL);

    return;

fail:
    goto out;
}

static void ipnoise_hostos_unknown(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t        fd       = a_packet->fd;
    IPNoiseFdInfo  *fd_info = NULL;

    if (0 > fd){
        goto out;
    }

    fd_info = ipnoise_get_fd_info(
        a_state,
        fd
    );
    if (!fd_info){
        perr("cannot get fd info for fd: '%d'\n",
            fd
        );
        goto fail;
    }

    ipnoise_call_rd(fd_info, a_packet);

out:
    return;

fail:
    goto out;
}

void ipnoise_hostos_connect(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    int32_t         res, fd  = a_packet->fd;
    struct sockaddr *daddr   = &a_packet->un.connect.daddr;
    int32_t         daddrlen = a_packet->un.connect.daddrlen;

    IPNoiseState            *state      = a_state;
    IPNoiseFdInfo           *fd_info    = NULL;
    static IPNoisePacket    *new_packet = NULL;

    if (0 > fd){
        // TODO XXX report error here (to downlink)
        perr("attempt to connect with fd: '%d'\n", fd);
        goto fail;
    }

    fd_info = ipnoise_get_fd_info(state, fd);
    if (!fd_info){
        // TODO XXX report error here (to downlink)
        perr("cannot get fd info for fd: '%d'\n", fd);
        goto fail;
    }

    do {
        res = connect(fd, daddr, daddrlen);
        res = ipnoise_socket_error(state, res);
    } while (res < 0 && -EINTR == res);

    if (res >= 0){
        // we are connected
        ipnoise_wait_rd_wr(fd_info, a_packet);
    } else if ( -EINPROGRESS == res
        ||      -EALREADY    == res)
    {
        // all ok
        if (!a_packet->non_block){
            // wait read or write events (connect result)
            ipnoise_wait_rd_wr(fd_info, a_packet);
            goto out;
        }
    } else {
        // we have error
    }

    // create new packet
    new_packet = ipnoise_create_packet(a_packet);
    if (!new_packet){
        perr("cannot allocate new packet\n");
        goto fail;
    }

    // store result
    new_packet->res = res;

    // get last socket error
    new_packet->last_sock_err = ipnoise_lsocket_error(
        state,
        fd
    );

    // add answer in downlink queue
    ipnoise_add_downlink(fd_info, new_packet);

    ipnoise_flush_downlink(state);

    // wait read or write events (for poll)
    ipnoise_wait_rd_wr(fd_info, NULL);

out:
    return;

fail:
    goto out;
}

static void ipnoise_uplink_process_packet(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet)
{
    IPNoiseState *state = a_state;

    if (50 <= g_debug_level){
        ipnoise_dump_packet(
            "ipnoise_uplink_process_packet",
            a_packet
        );
    }

    switch (a_packet->type){
        case IPNOISE_PACKET_FREE:
            ipnoise_dump_packet(
                "attempt to process empty uplink packet",
                a_packet
            );
            break;

        case IPNOISE_PACKET_OPEN_SOCKET:
            ipnoise_hostos_open_socket(state, a_packet);
            break;

        case IPNOISE_PACKET_CLOSE:
            ipnoise_hostos_close(state, a_packet);
            break;

        case IPNOISE_PACKET_CONNECT:
            ipnoise_hostos_connect(state, a_packet);
            break;

        case IPNOISE_PACKET_SENDMSG:
            ipnoise_hostos_sendmsg(state, a_packet);
            break;

        case IPNOISE_PACKET_RECVMSG:
            ipnoise_hostos_recvmsg(state, a_packet);
            break;

        case IPNOISE_PACKET_POLL:
            ipnoise_hostos_poll(state, a_packet);
            break;

        case IPNOISE_PACKET_BIND:
            ipnoise_hostos_bind(state, a_packet);
            break;

        case IPNOISE_PACKET_LISTEN:
            ipnoise_hostos_listen(state, a_packet);
            break;

        case IPNOISE_PACKET_ACCEPT:
            ipnoise_hostos_accept(state, a_packet);
            break;

        case IPNOISE_PACKET_SOCKOPT_GET:
            ipnoise_hostos_sockopt_get(state, a_packet);
            break;

        case IPNOISE_PACKET_SOCKOPT_SET:
            ipnoise_hostos_sockopt_set(state, a_packet);
            break;

        default:
            perr("unsupported packet type: '%d'\n",
                a_packet->type
            );
            ipnoise_hostos_unknown(state, a_packet);
            break;
    }

    return;
}

static void ipnoise_flush_uplink(
    IPNoiseState *a_state)
{
    unsigned char *buffer       = NULL;
    unsigned char *buffer_ptr   = NULL;
    IPNoiseState  *state        = a_state;

    if (!(IPNOISE_CR_ULDD & state->reg_cr)){
        goto out;
    }

    // allocate buffer
    buffer = ipnoise_malloc0(state->dma_uplink_size);
    if (!buffer){
        perr("cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            state->dma_downlink_size
        );
        goto fail;
    }
    buffer_ptr = buffer;

    // read uplink dma
    pci_dma_read(
        state->pci_dev,
        state->dma_uplink_addr,
        buffer,
        state->dma_uplink_size
    );

    while (buffer_ptr < (buffer + state->dma_uplink_size)){
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
        ipnoise_uplink_process_packet(state, packet);

        // seek to next packet
        buffer_ptr += sizeof(*packet) + packet->data_size;
    };

    // mark uplink us free
    ipnoise_clear_cr_flags(state, IPNOISE_CR_ULDD);

    // free buffer
    ipnoise_free(buffer);

out:
    return;

fail:
    goto out;
}

static void ipnoise_process_command(
    IPNoiseState *a_state)
{
    // PCIDevice   *d     = PCI_DEVICE(s);
    IPNoiseState    *state  = a_state;
    uint32_t        cmd_id  = state->reg_cmd_id;

    if (    !(IPNOISE_CR_SELF_TEST_PASSED & state->reg_cr)
        &&  IPNOISE_CMD_RESET != cmd_id)
    {
        perr("cannot process command: '%d',"
            " self test not passed,"
            " only reset command available\n",
            cmd_id
        );
        goto fail;
    }

    switch (cmd_id){
        case IPNOISE_CMD_RESET:
            pdebug(20, "[ ipnoise_process_command ]"
                " IPNOISE_CMD_RESET\n"
            );
            ipnoise_s_reset(state);
            break;

        case IPNOISE_CMD_SET_ULDD:
            pdebug(20, "[ ipnoise_process_command ]"
                " IPNOISE_CMD_SET_ULDD\n"
            );
            ipnoise_set_cr_flags(state, IPNOISE_CR_ULDD);
            break;

        case IPNOISE_CMD_CLEAR_DLDD:
            pdebug(20, "[ ipnoise_process_command ]"
                " IPNOISE_CMD_CLEAR_DLDD\n"
            );
            ipnoise_clear_cr_flags(state, IPNOISE_CR_DLDD);
            break;

        case IPNOISE_CMD_SET_IRQ_LEVEL:
            pdebug(20, "[ ipnoise_process_command ]"
                " IPNOISE_CMD_SET_IRQ_LEVEL value: '%d'\n",
                state->reg_cmd_0
            );
            if (state->irq_level != state->reg_cmd_0){
                state->irq_level = state->reg_cmd_0;
                qemu_set_irq(state->irq, state->irq_level);
                if (!state->irq_level){
                    ipnoise_flush(state);
                }
            }
            break;

        case IPNOISE_CMD_SET_DOWNLINK_DMA_ADDR:
            pdebug(20, "[ ipnoise_process_command ]"
                " IPNOISE_CMD_SET_DOWNLINK_DMA_ADDR\n"
            );
            state->dma_downlink_addr = state->reg_cmd_0;
            // update DMA flags
            ipnoise_cr_dma(state);
            break;

        case IPNOISE_CMD_SET_DOWNLINK_DMA_SIZE:
            pdebug(20, "[ ipnoise_process_command ]"
                " IPNOISE_CMD_SET_DOWNLINK_DMA_SIZE\n"
            );
            state->dma_downlink_size = state->reg_cmd_0;
            // update DMA flags
            ipnoise_cr_dma(state);
            break;

        case IPNOISE_CMD_SET_UPLINK_DMA_ADDR:
            pdebug(20, "[ ipnoise_process_command ]"
                " IPNOISE_CMD_SET_UPLINK_DMA_ADDR\n"
            );
            state->dma_uplink_addr = state->reg_cmd_0;
            // update DMA flags
            ipnoise_cr_dma(state);
            break;

        case IPNOISE_CMD_SET_UPLINK_DMA_SIZE:
            pdebug(20, "[ ipnoise_process_command ]"
                " IPNOISE_CMD_SET_UPLINK_DMA_SIZE\n"
            );
            state->dma_uplink_size = state->reg_cmd_0;
            // update DMA flags
            ipnoise_cr_dma(state);
            break;

        case IPNOISE_CMD_SET_DEBUG_LEVEL:
            pdebug(20, "[ ipnoise_process_command ]"
                " IPNOISE_CMD_SET_DEBUG_LEVEL\n"
            );
            g_debug_level = state->reg_cmd_0;
            pinfo("debug set to: '%d'\n", g_debug_level);
            break;

        case IPNOISE_CMD_SET_POLL_EVENTS:
            pdebug(20, "[ ipnoise_process_command ]"
                " IPNOISE_CMD_SET_POLL_EVENTS\n"
            );
            int32_t         fd          = state->reg_cmd_0;
            int32_t         events      = state->reg_cmd_1;
            IPNoiseFdInfo   *fd_info    = NULL;

            fd_info = ipnoise_get_fd_info(state, fd);
            if (fd_info){
                pinfo("set poll events: 0x%x for fd '%d'\n",
                    events,
                    fd
                );
                fd_info->poll_revents = events;
            } else {
                perr("cannot get fd info for fd: '%d'\n",
                    fd
                );
            }
            break;

        case IPNOISE_CMD_GET_DEBUG_LEVEL:
            pdebug(20, "[ ipnoise_process_command ]"
                " IPNOISE_CMD_GET_DEBUG_LEVEL\n"
            );
            state->reg_cmd_0 = g_debug_level;
            break;

        default:
            perr("[ ipnoise_process_command ]"
                " unsupported command ID: '%d'\n",
                cmd_id
            );
            break;
    }

out:
    return;
fail:
    goto out;
}

void ipnoise_h_reset(void *a_opaque)
{
    IPNoiseState *state = (IPNoiseState *)a_opaque;
    _ipnoise_generic_reset(state);
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

static inline int64_t ipnoise_get_next_poll_time(
    int64_t a_time_ms)
{
    int64_t ret = 0;
    ret = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL)
        + a_time_ms * 1e6;
    return ret;
}

static void ipnoise_poll_timer(void *a_opaque)
{
    IPNoiseState *state = (IPNoiseState *)a_opaque;

    pdebug(1, "MORIK's timer\n");

//    pdebug("timer before call ipnoise_flush_downlink {\n");
//    ipnoise_flush_downlink(s);
//    pdebug("timer after call ipnoise_flush_downlink }\n");

    timer_mod_ns(
        state->poll_timer,
        ipnoise_get_next_poll_time(5000)
    );

    return;
}

static int pci_ipnoise_init(
    PCIDevice *a_pci_dev)
{
    IPNoiseState  *state    = IPNOISE(a_pci_dev);
    PCIDevice     *device   = PCI_DEVICE(state);
    uint8_t       *pci_conf = a_pci_dev->config;

#ifdef _WIN32
    WSADATA Data;
    WSAStartup(MAKEWORD(2,0), &Data);
#endif

    INIT_LIST_HEAD(&state->downlink.list);
    INIT_LIST_HEAD(&state->fds.list);

    pci_set_word(
        pci_conf + PCI_STATUS,
        PCI_STATUS_FAST_BACK | PCI_STATUS_DEVSEL_MEDIUM
    );

    pci_set_word(pci_conf + PCI_SUBSYSTEM_VENDOR_ID,    0x00);
    pci_set_word(pci_conf + PCI_SUBSYSTEM_ID,           0x00);

    pci_conf[PCI_INTERRUPT_PIN] = 1; // interrupt pin A
    pci_conf[PCI_MIN_GNT]       = 0x06;
    pci_conf[PCI_MAX_LAT]       = 0xff;

    memory_region_init_io(
        &state->mmio_bar,
        OBJECT(state),
        &ipnoise_io_ops,
        state,
        "ipnoise-io",
        IPNOISE_IOPORT_SIZE
    );

    pci_register_bar(
        a_pci_dev,
        0,
        PCI_BASE_ADDRESS_SPACE_IO,
        &state->mmio_bar
    );

    pci_register_bar(a_pci_dev, 1, 0, &state->mmio);

    state->irq     = pci_allocate_irq(device);
    state->pci_dev = a_pci_dev;

    state->poll_timer = timer_new_ns(
        QEMU_CLOCK_VIRTUAL,
        ipnoise_poll_timer,
        state
    );
    timer_mod_ns(
        state->poll_timer,
        ipnoise_get_next_poll_time(5000)
    );

    return 0;
}

static void pci_reset(
    DeviceState *a_dev)
{
    IPNoiseState *state = IPNOISE(a_dev);
    pdebug(1, "ipnoise pci_reset\n");
    ipnoise_h_reset(state);
}

static void ipnoise_class_init(
    ObjectClass *a_class,
    void        *a_data)
{
    DeviceClass     *dc     = DEVICE_CLASS(a_class);
    PCIDeviceClass  *pdc    = PCI_DEVICE_CLASS(a_class);

    pdc->init       = pci_ipnoise_init;
    pdc->exit       = pci_ipnoise_uninit;
    pdc->vendor_id  = PCI_VENDOR_ID_IPNOISE;
    pdc->device_id  = PCI_DEVICE_ID_HOSTOS;
    pdc->revision   = 0x00;
    pdc->class_id   = PCI_CLASS_SYSTEM_OTHER;
    dc->reset       = pci_reset;
    dc->vmsd        = &vmstate_pci_ipnoise;
    dc->props       = ipnoise_properties;

    set_bit(DEVICE_CATEGORY_NETWORK, dc->categories);
}

static TypeInfo ipnoise_info = {
    .name          = TYPE_IPNOISE,
    .parent        = TYPE_PCI_DEVICE,
    .instance_size = sizeof(IPNoiseState),
    .class_init    = ipnoise_class_init
};

static void pci_ipnoise_register_types(void)
{
    type_register_static(&ipnoise_info);
}

type_init(pci_ipnoise_register_types)

