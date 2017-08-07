/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "main.h"
#include "net/command.h"
#include "objects/serverObject.h"
#include "objects/netCommandObject.h"
#include "objects/netEventObject.h"
#include "objects/ipnoiseObject.h"

Command::Command(ServerObject *server)
{
    _server = server;
};

Command::~Command()
{
};

int Command::init()
{
    return 0;
}

void Command::recvCommandSlot(SkBuffObject *skb)
{
    int i;
    char buffer[1024];

    vector <DomElement *> commands;
    vector <DomElement *> events;

    NetCommandObject    *command = NULL;
    NetEventObject      *event   = NULL;

    PDEBUG_OBJ(_server, 10, "new command was received {\n");
    PDEBUG_OBJ(_server, 10, "from:      '%s'\n", skb->getAttribute("from").c_str());
    PDEBUG_OBJ(_server, 10, "to:        '%s'\n", skb->getAttribute("to").c_str());
    PDEBUG_OBJ(_server, 10, "%s\n",     skb->toString().c_str());
    PDEBUG_OBJ(_server, 10, "new command was received }\n");

    // search commands
    snprintf(buffer, sizeof(buffer),
        "//ipn_in_dom/ipnoise/commands/command"
    );

    commands.clear();
    skb->getElementsByXpath(buffer, commands);
    for (i = 0; i < (int)commands.size(); i++){
        command = (NetCommandObject *)commands[i];
        assert (command != NULL);
        command->processCommand(skb);
    }

    if (skb->getAttribute("indev_name") != CLIENTS_DEV_NAME){
        // search events for non client's packages
        snprintf(buffer, sizeof(buffer),
            "//ipn_in_dom/ipnoise/events/event"
        );
        events.clear();
        skb->getElementsByXpath(buffer, events);
        for (i = 0; i < (int)events.size(); i++){
            event = (NetEventObject *)events[i];
            assert (event != NULL);
            event->processEvent(skb);
        }
    }

    return;
};

void Command::doSearchRouteSlot(SkBuffObject *skb)
{
    ServerObject    *server         = NULL;
    IPNoiseObject   *ipnoise        = NULL;

    DomElement          *outdom     = NULL;
    NetCommandObject    *commands   = NULL;
    NetCommandObject    *command    = NULL;

    ipnoise = (IPNoiseObject *)skb->getDocument()->getRootElement();
    assert(ipnoise);

    // get server
    server = ipnoise->getServerObject();
    assert (server != NULL);

    outdom  = skb->getOutDom(1);
    command = skb->getDocument()->createElement<NetCommandObject>("command");
    command->setType("searchRoute");

    commands = skb->getDocument()->createElement<NetCommandObject>("commands");
    commands->appendChildSafe(command);
    outdom->appendChildSafe(commands);

    // send packet
    skb->prepare();
    server->getDocument()->emitSignalAndDelObj((const char *)"packet_outcoming", skb);
}

