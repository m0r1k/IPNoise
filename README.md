# IPNoise
IPNoise - decentralized network engine for free internet

![Main diagram](https://github.com/m0r1k/IPNoise/blob/master/IPNoise.svg)

## Main parts

### IPNoise QEMU virtual PCI device
```
packages/ipnoise-qemu/qemu-1.7.0/hw/net/ipnoise.c (IPNoise virtual PCI device)
packages/ipnoise-qemu/qemu-1.7.0/hw/net/ipnoise.h (IPNoise virtual PCI device)
```
Involved in communicating between guest OS and host OS
- for sending/receiving packets
- for opening host OS sockets and etc..

### IPNoise Linux kernel PCI driver for IPNoise QEMU virtual PCI device
```
packages/ipnoise-kernel/linux-3.13.1/drivers/net/ipnoise.c (IPNoise PCI driver + new protocol family PF_HOSTOS)
packages/ipnoise-kernel/linux-3.13.1/drivers/net/ipnoise.h (IPNoise PCI driver + new protocol family PF_HOSTOS)
packages/ipnoise-kernel/linux-3.13.1/include/linux/pci_ids.h (IPNoise PCI vendor and device IDs)
packages/ipnoise-kernel/linux-3.13.1/drivers/net/tun.c (IPNoise virtual network devices support)
packages/ipnoise-kernel/linux-3.13.1/include/uapi/linux/if_tun.h (IPNoise virtual network devices support)
packages/ipnoise-kernel/linux-3.13.1/net/ipv4/arp.c (IPNoise's hardware addresses support)
packages/ipnoise-kernel/linux-3.13.1/net/core/rtnetlink.c (IPNoise's hardware addresses support)
packages/ipnoise-kernel/linux-3.13.1/net/core/neighbour.c (IPNoise's hardware addresses support)
```
- Communicating between Linux kernel and IPNoise QEMU virtual PCI device
- New sockets protocol family - PF_HOSTOS (for opening host OS sockets, from user space)

### IPNoise iproute2 support
```
packages/ipnoise-iproute2/iproute2-rc/lib/ll_addr.c (IPNoise's hardware addresses support)
packages/ipnoise-iproute2/iproute2-rc/lib/ll_types.c (IPNoise's hardware addresses support)
```

## Build
```
    ## start downloading the Fedora ISO
    mkdir -p /mnt/multimedia/iso
    cd /mnt/multimedia/iso
    wget -c http://ftp.colocall.net/pub/fedora/linux/releases/26/Server/x86_64/iso/Fedora-Server-dvd-x86_64-26-1.5.iso
    cd -

    ## clone IPNoise
    git clone https://github.com/m0r1k/IPNoise.git
    cd IPNoise/packages/

    ## install deps
    make setup

    ## make packages
    make lin32

    ## make qemu
    make qemu-lin64

    ## install all to out
    make install-lin64

    ## prepare virtual machine
    cd ipnoise-builder/
    sudo losetup -f
    sudo mkdir -p /mnt/iso/
    sudo mount -o loop /mnt/multimedia/iso/Fedora-Server-dvd-x86_64-26-1.5.iso /mnt/iso/
    sudo cp -f /mnt/iso/* iso/
    sudo umount /mnt/iso
    make install_os
    ## you can view installation via command: 'vncviewer :0'
    cd ../

    ## after installation you can start virtual machine by command:
    make -C ipnoise-builder start

```

