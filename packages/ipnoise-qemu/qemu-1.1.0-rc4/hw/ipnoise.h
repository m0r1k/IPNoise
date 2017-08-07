#ifndef IPNOISE_H
#define IPNOISE_H

#define IPNOISE_IOPORT_SIZE       0x40
#define IPNOISE_PNPMMIO_SIZE      0x20

#include <stdio.h>

#include "pci.h"
#include "net.h"
#include "loader.h"
#include "qemu-timer.h"
#include "dma.h"
#include "qdev.h"
#include "qemu_socket.h"
#include "sysemu.h"
#include "memory.h"
#include "osdep.h"

#include "qemu-common.h"
#include "qemu-error.h"

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
    void                *s;
    void                *os_private;
    uint32_t            poll_wevents;       // wait events
    uint32_t            poll_revents;       // return events
    IPNoisePacket       *waiting_packet;    // waiting packet (copy)

    IPNoiseBuff         send;
    IPNoiseBuff         recv;

    struct list_head    list;
} IPNoiseFdInfo;

typedef struct
{
    uint32_t                reg_cr;
    //
    uint32_t                reg_cmd_id;
    uint32_t                reg_cmd_0;
    uint32_t                reg_cmd_1;
    uint32_t                reg_cmd_2;
    uint32_t                reg_cmd_3;
    uint32_t                reg_cmd_4;
    //
    MemoryRegion            mmio;
    qemu_irq                irq;
    int                     irq_level;
    void                    *pci_dev;
    uint32_t                downlink_dma_addr;
    uint32_t                downlink_dma_size;
    uint32_t                uplink_dma_addr;
    uint32_t                uplink_dma_size;

    QEMUTimer               *poll_timer;
    IPNoisePacket           downlink;
    IPNoiseFdInfo           fds;
} IPNoiseState;

typedef struct
{
    PCIDevice       pci_dev;
    IPNoiseState    state;
    MemoryRegion    io_bar;
} PCIIPNoiseState;

static void ipnoise_flush_uplink(IPNoiseState *s);
static void ipnoise_flush_downlink(IPNoiseState *s);
static void ipnoise_flush(IPNoiseState *s);
static void ipnoise_read_cb(void *opaque);
static void ipnoise_write_cb(void *opaque);

extern const VMStateDescription vmstate_ipnoise;
static uint64_t ipnoise_ioport_read(
    void                *opaque,
    target_phys_addr_t  addr,
    unsigned            size
);
static void ipnoise_ioport_write(
    void                *opaque,
    target_phys_addr_t  addr,
    uint64_t            data,
    unsigned            size
);
static IPNoiseFdInfo * ipnoise_get_fd_info(
    IPNoiseState    *s,
    int32_t         fd
);
static void ipnoise_delete_fd_info(
    IPNoiseState    *s,
    IPNoiseFdInfo   *fd_info
);
static IPNoiseFdInfo * _ipnoise_get_create_fd_info(
    IPNoiseState    *s,
    int32_t         fd,
    int32_t         do_clear
);

void ipnoise_clear_buff(IPNoiseBuff *buff);
void ipnoise_free_waiting_packet(IPNoiseFdInfo *fd_info);
void ipnoise_clear_fd_info(IPNoiseFdInfo *fd_info);
void ipnoise_clear_poll_revents(
    IPNoiseFdInfo   *fd_info,
    uint32_t        events
);
void ipnoise_set_poll_revents(
    IPNoiseFdInfo   *fd_info,
    uint32_t        events
);
void ipnoise_clear_poll_events(
    IPNoiseFdInfo   *fd_info,
    uint32_t        events
);
void ipnoise_set_poll_events(
    IPNoiseFdInfo   *fd_info,
    uint32_t        events
);
void ipnoise_clear_poll_wevents(
    IPNoiseFdInfo   *fd_info,
    uint32_t        events
);
void ipnoise_set_poll_wevents(
    IPNoiseFdInfo   *fd_info,
    uint32_t        events
);
static void ipnoise_set_waiting_packet(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *packet
);
void ipnoise_wait(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet
);
void ipnoise_wait_rd_wr(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet
);
void ipnoise_call_rd(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet
);
void ipnoise_call_wr(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet
);
void ipnoise_wait_rd(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet
);
void ipnoise_wait_wr(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet
);
void ipnoise_no_wait_rd_wr(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet
);
void ipnoise_no_wait_rd(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet
);
void ipnoise_no_wait_wr(
    IPNoiseFdInfo   *fd_info,
    IPNoisePacket   *waiting_packet
);

static void ipnoise_poll_timer(void *opaque);
static void ipnoise_process_command(IPNoiseState *s);
static void ipnoise_cr(
    IPNoiseState    *s,
    uint32_t        value
);
static void ipnoise_set_cr_flags(
    IPNoiseState    *s,
    uint32_t        flags
);
static void ipnoise_clear_cr_flags(
    IPNoiseState    *s,
    uint32_t        flags
);
static int pci_ipnoise_uninit(PCIDevice *dev);
static void ipnoise_s_reset(IPNoiseState *s);
static int pci_ipnoise_init(PCIDevice *pci_dev);
static void pci_reset(DeviceState *dev);
static void ipnoise_class_init(ObjectClass *klass, void *data);
static void pci_ipnoise_register_types(void);
void ipnoise_h_reset(void *opaque);

#endif

