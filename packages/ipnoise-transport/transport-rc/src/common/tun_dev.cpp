#include "tun_dev.hpp"

/*
#ifndef OTUNSETNOCSUM
// pre 2.4.6 compatibility
#define OTUNSETNOCSUM    (('T'<< 8) | 200)
#define OTUNSETDEBUG    (('T'<< 8) | 201)
#define OTUNSETIFF    (('T'<< 8) | 202)
#define OTUNSETPERSIST    (('T'<< 8) | 203)
#define OTUNSETOWNER    (('T'<< 8) | 204)
#endif
*/

int tun_open_common(string &a_dev)
{
    char            buffer[1024] = { 0x00 };
    struct ifreq    ifr;
    int             res, fd = -1;

    fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0){
        goto failed;
    }

    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = IFF_IPNOISE | IFF_NO_PI;

    // setup interface name if exist
    if (not a_dev.empty()){
        strncpy(ifr.ifr_name, a_dev.c_str(), IFNAMSIZ);
    }

    res = ioctl(fd, TUNSETIFF, (void *) &ifr);
    if (res < 0){
        PERROR("ioctl TUNSETIFF failed\n");
        goto failed;
    }

    // store interface name
    strncpy(buffer, ifr.ifr_name, sizeof(buffer));
    a_dev = buffer;

out:
    return fd;

failed:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    goto out;
}

int tun_open(string &a_dev)
{
    return tun_open_common(a_dev);
}

int tun_close(
    int             a_fd,
    const string    &) // a_dev
{
    return close(a_fd);
}

// write frames to TUN device
int tun_write(
    int         a_fd,
    const void  *a_buf,
    int         a_len)
{
    return write(a_fd, a_buf, a_len);
}

// read frames from TUN device
int tun_read(
    int         a_fd,
    void        *a_buf,
    int         a_len)
{
    return read(a_fd, a_buf, a_len);
}

// prepare ioctl command
int ioctl_prepare(
    const string    &a_dev,
    struct ifreq    &a_ifr)
{
    int fd = -1;

    memset(&a_ifr, 0, sizeof(a_ifr));

    if (a_dev.empty()){
        PERROR("Cannot get hw address, empty device name\n");
        goto fail;
    }

    fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (fd < 0){
        PERROR("Cannot create new socket(PF_INET, SOCK_DGRAM, 0)\n");
        goto fail;
    }

    a_ifr.ifr_hwaddr.sa_family = ARPHRD_IPNOISE;
    strncpy(a_ifr.ifr_name, a_dev.c_str(), IFNAMSIZ);

out:
    return fd;

fail:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    goto out;
}

