#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include <QObject>
#include <QTime>
#include <QString>
#include <QSocketNotifier>

#include <QDebug>

#include "libnet.h"
#include "log.h"
#include "skbuff.h"

#define IPNOISE_UDP_PORT 2210

class Transport : public QObject
{
    Q_OBJECT

    public:
        Transport(QString _ifname, int _ifindex);
        ~Transport();

        QTime       last_check;
        QString     ifname;
        int         ifindex;
        int         sd;

        int sendPacket(sk_buff *skb);

    private:
        QSocketNotifier *udp;
        libnet_ptag_t   t;
        libnet_t        *l;
        char errbuf[LIBNET_ERRBUF_SIZE];

    public slots:
        void    readUdp(int socket);
};

#endif

