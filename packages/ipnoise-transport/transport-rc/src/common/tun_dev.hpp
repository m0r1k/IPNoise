#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_tun.h>

// stl
#include <string>

// IPNoise kernel support
#include <linux/if_tun.h>
#include <ipnoise-common/ipnoise.h>

using namespace std;

#ifndef TUN_DEV_HPP
#define TUN_DEV_HPP

#include "ipnoise-common/log.h"

int tun_open_common(string &a_dev);
int tun_open(string &a_dev);
int tun_close(int a_fd, const string &a_dev);

int tun_write(int a_fd, const void *a_buf, int a_len);
int tun_read(int a_fd, void *a_buf, int a_len);

int ioctl_prepare(const string &a_dev,  struct ifreq &a_ifr);

int tun_bring_up(const string &a_dev);
int tun_bring_down(const string &a_dev);

int tun_get_hwaddr(const string &a_dev, string &a_addr);
int tun_set_hwaddr(const string &a_dev, const string &a_hwaddr);
int tun_set_mtu(const string &a_dev, int a_mtu);

int tun_get_flags(const string &a_dev, short &a_flags);
int tun_set_flags(const string &a_dev, short a_flags);
int tun_clr_flags(const string &a_dev, short a_flags);
int tun_test_flag(const string &a_dev, short a_flags);

int tun_bring_up(const string &a_dev);
int tun_bring_down(const string &a_dev);

#endif

