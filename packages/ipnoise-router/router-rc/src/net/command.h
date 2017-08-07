/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

using namespace std;

class Command;

#ifndef COMMAND_H
#define COMMAND_H

#include <ipnoise-common/log.h>
#include "objects/serverObject.h"
#include "objects/skBuffObject.h"

class Command
{
    public:
        Command(ServerObject *server);
        ~Command();
        int init();

        void sendPacket(SkBuffObject *skb);
        void searchRoute(SkBuffObject *skb);
        void searchRouteAnswer(SkBuffObject *skb);

        void recvCommandSlot(SkBuffObject *skb);
        void doSearchRouteSlot(SkBuffObject *skb);

    private:
        ServerObject *_server;
};

#endif

