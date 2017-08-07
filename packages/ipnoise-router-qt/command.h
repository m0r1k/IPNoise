#ifndef COMMAND_H
#define COMMAND_H

#include <QObject>

#include "log.h"
#include "skbuff.h"

class Command : public QObject
{
    Q_OBJECT

    public:
        Command();
        ~Command();
        int init();

    signals:
        void sendPacket(sk_buff *skb);
        void searchRoute(sk_buff *skb);
        void searchRouteAnswer(sk_buff *skb);

    public slots:
        void recvCommandSlot(sk_buff *skb);
        void sendCommandSlot(sk_buff *skb);

        void searchRouteSlot(sk_buff *skb);
        void searchRouteAnswerSlot(sk_buff *skb);
        void doSearchRouteSlot(sk_buff *skb);
};

#endif

