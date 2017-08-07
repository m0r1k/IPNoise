/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jan 2012 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <map>
#include <string>

using namespace std;

class NetCommandAndEventObject;

#ifndef NET_COMMAND_AND_EVENT_OBJECT_H
#define NET_COMMAND_AND_EVENT_OBJECT_H

#include "objects/object.h"
class DomDocument;

#include "objects/skBuffObject.h"
#include "objects/itemObject.h"
#include "objects/neighsObject.h"
#include "objects/neighObject.h"
#include "objects/serverObject.h"

#include "objects/client/handlerHttp/clientHandlerHttpObject.h"

class NetCommandAndEventObject
    :   public Object
{
    public:
        NetCommandAndEventObject(
            xmlNodePtr      _node,
            DomDocument     *_doc,
            const string    &_tagname
        );
        ~NetCommandAndEventObject();

        // internal
        virtual NetCommandAndEventObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname
        );

        // generic
        void            setType(const string &type);
        string          getType();

        string          getSessId(SkBuffObject*);
        SessionObject * getSession(
            SkBuffObject    *skb,
            const string    &a_sessid = ""
        );

        int             sendImmediately(SkBuffObject *skb, string &answer);
        int             errorUnsupportedCommand(SkBuffObject *skb);
        int             errorSessionFailed(SkBuffObject *skb);
        int             errorInternalError(SkBuffObject *skb);
        int             errorNotLogged(SkBuffObject *skb);
        int             errorSyntaxError(SkBuffObject *skb);

        static int appendNeighsUp(
            DomElement * // where must be added new neighs
        );
};

#endif

