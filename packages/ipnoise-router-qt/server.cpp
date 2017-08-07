#include <sys/ioctl.h>
#include <net/if.h>

#include "server.h"
#include "main.h"

extern Instance *inst;

Server::Server()
{
    timer  = NULL;
    uptime = 0;
}

Server::~Server()
{
}

void Server::CheckInterfaces()
{
    int i, res;

    QString ifname  = "";
    int s           = -1;
    short int flags = 0;
    struct ifreq ifr;

    sk_buff     *skb    = NULL;
    Transport   *tr     = NULL;

    TRANSPORTS::iterator transports_i;
    QTime cur_time = QTime::currentTime();

    // update uptime
    uptime++;

    for (i = 1;; i++){
        // EVERY TIME INITIALIZE STACK-VARIABLES!
        memset(&ifr, 0x00, sizeof(ifr));

        // sometime QT (QNetworkInterface::allInterfaces)
        // have return empty interface name,
        // so, searching name via ioctl
        s = socket (PF_INET, SOCK_STREAM, 0);
        if (s < 0){
            break;
        }

        // prepare ioctl
        ifr.ifr_ifindex = i;

        do {
            // get ifname
            res = ioctl (s, SIOCGIFNAME, &ifr);
            if (res < 0){
                break;
            }
            ifname = ifr.ifr_name;

            // get ifflags
            res = ioctl (s, SIOCGIFFLAGS, &ifr);
            if (res < 0){
                break;
            }
            flags  = ifr.ifr_flags;
        } while (0);

        // close socket
        close(s);

        // check errors (do after close socket)
        if (res < 0){
            break;
        }

        // skip 'lo' and empty interface
        if (ifname == ""
            || ifname == "lo")
        {
            continue;
        }

        // update liave-interfaces
        if (   (flags & IFF_UP)
            && (flags & IFF_RUNNING))
        {
            transports_i = transports.find(ifname);
            if (transports_i != transports.end()){
                // interface already exist
                tr = transports_i.value();
            } else {
                // add new transport
                tr = new Transport(ifname, i);
                transports[tr->ifname] = tr;
            }
            // mark last check time
            tr->last_check = cur_time;
        }
    }

again:
    // remove old interfaces
    for (transports_i = transports.begin();
        transports_i != transports.end();
        transports_i++)
    {
        tr = transports_i.value();
        if (tr->last_check != cur_time){
            delete tr;
            // interface now in down state, remove it
            transports.erase(transports_i);
            goto again;
        }
    }

    if (uptime == 10){
        uptime = 0;
        skb             = new sk_buff();
        skb->to         = "2210:0:7309:227d:3030:fb23:816a:cc5e";
        skb->from       = "2210:0:7309:227d:3030:fb23:816a:cc5d";
        skb->to_ll      = "82.198.186.46:2210";
        inst->cmd->doSearchRouteSlot(skb);
    }
}

void Server::updateIPListSlot(sk_buff *skb)
{
    QString cmd = "";
    PDEBUG(5, "+from ip:            '%s'\n", skb->from.toAscii().data());
    PDEBUG(5, "+from ll:            '%s'\n", skb->from_ll.toAscii().data());
    PDEBUG(5, "+from indev_name:    '%s'\n", skb->indev_name.toAscii().data());

    iplist.updateRecord(
        skb->from,
        skb->from_ll,
        skb->indev_name
    );
    iplist.dump();

}

void Server::netif_rx(sk_buff *skb)
{
    if (skb->to != "2210:0:7309:227d:3030:fb23:816a:cc5e"){
        goto drop;
    }

    // emit signal
    //emit packetReceived(*skb);
    updateIPListSlot(skb);
    
    // receive command
    inst->cmd->recvCommandSlot(skb);

ret:
    return;
drop:
    delete skb;
    goto ret;
}

void Server::sendPacket(sk_buff *skb)
{
    Transport *tr = NULL;
    TRANSPORTS::iterator transports_i;
    for (transports_i = transports.begin();
        transports_i != transports.end();
        transports_i++)
    {
        tr = transports_i.value();
        if (skb->outdev_name.size()
            && skb->outdev_name != tr->ifname)
        {
            PDEBUG(5, "Server::sendPacket invalid ifname\n");
            continue;
        }
        if (skb->outdev_index >= 0
            && skb->outdev_index != tr->ifindex)
        {
            PDEBUG(5, "Server::sendPacket invalid index\n");
            continue;
        }
        tr->sendPacket(skb);
    }
    delete skb;
}

int32_t Server::init()
{
    int32_t err;

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(CheckInterfaces()));
    connect(
        this,   SIGNAL(packetReceived(sk_buff skb)),
        this,   SLOT(updateIPListSlot(sk_buff skb))
    );

    CheckInterfaces();

    timer->start(1000);

    err = 0;
ret:
    return err;
fail:
    err = -1;
    goto ret;
}


