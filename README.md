# IPNoise
IPNoise - decentralized network engine for free internet

Main parts:

***** IPNoise QEMU virtual PCI device *****
packages/ipnoise-qemu/qemu-1.7.0/hw/net/ipnoise.c

Involved in communicating between guest OS and host OS
- for sending/receiving packets
- for opening host OS sockets and etc..

***** IPNoise Linux kernel PCI driver for IPNoise QEMU virtual PCI device *****
packages/ipnoise-kernel/linux-3.13.1/drivers/net/ipnoise.c (IPNoise PCI driver + new protocol family PF_HOSTOS)
packages/ipnoise-kernel/linux-3.13.1/drivers/net/ipnoise.h (IPNoise PCI driver + new protocol family PF_HOSTOS)
packages/ipnoise-kernel/linux-3.13.1/include/linux/pci_ids.h (IPNoise PCI vendor and device IDs)
packages/ipnoise-kernel/linux-3.13.1/drivers/net/tun.c (IPNoise virtual network devices support)
packages/ipnoise-kernel/linux-3.13.1/include/uapi/linux/if_tun.h (IPNoise virtual network devices support)
packages/ipnoise-kernel/linux-3.13.1/net/ipv4/arp.c (IPNoise new hw addresses support)
packages/ipnoise-kernel/linux-3.13.1/net/core/rtnetlink.c (IPNoise new hw addresses support)
packages/ipnoise-kernel/linux-3.13.1/net/core/neighbour.c (IPNoise new hw addresses support)

- Communicating between Linux kernel and IPNoise QEMU virtual PCI device
- New sockets protocol family - PF_HOSTOS (for opening host OS sockets, from user space)
