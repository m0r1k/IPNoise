/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <map>
#include <string>

using namespace std;

class NetCommandObject;

#include "objects/api/apiCommand.hpp"

typedef map<string, ApiCommand *>   NetCommands;
typedef NetCommands::iterator       NetCommandsIt;

#ifndef NET_COMMAND_OBJECT_H
#define NET_COMMAND_OBJECT_H

#include <ipnoise-common/log.h>
#include "objects/netCommandAndEventObject.h"

class DomDocument;

#include "objects/skBuffObject.h"
#include "objects/itemObject.h"
#include "objects/neighsObject.h"
#include "objects/neighObject.h"
#include "objects/serverObject.h"

#include "objects/client/handlerHttp/clientHandlerHttpObject.h"

class NetCommandObject
    :   public NetCommandAndEventObject
{
    public:
        NetCommandObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "command"
        );
        ~NetCommandObject();

        // internal
        virtual NetCommandObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "command"
        );

        // generic
        virtual int processCommand(SkBuffObject *skb);

    protected:
        void    registerCommandsClient();
        void    registerCommandsLink();
        void    registerCommandsIPNoise();
};

#endif

