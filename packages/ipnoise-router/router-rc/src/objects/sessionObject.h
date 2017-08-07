/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jun 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>

using namespace std;

class SessionObject;

#ifndef SESSION_OBJECT_H
#define SESSION_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/ioObject.h"

class DomDocument;

#include "objects/userObject.h"
#include "objects/itemObject.h"
#include "objects/packetReceiverObject.h"

#define SESSION_ID_LENGTH_BYTES 10
#define SESSION_INIT_TIMEOUT    "+30"

class SessionObject
    :   public IoObject
{
    public:
        SessionObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname
        );
        virtual ~SessionObject();

        // internal
        virtual SessionObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname
        );
        virtual void            shutdown();
        virtual int32_t         autorun();
        virtual void            registrySignals();
        virtual void            timer();

        // interface
        virtual int         connect();
        virtual ssize_t     xmit(
            DomElement      *,
            const int32_t   &a_delete_after_send = 1
        );
        virtual void        netif_rx(SkBuffObject *);
        virtual void        process_packet(SkBuffObject *);

        // generic
        PacketReceiverObject *  getNextReceiver();
        PacketReceiverObject *  queue(const string &);
        PacketReceiverObject *  queue(DomElement *);
        void                    xmit(const string &);
        ssize_t                 xmitLocal(DomElement *);
        DomElement      *       getCreateCommandsObject();
        DomElement      *       getCreateEventsObject();
        SessionsObject  *       getSessionsObject();
        string                  getApi();
        void                    setApi(const string &api);

        string              getSessId();
        void                setSessId(const string &sessid);
        string              getSrcSessId();
        void                setSrcSessId(const string &);
        string              getDstSessId();
        void                setDstSessId(const string &);

        // seq
        void                setSeq(const string &);
        void                setSeq(const uint32_t &);
        uint32_t            getSeqUint();
        string              getSeq();

        // ack
        void                setAck(const string &);
        void                setAck(const uint32_t &);
        uint32_t            getAckUint();
        string              getAck();
        void                sendAck();
        void                sendAck(const uint32_t &);

        ItemObject  *       getItem();

        void                setHuid(string &huid);
        string              getHuid();
        string              getType();
        void                setType(const string &type);
        bool                isOnline();
        string              getState();
        void                setState(const string &a_state);

        void                flush();
        void                clearQueue();

    protected:
        string createSessionId();
};

#endif

