#ifndef SERVER_H
#define SERVER_H

#include <stdint.h>
#include <QObject>
#include <QTimer>
#include <QHash>
#include <QTime>
#include <QString>

#include "transport.h"
#include "iplist.h"

// <if_name, Transport>
typedef QHash<QString, Transport *> TRANSPORTS;

class Server : public QObject
{
    Q_OBJECT

    public:
        Server();
        ~Server();

        int32_t init();
        int32_t uptime;

    private:
        QTimer      *timer;
        TRANSPORTS  transports;
        IPList      iplist; 
        
    signals:
        void    packetReceived(sk_buff skb);

    public slots:
        void    CheckInterfaces();
        void    sendPacket(sk_buff *skb);
        void    netif_rx(sk_buff *);
        void    updateIPListSlot(sk_buff *skb);
};

#endif

