/*
 *
 *  *** IPNoise ***
 *
 *  IPNoise PCI device for QEMU and Linux kernel
 *
 *  Roman E. Chechnev Jun 2012 (c)
 *  <morik@ipnoise.ru>
 *
 */

#ifndef IPNOISE_PCI_DEV_H
#define IPNOISE_PCI_DEV_H

// #define IPNOISE_DEBUG

#define PCI_VENDOR_ID_IPNOISE            0x2210
#define PCI_DEVICE_ID_HOSTOS             0x2000

// ------------------------------- userspace ----------------------------------

#ifndef __KERNEL__ // user space

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "list.h"

#ifndef min
#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })
#endif

#ifndef max
#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })
#endif

#define pinfo(fmt, ...) \
    fprintf(stderr, fmt, ##__VA_ARGS__)

#define pwarn(fmt, ...) \
    fprintf(stderr, fmt, ##__VA_ARGS__)

#define perr(fmt, ...)                                  \
    do {                                                \
        fprintf(stderr, "%s:%d (%s) ",                  \
            __FILE__,                                   \
            __LINE__,                                   \
            __FUNCTION__                                \
        );                                              \
        fprintf(stderr, fmt, ##__VA_ARGS__);            \
    } while (0)

#define pdebug(level, fmt, ...)                         \
    if (g_debug_level >= level){                        \
        fprintf(stderr, "[ DEBUG %d ] "fmt,             \
            level,                                      \
            ##__VA_ARGS__                               \
        );                                              \
    }

#else // else kernel space

#define pdebug(level, fmt, ...)                         \
    if (g_debug_level >= level){                        \
        pr_err("[ DEBUG %d ] "fmt,                      \
            level,                                      \
            ##__VA_ARGS__                               \
        );                                              \
    }

#define perr(fmt, ...)                                  \
    pr_err("%s:%d (%s) ",                               \
        __FILE__,                                       \
        __LINE__,                                       \
        __FUNCTION__                                    \
    );                                                  \
    pr_err(fmt, ##__VA_ARGS__);

#define pinfo(fmt, ...)                                 \
    pr_err(fmt, ##__VA_ARGS__);

#endif // #ifndef __KERNEL__

// ------------------------------- kernel -------------------------------------

#ifdef __KERNEL__

static inline struct ipnoisehdr *ipnoise_hdr(
    const struct sk_buff *skb)
{
    return (struct ipnoisehdr *)skb_mac_header(skb);
}

#endif // #ifdef __KERNEL__

// ------------------------------- kernel and userspace -----------------------

//#ifndef IPNOISE_DEBUG
//#undef  pdebug
//#define pdebug(fmt, ...)
//#endif

#define IPNOISE_POLLIN      0x0001
#define IPNOISE_POLLPRI     0x0002
#define IPNOISE_POLLOUT     0x0004
#define IPNOISE_POLLERR     0x0008
#define IPNOISE_POLLHUP     0x0010
#define IPNOISE_POLLNVAL    0x0020

// Registers:
enum ipnoise_reg
{
    IPNOISE_REG_CR          = 0x10,   // control register
    IPNOISE_REG_CMD_ID      = 0x14,   // command ID
    IPNOISE_REG_CMD_0       = 0x18,   // command register 0
    IPNOISE_REG_CMD_1       = 0x2c,   // command register 1
    IPNOISE_REG_CMD_2       = 0x30,   // command register 2
    IPNOISE_REG_CMD_3       = 0x34,   // command register 3
    IPNOISE_REG_CMD_4       = 0x38,   // command register 4
    IPNOISE_REG_CMD_LAST    = 0x38    // command register 5
};

// Commands:
enum ipnoise_cmd_id
{
    IPNOISE_CMD_RESET                   = 1,
    IPNOISE_CMD_SET_ULDD                = 2,
    IPNOISE_CMD_CLEAR_DLDD              = 3,
    IPNOISE_CMD_SET_IRQ_LEVEL           = 4,
    IPNOISE_CMD_SET_DOWNLINK_DMA_ADDR   = 5,
    IPNOISE_CMD_SET_DOWNLINK_DMA_SIZE   = 6,
    IPNOISE_CMD_SET_UPLINK_DMA_ADDR     = 7,
    IPNOISE_CMD_SET_UPLINK_DMA_SIZE     = 8,
    IPNOISE_CMD_SET_DEBUG_LEVEL         = 9,
    IPNOISE_CMD_GET_DEBUG_LEVEL         = 10,
    IPNOISE_CMD_SET_POLL_EVENTS         = 11
};

// Control Register
enum ipnoise_cr_flags
{
    IPNOISE_CR_SELF_TEST_PASSED     = 1 << 31,
    IPNOISE_CR_DOWNLINK_DMA_READY   = 1 << 30,
    IPNOISE_CR_UPLINK_DMA_READY     = 1 << 29,
    IPNOISE_CR_ULDD                 = 1 << 3,
    IPNOISE_CR_DLDD                 = 1 << 2,
    IPNOISE_CR_EVENTS_ENABLED       = 1 << 1
};

// Packet type
typedef enum
{
    IPNOISE_PACKET_FREE = 0,
    IPNOISE_PACKET_OPEN_SOCKET,
    IPNOISE_PACKET_CLOSE,
    IPNOISE_PACKET_CONNECT,
    IPNOISE_PACKET_SENDMSG,
    IPNOISE_PACKET_RECVMSG,
    IPNOISE_PACKET_BIND,
    IPNOISE_PACKET_LISTEN,
    IPNOISE_PACKET_ACCEPT,
    IPNOISE_PACKET_POLL,
    IPNOISE_PACKET_SOCKOPT_SET,
    IPNOISE_PACKET_SOCKOPT_GET
} IPNoisePacketType;

// msg type
typedef enum
{
    IPNOISE_MSG_TYPE_FLAGS = 1,
    IPNOISE_MSG_TYPE_NAME,
    IPNOISE_MSG_TYPE_IOV,
    IPNOISE_MSG_TYPE_END
} SendMsgType;

// Packet
typedef struct
{
    IPNoisePacketType   type;
    int32_t             fd;
    unsigned char       *data;
    uint32_t            data_size;
    void                *os_private;
    int32_t             non_block;
    int32_t             poll_revents;
    int32_t             last_sock_err;
    int32_t             res;
    union {
        struct {
            uint32_t    pf_family;
            uint32_t    type;
            uint32_t    protocol;
        } open_socket;
        struct {
        } close;
        struct {
            struct sockaddr daddr;
            int32_t         daddrlen;
        } connect;
        struct {
            uint32_t        type;
            union {
                struct {
                } msg_flags;
                struct {
                } msg_name;
                struct {
                    int32_t id;
                } msg_iov;
                struct {
                } msg_end;
            } un;
        } sendmsg;
        struct {
            uint32_t        type;
            union {
                struct {
                } msg_flags;
                struct {
                } msg_name;
                struct {
                    uint32_t    iov_len;
                } msg_iov;
                struct {
                    struct sockaddr addr;
                    int32_t         addrlen;
                } msg_end;
            } un;
        } recvmsg;
        struct {
            struct sockaddr addr;
            int32_t         addrlen;
        } bind;
        struct {
            int32_t         backlog;
        } listen;
        struct {
            void            *os_private;
            struct sockaddr addr;
            uint32_t        addrlen;
        } accept;
        struct {
            uint32_t        events;
        } poll;
        struct {
            int32_t         level;
            int32_t         optname;
            int32_t         optlen;
            // for sockopt_get data will contain optval
            // data_size will contain optval size
        } sockopt;
    } un;
    struct list_head    list;
} IPNoisePacket;

typedef struct
{
    uint32_t    cmd_id;
    uint32_t    cmd_reg_0;
    uint32_t    cmd_reg_1;
    uint32_t    cmd_reg_2;
    uint32_t    cmd_reg_3;
    uint32_t    cmd_reg_4;
} IPNoiseRegsInfo;

// ------------------------------- kernel and userspace utils -----------------

inline static void CLEAR_REG_INFO(IPNoiseRegsInfo *ri)
{
    memset(ri, 0x00, sizeof(*ri));
}

inline static void ipnoise_dump_packet(
    const char      *descr,
    IPNoisePacket   *packet)
{
    if (descr){
        pinfo("%s {\n", descr);
    } else {
        pinfo("packet {\n");
    }

    switch(packet->type){
        case IPNOISE_PACKET_FREE:
            pinfo(
                "fd:            '%d'\n"
                "type:          '%d' (packet free)\n"
                "res:           '%d'\n"
                "data:          0x'%x'\n"
                "data_size:     '%u'\n"
                "os_private:    0x'%x'\n"
                "poll_revents:  0x'%x'\n"
                "last_sock_err: '%d'\n"
                "non_block:     '%d'\n"
                "---\n",
                packet->fd,
                packet->type,
                packet->res,
                (unsigned int)packet->data,
                packet->data_size,
                (unsigned int)packet->os_private,
                packet->poll_revents,
                packet->last_sock_err,
                packet->non_block
            );
            break;

        case IPNOISE_PACKET_OPEN_SOCKET:
            pinfo(
                "fd:            '%d'\n"
                "type:          '%d' (open socket)\n"
                "res:           '%d'\n"
                "data:          0x'%x'\n"
                "data_size:     '%u'\n"
                "os_private:    0x'%x'\n"
                "poll_revents:  0x'%x'\n"
                "last_sock_err: '%d'\n"
                "non_block:     '%d'\n"
                "---\n"
                "type:          '%d'\n"
                "pf_family:     '%d'\n"
                "protocol:      '%d'\n",
                packet->fd,
                packet->type,
                packet->res,
                (unsigned int)packet->data,
                packet->data_size,
                (unsigned int)packet->os_private,
                packet->poll_revents,
                packet->last_sock_err,
                packet->non_block,
                packet->un.open_socket.type,
                packet->un.open_socket.pf_family,
                packet->un.open_socket.protocol
            );
            break;

        case IPNOISE_PACKET_CLOSE:
            pinfo(
                "fd:            '%d'\n"
                "type:          '%d' (close)\n"
                "res:           '%d'\n"
                "data:          0x'%x'\n"
                "data_size:     '%u'\n"
                "os_private:    0x'%x'\n"
                "poll_revents:  0x'%x'\n"
                "last_sock_err: '%d'\n"
                "non_block:     '%d'\n"
                "---\n",
                packet->fd,
                packet->type,
                packet->res,
                (unsigned int)packet->data,
                packet->data_size,
                (unsigned int)packet->os_private,
                packet->poll_revents,
                packet->last_sock_err,
                packet->non_block
            );
            break;

        case IPNOISE_PACKET_CONNECT:
            if (PF_INET == packet->un.connect.daddr.sa_family){
                struct sockaddr_in  *daddr      = NULL;
                int32_t             sin_port    = 0;
                uint32_t            sin_addr    = 0;

                daddr       = (struct sockaddr_in *)&packet->un.connect.daddr;
                sin_port    = ntohs(daddr->sin_port);
                sin_addr    = ntohl(daddr->sin_addr.s_addr);

                pinfo(
                    "fd:                '%d'\n"
                    "type:              '%d' (connect)\n"
                    "res:               '%d'\n"
                    "data:              0x'%x'\n"
                    "data_size:         '%u'\n"
                    "os_private:        0x'%x'\n"
                    "poll_revents:      0x'%x'\n"
                    "last_sock_err:     '%d'\n"
                    "non_block:         '%d'\n"
                    "---\n"
                    "sin_family:        '%d'\n"
                    "sin_port:          '%d'\n"
                    "sin_addr.d_addr:   %d.%d.%d.%d\n",
                    packet->fd,
                    packet->type,
                    packet->res,
                    (unsigned int)packet->data,
                    packet->data_size,
                    (unsigned int)packet->os_private,
                    packet->poll_revents,
                    packet->last_sock_err,
                    packet->non_block,
                    daddr->sin_family,
                    sin_port,
                    (sin_addr & 0xff000000) >> 24,
                    (sin_addr & 0x00ff0000) >> 16,
                    (sin_addr & 0x0000ff00) >> 8,
                    (sin_addr & 0x000000ff) >> 0
                );
            } else {
                pinfo(
                    "fd:                '%d'\n"
                    "type:              '%d' (connect)\n"
                    "res:               '%d'\n"
                    "data:              0x'%x'\n"
                    "data_size:         '%u'\n"
                    "os_private:        0x'%x'\n"
                    "poll_revents:      0x'%x'\n"
                    "last_sock_err:     '%d'\n"
                    "non_block:         '%d'\n"
                    "---\n"
                    "sa_family:         '%d'\n",
                    packet->fd,
                    packet->type,
                    packet->res,
                    (unsigned int)packet->data,
                    packet->data_size,
                    (unsigned int)packet->os_private,
                    packet->poll_revents,
                    packet->last_sock_err,
                    packet->non_block,
                    packet->un.connect.daddr.sa_family
                );
            }
            break;

        case IPNOISE_PACKET_SENDMSG:
            {
                uint32_t type = packet->un.sendmsg.type;

                switch(type){
                    case IPNOISE_MSG_TYPE_NAME:
                        pinfo(
                            "fd:            '%d'\n"
                            "type:          '%d' (sendmsg)\n"
                            "res:           '%d'\n"
                            "data:          0x'%x'\n"
                            "data_size:     '%u'\n"
                            "os_private:    0x'%x'\n"
                            "poll_revents:  0x'%x'\n"
                            "last_sock_err: '%d'\n"
                            "non_block:     '%d'\n"
                            "---\n"
                            "type:          '%d' (msg name)\n",
                            packet->fd,
                            packet->type,
                            packet->res,
                            (unsigned int)packet->data,
                            packet->data_size,
                            (unsigned int)packet->os_private,
                            packet->poll_revents,
                            packet->last_sock_err,
                            packet->non_block,
                            type
                        );
                        break;

                    case IPNOISE_MSG_TYPE_IOV:
                        pinfo(
                            "fd:            '%d'\n"
                            "type:          '%d' (sendmsg)\n"
                            "res:           '%d'\n"
                            "data:          0x'%x'\n"
                            "data_size:     '%u'\n"
                            "os_private:    0x'%x'\n"
                            "poll_revents:  0x'%x'\n"
                            "last_sock_err: '%d'\n"
                            "non_block:     '%d'\n"
                            "---\n"
                            "type:          '%d' (msg iov)\n"
                            "iov_id:        '%d'\n",
                            packet->fd,
                            packet->type,
                            packet->res,
                            (unsigned int)packet->data,
                            packet->data_size,
                            (unsigned int)packet->os_private,
                            packet->poll_revents,
                            packet->last_sock_err,
                            packet->non_block,
                            type,
                            packet->un.sendmsg.un.msg_iov.id
                        );
                        break;

                    case IPNOISE_MSG_TYPE_END:
                        pinfo(
                            "fd:            '%d'\n"
                            "type:          '%d' (sendmsg)\n"
                            "res:           '%d'\n"
                            "data:          0x'%x'\n"
                            "data_size:     '%u'\n"
                            "os_private:    0x'%x'\n"
                            "poll_revents:  0x'%x'\n"
                            "last_sock_err: '%d'\n"
                            "non_block:     '%d'\n"
                            "---\n"
                            "type:          '%d' (msg end)\n",
                            packet->fd,
                            packet->type,
                            packet->res,
                            (uint32_t)packet->data,
                            packet->data_size,
                            (uint32_t)packet->os_private,
                            packet->poll_revents,
                            packet->last_sock_err,
                            packet->non_block,
                            type
                        );
                        break;

                    default:
                        pinfo(
                            "fd:            '%d'\n"
                            "type:          '%d' (sendmsg)\n"
                            "res:           '%d'\n"
                            "data:          0x'%x'\n"
                            "data_size:     '%u'\n"
                            "os_private:    0x'%x'\n"
                            "poll_revents:  0x'%x'\n"
                            "last_sock_err: '%d'\n"
                            "non_block:     '%d'\n"
                            "---\n"
                            "type:          '%d' (msg unknown)\n",
                            packet->fd,
                            packet->type,
                            packet->res,
                            (unsigned int)packet->data,
                            packet->data_size,
                            (unsigned int)packet->os_private,
                            packet->poll_revents,
                            packet->last_sock_err,
                            packet->non_block,
                            type
                        );
                        break;
                }
                break;
            }

        case IPNOISE_PACKET_RECVMSG:
            {
                uint32_t type = packet->un.sendmsg.type;

                switch(type){
                    case IPNOISE_MSG_TYPE_FLAGS:
                        pinfo(
                            "fd:            '%d'\n"
                            "type:          '%d' (recvmsg)\n"
                            "res:           '%d'\n"
                            "data:          0x'%x'\n"
                            "data_size:     '%u'\n"
                            "os_private:    0x'%x'\n"
                            "poll_revents:  0x'%x'\n"
                            "last_sock_err: '%d'\n"
                            "non_block:     '%d'\n"
                            "---\n"
                            "type:          '%d' (msg flags)\n",
                            packet->fd,
                            packet->type,
                            packet->res,
                            (unsigned int)packet->data,
                            packet->data_size,
                            (unsigned int)packet->os_private,
                            packet->poll_revents,
                            packet->last_sock_err,
                            packet->non_block,
                            type
                        );
                        break;

                    case IPNOISE_MSG_TYPE_NAME:
                        pinfo(
                            "fd:            '%d'\n"
                            "type:          '%d' (recvmsg)\n"
                            "res:           '%d'\n"
                            "data:          0x'%x'\n"
                            "data_size:     '%u'\n"
                            "os_private:    0x'%x'\n"
                            "poll_revents:  0x'%x'\n"
                            "last_sock_err: '%d'\n"
                            "non_block:     '%d'\n"
                            "---\n"
                            "type:          '%d' (msg name)\n",
                            packet->fd,
                            packet->type,
                            packet->res,
                            (unsigned int)packet->data,
                            packet->data_size,
                            (unsigned int)packet->os_private,
                            packet->poll_revents,
                            packet->last_sock_err,
                            packet->non_block,
                            type
                        );
                        break;

                    case IPNOISE_MSG_TYPE_IOV:
                        pinfo(
                            "fd:            '%d'\n"
                            "type:          '%d' (recvmsg)\n"
                            "res:           '%d'\n"
                            "data:          0x'%x'\n"
                            "data_size:     '%u'\n"
                            "os_private:    0x'%x'\n"
                            "poll_revents:  0x'%x'\n"
                            "last_sock_err: '%d'\n"
                            "non_block:     '%d'\n"
                            "---\n"
                            "type:          '%d' (msg iov)\n"
                            "iov_len:       '%d'\n",
                            packet->fd,
                            packet->type,
                            packet->res,
                            (unsigned int)packet->data,
                            packet->data_size,
                            (unsigned int)packet->os_private,
                            packet->poll_revents,
                            packet->last_sock_err,
                            packet->non_block,
                            type,
                            packet->un.recvmsg.un.msg_iov.iov_len
                        );
                        break;

                    case IPNOISE_MSG_TYPE_END:
                        if (PF_INET == packet->un.recvmsg
                            .un.msg_end
                            .addr.sa_family)
                        {
                            struct sockaddr_in  *addr       = NULL;
                            int32_t             sin_port    = 0;
                            uint32_t            sin_addr    = 0;

                            addr = (struct sockaddr_in *)
                                &packet->un.recvmsg
                                    .un.msg_end
                                    .addr;

                            sin_port = ntohs(addr->sin_port);
                            sin_addr = ntohl(addr->sin_addr.s_addr);

                            pinfo(
                                "fd:                '%d'\n"
                                "type:              '%d' (recvmsg)\n"
                                "res:               '%d'\n"
                                "data:              0x'%x'\n"
                                "data_size:         '%u'\n"
                                "os_private:        0x'%x'\n"
                                "poll_revents:      0x'%x'\n"
                                "last_sock_err:     '%d'\n"
                                "non_block:         '%d'\n"
                                "---\n"
                                "type:              '%d' (msg end)\n"
                                "addrlen:           '%d'\n"
                                "sin_family:        '%d'\n"
                                "sin_port:          '%d'\n"
                                "sin_addr.s_addr:   %d.%d.%d.%d\n",
                                packet->fd,
                                packet->type,
                                packet->res,
                                (unsigned int)packet->data,
                                packet->data_size,
                                (unsigned int)packet->os_private,
                                packet->poll_revents,
                                packet->last_sock_err,
                                packet->non_block,
                                type,
                                packet->un.recvmsg.un.msg_end.addrlen,
                                addr->sin_family,
                                sin_port,
                                (sin_addr & 0xff000000) >> 24,
                                (sin_addr & 0x00ff0000) >> 16,
                                (sin_addr & 0x0000ff00) >> 8,
                                (sin_addr & 0x000000ff) >> 0
                            );
                        } else {
                            pinfo(
                                "fd:                '%d'\n"
                                "type:              '%d' (recvmsg)\n"
                                "res:               '%d'\n"
                                "data:              0x'%x'\n"
                                "data_size:         '%u'\n"
                                "os_private:        0x'%x'\n"
                                "poll_revents:      0x'%x'\n"
                                "last_sock_err:     '%d'\n"
                                "non_block:         '%d'\n"
                                "---\n"
                                "type:              '%d' (msg end)\n"
                                "addrlen:           '%d'\n"
                                "sa_family:         '%d'\n",
                                packet->fd,
                                packet->type,
                                packet->res,
                                (unsigned int)packet->data,
                                packet->data_size,
                                (unsigned int)packet->os_private,
                                packet->poll_revents,
                                packet->last_sock_err,
                                packet->non_block,
                                type,
                                packet->un.recvmsg.un.msg_end.addrlen,
                                packet->un.recvmsg
                                    .un.msg_end
                                    .addr.sa_family
                            );
                        }
                        break;

                    default:
                        pinfo(
                            "fd:            '%d'\n"
                            "type:          '%d' (recvmsg)\n"
                            "res:           '%d'\n"
                            "data:          0x'%x'\n"
                            "data_size:     '%u'\n"
                            "os_private:    0x'%x'\n"
                            "poll_revents:  0x'%x'\n"
                            "last_sock_err: '%d'\n"
                            "non_block:     '%d'\n"
                            "---\n"
                            "type:          '%d' (msg unknown)\n",
                            packet->fd,
                            packet->type,
                            packet->res,
                            (unsigned int)packet->data,
                            packet->data_size,
                            (unsigned int)packet->os_private,
                            packet->poll_revents,
                            packet->last_sock_err,
                            packet->non_block,
                            type
                        );
                        break;
                }
                break;
            }

        case IPNOISE_PACKET_BIND:
            if (PF_INET == packet->un.bind.addr.sa_family){
                struct sockaddr_in  *addr      = NULL;
                int32_t             sin_port   = 0;
                uint32_t            sin_addr   = 0;

                addr = (struct sockaddr_in *)&packet->un.bind.addr;
                sin_port = ntohs(addr->sin_port);
                sin_addr = ntohl(addr->sin_addr.s_addr);

                pinfo(
                    "fd:                '%d'\n"
                    "type:              '%d' (bind)\n"
                    "res:               '%d'\n"
                    "data:              0x'%x'\n"
                    "data_size:         '%u'\n"
                    "os_private:        0x'%x'\n"
                    "poll_revents:      0x'%x'\n"
                    "last_sock_err:     '%d'\n"
                    "non_block:         '%d'\n"
                    "---\n"
                    "sin_family:        '%d'\n"
                    "sin_port:          '%d'\n"
                    "sin_addr.s_addr:   %d.%d.%d.%d\n",
                    packet->fd,
                    packet->type,
                    packet->res,
                    (unsigned int)packet->data,
                    packet->data_size,
                    (unsigned int)packet->os_private,
                    packet->poll_revents,
                    packet->last_sock_err,
                    packet->non_block,
                    addr->sin_family,
                    sin_port,
                    (sin_addr & 0xff000000) >> 24,
                    (sin_addr & 0x00ff0000) >> 16,
                    (sin_addr & 0x0000ff00) >> 8,
                    (sin_addr & 0x000000ff) >> 0
                );
            } else {
                pinfo(
                    "fd:                '%d'\n"
                    "type:              '%d' (bind)\n"
                    "res:               '%d'\n"
                    "data:              0x'%x'\n"
                    "data_size:         '%u'\n"
                    "os_private:        0x'%x'\n"
                    "poll_revents:      0x'%x'\n"
                    "last_sock_err:     '%d'\n"
                    "non_block:         '%d'\n"
                    "---\n"
                    "sa_family:         '%d'\n",
                    packet->fd,
                    packet->type,
                    packet->res,
                    (unsigned int)packet->data,
                    packet->data_size,
                    (unsigned int)packet->os_private,
                    packet->poll_revents,
                    packet->last_sock_err,
                    packet->non_block,
                    packet->un.bind.addr.sa_family
                );
            }
            break;

        case IPNOISE_PACKET_LISTEN:
            {
                pinfo(
                    "fd:                '%d'\n"
                    "type:              '%d' (listen)\n"
                    "res:               '%d'\n"
                    "data:              0x'%x'\n"
                    "data_size:         '%u'\n"
                    "os_private:        0x'%x'\n"
                    "poll_revents:      0x'%x'\n"
                    "last_sock_err:     '%d'\n"
                    "non_block:         '%d'\n"
                    "---\n"
                    "backlog:           '%d'\n",
                    packet->fd,
                    packet->type,
                    packet->res,
                    (unsigned int)packet->data,
                    packet->data_size,
                    (unsigned int)packet->os_private,
                    packet->poll_revents,
                    packet->last_sock_err,
                    packet->non_block,
                    packet->un.listen.backlog
                );
            }
            break;

        case IPNOISE_PACKET_ACCEPT:
            if (PF_INET == packet->un.accept.addr.sa_family){
                struct sockaddr_in  *addr       = NULL;
                int32_t             sin_port    = 0;
                uint32_t            sin_addr    = 0;

                addr        = (struct sockaddr_in *)&packet->un.accept.addr;
                sin_port    = ntohs(addr->sin_port);
                sin_addr    = ntohl(addr->sin_addr.s_addr);

                pinfo(
                    "fd:                '%d'\n"
                    "type:              '%d' (accept)\n"
                    "res:               '%d'\n"
                    "data:              0x'%x'\n"
                    "data_size:         '%u'\n"
                    "os_private:        0x'%x'\n"
                    "poll_revents:      0x'%x'\n"
                    "last_sock_err:     '%d'\n"
                    "non_block:         '%d'\n"
                    "---\n"
                    "sin_family:        '%d'\n"
                    "sin_port:          '%d'\n"
                    "sin_addr.s_addr:   %d.%d.%d.%d\n",
                    packet->fd,
                    packet->type,
                    packet->res,
                    (unsigned int)packet->data,
                    packet->data_size,
                    (unsigned int)packet->os_private,
                    packet->poll_revents,
                    packet->last_sock_err,
                    packet->non_block,
                    addr->sin_family,
                    sin_port,
                    (sin_addr & 0xff000000) >> 24,
                    (sin_addr & 0x00ff0000) >> 16,
                    (sin_addr & 0x0000ff00) >> 8,
                    (sin_addr & 0x000000ff) >> 0
                );
            } else {
                pinfo(
                    "fd:                '%d'\n"
                    "type:              '%d' (accept)\n"
                    "res:               '%d'\n"
                    "data:              0x'%x'\n"
                    "data_size:         '%u'\n"
                    "os_private:        0x'%x'\n"
                    "poll_revents:      0x'%x'\n"
                    "last_sock_err:     '%d'\n"
                    "non_block:         '%d'\n"
                    "---\n"
                    "sa_family:         '%d'\n",
                    packet->fd,
                    packet->type,
                    packet->res,
                    (unsigned int)packet->data,
                    packet->data_size,
                    (unsigned int)packet->os_private,
                    packet->poll_revents,
                    packet->last_sock_err,
                    packet->non_block,
                    packet->un.accept.addr.sa_family
                );
            }
            break;

        case IPNOISE_PACKET_POLL:
            {
                pinfo(
                    "fd:                '%d'\n"
                    "type:              '%d' (poll)\n"
                    "res:               '%d'\n"
                    "data:              0x'%x'\n"
                    "data_size:         '%u'\n"
                    "os_private:        0x'%x'\n"
                    "poll_revents:      0x'%x'\n"
                    "last_sock_err:     '%d'\n"
                    "non_block:         '%d'\n"
                    "---\n"
                    "events:            0x'%x'\n",
                    packet->fd,
                    packet->type,
                    packet->res,
                    (unsigned int)packet->data,
                    packet->data_size,
                    (unsigned int)packet->os_private,
                    packet->poll_revents,
                    packet->last_sock_err,
                    packet->non_block,
                    (unsigned int)packet->un.poll.events
                );
            }
            break;

        case IPNOISE_PACKET_SOCKOPT_GET:
            {
                pinfo(
                    "fd:                '%d'\n"
                    "type:              '%d' (sockopt_get)\n"
                    "res:               '%d'\n"
                    "data:              0x'%x'\n"
                    "data_size:         '%u'\n"
                    "os_private:        0x'%x'\n"
                    "poll_revents:      0x'%x'\n"
                    "last_sock_err:     '%d'\n"
                    "non_block:         '%d'\n"
                    "---\n"
                    "level:             '%d'\n"
                    "optname:           '%d'\n"
                    "optlen:            '%d'\n",
                    packet->fd,
                    packet->type,
                    packet->res,
                    (unsigned int)packet->data,
                    packet->data_size,
                    (unsigned int)packet->os_private,
                    packet->poll_revents,
                    packet->last_sock_err,
                    packet->non_block,
                    packet->un.sockopt.level,
                    packet->un.sockopt.optname,
                    packet->un.sockopt.optlen
                );
            }
            break;

        case IPNOISE_PACKET_SOCKOPT_SET:
            {
                pinfo(
                    "fd:                '%d'\n"
                    "type:              '%d' (sockopt_set)\n"
                    "res:               '%d'\n"
                    "data:              0x'%x'\n"
                    "data_size:         '%u'\n"
                    "os_private:        0x'%x'\n"
                    "poll_revents:      0x'%x'\n"
                    "last_sock_err:     '%d'\n"
                    "non_block:         '%d'\n"
                    "---\n"
                    "level:             '%d'\n"
                    "optname:           '%d'\n"
                    "optlen:            '%d'\n",
                    packet->fd,
                    packet->type,
                    packet->res,
                    (unsigned int)packet->data,
                    packet->data_size,
                    (unsigned int)packet->os_private,
                    packet->poll_revents,
                    packet->last_sock_err,
                    packet->non_block,
                    packet->un.sockopt.level,
                    packet->un.sockopt.optname,
                    packet->un.sockopt.optlen
                );
            }
            break;

        default:
            pinfo(
                "fd:            '%d'\n"
                "type:          '%d' (unknown)\n"
                "res:           '%d'\n"
                "data:          0x'%x'\n"
                "data_size:     '%u'\n"
                "os_private:    0x'%x'\n"
                "poll_revents:  0x'%x'\n"
                "last_sock_err: '%d'\n"
                "non_block:     '%d'\n"
                "---\n",
                packet->fd,
                packet->type,
                packet->res,
                (unsigned int)packet->data,
                packet->data_size,
                (unsigned int)packet->os_private,
                packet->poll_revents,
                packet->last_sock_err,
                packet->non_block
            );
            break;
    }

    if (descr){
        pinfo("%s }\n", descr);
    } else {
        pinfo("packet }\n");
    }
}

inline static void ipnoise_dump_queue(
    const char      *descr,
    IPNoisePacket   *queue)
{
    struct list_head *this, *tmp, *head;

    if (descr){
        pinfo("%s {\n", descr);
    } else {
        pinfo("queue {\n");
    }

    head = &queue->list;

    list_for_each_safe(this, tmp, head){
        IPNoisePacket *packet = list_entry(this, IPNoisePacket, list);
        ipnoise_dump_packet(NULL, packet);
    }

    if (descr){
        pinfo("%s }\n", descr);
    } else {
        pinfo("queue }\n");
    }
}

#endif

