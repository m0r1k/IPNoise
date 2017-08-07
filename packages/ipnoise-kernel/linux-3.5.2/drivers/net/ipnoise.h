#ifndef IPNOISE_KERNEL_DRV_H
#define IPNOISE_KERNEL_DRV_H

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/in.h>
#include <linux/wait.h>
#include <linux/list.h>
#include <net/sock.h>
#include <net/inet_sock.h>
#include <asm/dma.h>
#include <asm/irq.h>

#include "ipnoise-common/pcidev.h"

#define DOWNLINK_DMA_SIZE   1*1024*1024
#define UPLINK_DMA_SIZE     1*1024*1024
#define DMA_TOTAL_SIZE      (DOWNLINK_DMA_SIZE + UPLINK_DMA_SIZE)

#define IPNOISE_DMA_MASK    0xffffffff
#define IPNOISE_TOTAL_SIZE  0x40        // PCI IO ports window size

typedef struct
{
    int             have_pci;
    unsigned char   *dma_buffer;
    dma_addr_t      dma_buffer_addr;
    unsigned char   *dma_downlink;
    unsigned char   *dma_uplink;
    struct pci_dev  *pdev;
    IPNoisePacket   uplink;
    IPNoisePacket   downlink;
} IPNoise;

static int      ipnoise_init_device();
static void     _ipnoise_process_command(IPNoiseRegsInfo *ri_cmd);
static void     ipnoise_process_command(IPNoiseRegsInfo *ri_cmd);
static uint32_t ipnoise_read_reg(u32 reg_name);
static void     ipnoise_dump_queue(const char *descr, IPNoisePacket *queue);
static void     ipnoise_dump_packet(const char *descr, IPNoisePacket *packet);

static int _ipnoise_hostos_socket(
    struct net      *net,
    struct socket   *sock,
    int             protocol,
    int             fd
);

enum {
    HOSTOS_STATE_UNCONNECTED = 0,
    HOSTOS_STATE_LISTEN,
    HOSTOS_STATE_CONNECTING,
    HOSTOS_STATE_CONNECTED,
    HOSTOS_STATE_SHUTDOWN
};

typedef struct
{
    struct sockaddr saddr;
    uint32_t        saddrlen;
    struct sockaddr daddr;
    uint32_t        daddrlen;
    int32_t         fd;
} IPNoisePeer;

typedef struct
{
    IPNoisePacket   *last_packet;
} IPNoisePCIDev;

struct hostos_sock
{
	// inet_sock MUST be the first member
	struct inet_sock    inet;
    IPNoisePeer         peer;
    int32_t             state;
    uint32_t            poll_mask;
    IPNoisePCIDev       pcidev;
};

static inline struct hostos_sock *hostos_sk(const struct sock *sk)
{
	return (struct hostos_sock *)sk;
}

static inline void ipnoise_dump_regs(IPNoiseRegsInfo *ri)
{
    pr_info("ipnoise PCI device registers:\n"
        "cmd_id:        0x'%x'\n"
        "cmd_reg_0:     0x'%x'\n"
        "cmd_reg_1:     0x'%x'\n"
        "cmd_reg_2:     0x'%x'\n"
        "cmd_reg_3:     0x'%x'\n"
        "cmd_reg_4:     0x'%x'\n"
        "\n",
        ri->cmd_id,
        ri->cmd_reg_0,
        ri->cmd_reg_1,
        ri->cmd_reg_2,
        ri->cmd_reg_3,
        ri->cmd_reg_4
    );
}

#endif