// Get interface hw address
int tun_get_hwaddr(
    const string    &a_dev,
    string          &a_addr)
{
    char            buffer[1024] = { 0x00 };
    struct ifreq    ifr;
    int             res;
    int             fd  = -1;
    int             err = -1;

    // prepare ioctl
    fd = ioctl_prepare(a_dev, ifr);
    if (fd < 0){
        PERROR("Cannot prepare ioctl for device '%s'\n", a_dev.c_str());
        goto fail;
    }

    // request hw addr
    res = ioctl(fd, SIOCGIFHWADDR, &ifr);
    if (res){
        PERROR("Cannot get hw addr, ioctl SIOCGIFHWADDR failed\n");
        goto fail;
    }

    // store address
    strncpy(buffer, (char *)&ifr.ifr_hwaddr, sizeof(buffer));
    a_addr = buffer;

    // all ok
    err = 0;

out:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// Set interface hw adress
int tun_set_hwaddr(
    const string    &a_dev,
    const string    &a_hwaddr)
{
    char            *hw_ptr = NULL;
    int             fd      = -1;
    int             err     = -1;
    struct ifreq    ifr;
    int             res;

    // prepare ioctl
    fd = ioctl_prepare(a_dev, ifr);
    if (fd < 0){
        PERROR("Cannot prepare ioctl for device '%s'\n", a_dev.c_str());
        goto fail;
    }

    // fill hw addr
    hw_ptr = (char *)&ifr.ifr_hwaddr.sa_data;
    strncpy(hw_ptr, a_hwaddr.c_str(), sizeof(ifr.ifr_hwaddr.sa_data));

    // do ioctl
    res = ioctl(fd, SIOCSIFHWADDR, &ifr);
    if (res < 0){
        PERROR("Cannot set hw addr, ioctl SIOCSIFHWADDR failed\n");
        goto fail;
    }

    // all ok
    err = 0;

out:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// interface mtu
int tun_set_mtu(
    const string    &a_dev,
    int             a_mtu)
{
    int             fd      = -1;
    int             err     = -1;
    struct ifreq    ifr;
    int             res;

    // prepare ioctl
    fd = ioctl_prepare(a_dev, ifr);
    if (fd < 0){
        PERROR("Cannot prepare ioctl for device '%s'\n", a_dev.c_str());
        goto fail;
    }

    // fill mtu
    ifr.ifr_mtu = a_mtu;

    // call the IOCTL
    res = ioctl(fd, SIOCSIFMTU, &ifr);
    if (res < 0){
        PERROR("Cannot set MTU, ioctl SIOCSIFMTU failed\n");
        goto fail;
    }

    // all ok
    err = 0;

out:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// get interface flags
int tun_get_flags(const string &a_dev, short &a_flags)
{
    int             fd      = -1;
    int             err     = -1;
    struct ifreq    ifr;
    int             res;

    // prepare ioctl
    fd = ioctl_prepare(a_dev, ifr);
    if (fd < 0){
        PERROR("Cannot prepare ioctl for device '%s'\n", a_dev.c_str());
        goto fail;
    }

    // get interface flags
    res = ioctl(fd, SIOCGIFFLAGS, &ifr);
    if (res < 0){
        PERROR("Cannot get interface: '%s' flags,"
            " ioctl SIOCGIFFLAGS failed\n",
            a_dev.c_str()
        );
        goto fail;
    }

    // return flags
    a_flags = ifr.ifr_flags;

    // all ok
    err = 0;

out:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// set interface flags
int tun_set_flags(const string &a_dev, short flags)
{
    int             fd      = -1;
    int             err     = -1;
    struct ifreq    ifr;
    int             res;

    // prepare ioctl
    fd = ioctl_prepare(a_dev, ifr);
    if (fd < 0){
        PERROR("Cannot prepare ioctl for device '%s'\n", a_dev.c_str());
        goto fail;
    }

    // get interface flags
    res = tun_get_flags(a_dev, ifr.ifr_flags);
    if (res < 0){
        PERROR("Cannot get interface: '%s' flags\n", a_dev.c_str());
        goto fail;
    }

    // update flags
    ifr.ifr_flags |= flags;

    // set interface flags
    res = ioctl(fd, SIOCSIFFLAGS, &ifr);
    if (res < 0){
        PERROR("Cannot set interface: '%s' (fd: %d) flags"
            ", ioctl SIOCSIFFLAGS, failed\n",
            a_dev.c_str(),
            fd
        );
        goto fail;
    }

    // all ok
    err = 0;

out:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// clear interface flags
int tun_clr_flags(const string &a_dev, short flags)
{
    int             fd      = -1;
    int             err     = -1;
    struct ifreq    ifr;
    int             res;

    // prepare ioctl
    fd = ioctl_prepare(a_dev, ifr);
    if (fd < 0){
        PERROR("Cannot prepare ioctl for device '%s'\n", a_dev.c_str());
        goto fail;
    }

    // get interface flags
    res = tun_get_flags(a_dev, ifr.ifr_flags);
    if (res < 0){
        PERROR("Cannot get interface: '%s' flags\n", a_dev.c_str());
        goto fail;
    }

    // update flags
    ifr.ifr_flags &= ~flags;

    // set interface flags
    res = ioctl(fd, SIOCSIFFLAGS, &ifr);
    if (res < 0){
        PERROR("Cannot set interface: '%s' flags,"
            " ioctl SIOCSIFFLAGS, failed\n",
            a_dev.c_str()
        );
        goto fail;
    }

    // all ok
    err = 0;

out:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// test if a specified flags is set
int tun_test_flag(const string &a_dev, short a_flags)
{
    short   flags   = 0;
    int     ret     = 0;
    int     res;

    // get interface flags
    res = tun_get_flags(a_dev, flags);
    if (res < 0){
        PERROR("Cannot get interface: '%s' flags\n", a_dev.c_str());
        goto fail;
    }

    // calculate flags
    ret = flags & a_flags;

out:
    return ret;

fail:
    goto out;
}

int tun_bring_up(const string &a_dev)
{
    return tun_set_flags(a_dev, IFF_UP);
}

int tun_bring_down(const string &a_dev)
{
    return tun_clr_flags(a_dev, IFF_UP);
}

