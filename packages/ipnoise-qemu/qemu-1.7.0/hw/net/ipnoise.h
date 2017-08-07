#ifndef IPNOISE_H
#define IPNOISE_H

#define IPNOISE_IOPORT_SIZE       0x40
#define IPNOISE_PNPMMIO_SIZE      0x20

#include <stdio.h>

#include "hw/pci/pci.h"
#include "net/net.h"
#include "hw/loader.h"
//#include "qemu-timer.h"
#include "sysemu/dma.h"
#include "hw/qdev.h"
#include "qemu/sockets.h"
#include "sysemu/sysemu.h"
#include "memory.h"
#include "qemu/osdep.h"

#include "qemu-common.h"
//#include "qemu-error.h"

#include "ipnoise-common/pcidev.h"

enum hostos_state
{
    HOSTOS_STATE_UNCONNECTED = 1,
    HOSTOS_STATE_LISTEN,
    HOSTOS_STATE_CONNECTING,
    HOSTOS_STATE_CONNECTED,
    HOSTOS_STATE_SHUTDOWN
};

typedef struct
{
    void            *name;      // not mandatory address
    uint32_t        namelen;    // address size
    unsigned char   *data;
    uint32_t        data_size;
} IPNoiseBuff;

typedef struct
{
    int32_t             fd;
    void                *state;
    void                *os_private;
    uint32_t            poll_wevents;       // wait events
    uint32_t            poll_revents;       // return events
    IPNoisePacket       *waiting_packet;    // waiting packet (copy)

    struct msghdr       send;
    int32_t             send_flags;

    IPNoiseBuff         recv;
    int32_t             recv_flags;

    struct list_head    list;
} IPNoiseFdInfo;

typedef struct
{
    // private
    PCIDevice               parent_obj;

    // public
    MemoryRegion            mmio;
    MemoryRegion            mmio_bar;

    // registers
    uint32_t                reg_cr;
    uint32_t                reg_cmd_id;
    uint32_t                reg_cmd_0;
    uint32_t                reg_cmd_1;
    uint32_t                reg_cmd_2;
    uint32_t                reg_cmd_3;
    uint32_t                reg_cmd_4;

    qemu_irq                irq;
    int32_t                 irq_level;

    void                    *pci_dev;

    IPNoisePacket           downlink;
    uint32_t                dma_downlink_addr;
    uint32_t                dma_downlink_size;
    uint32_t                dma_uplink_addr;
    uint32_t                dma_uplink_size;

    QEMUTimer               *poll_timer;
    IPNoiseFdInfo           fds;
} IPNoiseState;

static void *   ipnoise_malloc0(size_t);
static void *   ipnoise_realloc(void *, size_t);
static void     ipnoise_free(void *);

static void     ipnoise_flush_uplink(IPNoiseState *s);
static void     ipnoise_flush_downlink(IPNoiseState *s);
static void     ipnoise_flush(IPNoiseState *s);
static void     ipnoise_read_cb(void *opaque);
static void     ipnoise_write_cb(void *opaque);

static struct iovec * ipnoise_alloc_iov(
    IPNoiseState    *a_state,
    struct msghdr   *a_msg
);

static void ipnoise_hostos_sendmsg(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet
);

static void ipnoise_hostos_recvmsg(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet
);

static int32_t ipnoise_normalize_error(
    IPNoiseState    *a_state,
    int32_t         a_err
);

static int32_t ipnoise_socket_error(
    IPNoiseState    *a_state,
    int32_t         a_res
);

static int32_t ipnoise_lsocket_error(
    IPNoiseState    *a_state,
    int32_t         a_fd
);

extern const VMStateDescription vmstate_ipnoise;
static uint64_t ipnoise_ioport_read(
    void                *a_opaque,
    hwaddr              a_addr,
    unsigned            a_size
);
static void ipnoise_ioport_write(
    void                *a_opaque,
    hwaddr              a_addr,
    uint64_t            a_data,
    unsigned            a_size
);
static IPNoiseFdInfo * ipnoise_get_fd_info(
    IPNoiseState    *a_state,
    int32_t         a_fd
);
static void ipnoise_delete_fd_info(
    IPNoiseState    *a_state,
    IPNoiseFdInfo   *a_fd_info
);
static IPNoiseFdInfo * _ipnoise_get_create_fd_info(
    IPNoiseState    *a_state,
    int32_t         a_fd,
    int32_t         a_do_clear
);

static void _ipnoise_free_all_fds(
    IPNoiseState *a_state
);

static void ipnoise_clear_buff(IPNoiseBuff *a_buff);
static void ipnoise_clear_msg(struct msghdr *a_msg);

void ipnoise_free_waiting_packet(IPNoiseFdInfo *a_fd_info);
void ipnoise_clear_fd_info(IPNoiseFdInfo *a_fd_info);
void ipnoise_clear_poll_revents(
    IPNoiseFdInfo   *a_fd_info,
    uint32_t        a_events
);
void ipnoise_set_poll_revents(
    IPNoiseFdInfo   *a_fd_info,
    uint32_t        a_events
);
void ipnoise_clear_poll_events(
    IPNoiseFdInfo   *a_fd_info,
    uint32_t        a_events
);
void ipnoise_set_poll_events(
    IPNoiseFdInfo   *a_fd_info,
    uint32_t        a_events
);
void ipnoise_clear_poll_wevents(
    IPNoiseFdInfo   *a_fd_info,
    uint32_t        a_events
);
void ipnoise_set_poll_wevents(
    IPNoiseFdInfo   *a_fd_info,
    uint32_t        a_events
);
static void ipnoise_set_waiting_packet(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_packet
);
void ipnoise_setup_waiting(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet
);
void ipnoise_wait_rd_wr(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet
);
void ipnoise_call_rd(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet
);
void ipnoise_call_now(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet
);
void ipnoise_call_wr(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet
);
void ipnoise_wait_rd(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet
);
void ipnoise_wait_wr(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet
);
void ipnoise_no_wait_rd_wr(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet
);
void ipnoise_no_wait_rd(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet
);
void ipnoise_no_wait_wr(
    IPNoiseFdInfo   *a_fd_info,
    IPNoisePacket   *a_waiting_packet
);

static void ipnoise_poll_timer(void *a_opaque);
static void ipnoise_process_command(IPNoiseState *a_state);
//static void ipnoise_cr(
//    IPNoiseState    *a_state,
//    uint32_t        a_value
//);
static void ipnoise_set_cr_flags(
    IPNoiseState    *a_state,
    uint32_t        a_flags
);
static void ipnoise_clear_cr_flags(
    IPNoiseState    *a_state,
    uint32_t        a_flags
);
static void ipnoise_free_packet(
    IPNoisePacket *a_packet
);
static void pci_ipnoise_uninit(PCIDevice *a_dev);
static void ipnoise_s_reset(IPNoiseState *a_state);
static int pci_ipnoise_init(PCIDevice *a_pci_dev);
static void pci_reset(DeviceState *a_dev);
static void ipnoise_class_init(
    ObjectClass *a_class,
    void *a_data
);
static void pci_ipnoise_register_types(void);
void ipnoise_h_reset(void *a_opaque);

void ipnoise_hostos_connect(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet
);

static void ipnoise_uplink_process_packet(
    IPNoiseState    *a_state,
    IPNoisePacket   *a_packet
);

#endif

