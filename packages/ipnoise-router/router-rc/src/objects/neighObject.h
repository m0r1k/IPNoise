/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string.h>

class NeighObject;

using namespace std;

// time (sec) after that neigh record will rechecked
#define NEIGH_RECHECK_TIME  10

// max recheck fail count, before neigh will be marked as down
#define NEIGH_RECHECK_FAIL_MAX  3

#ifndef NEIGH_OBJECT_H
#define NEIGH_OBJECT_H

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include "objects/neighCheckingObject.h"

#include "objects/object.h"
#include "objects/neighCheckingObject.h"
class DomDocument;
class NeighsObject;

#include "objects/skBuffObject.h"

class NeighObject
    :   public Object
{
    public:
        NeighObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_neigh"
        );
        ~NeighObject();

        // internal
        virtual NeighObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_neigh"
        );
        virtual string      getAttribute(const string &name);
        virtual void        setAttribute(
            const string    &a_name,
            const string    &a_val
        );
        virtual int32_t     autorun();
        virtual void        timer();

        // generic
        void        doCheck(const string &a_src_huid);
        void        doCheck(map<const string, int> &);
        void        doCheck();
        string      getState();
        bool        isUp();
        bool        is_local();

        string      getDstHuid();
        string      getLLAddr();
        string      getDev();
        string      getLinkName();
        string      getLinkAddress();
        string      getMetric();
        int32_t     getRecheckTime();
        int32_t     getRecheckAfter();
        int32_t     getRecheckFailMax();
        string      getLastRecv();
        string      getLastCheck();
        int32_t     getDownTime();
        int32_t     getUpTime();

        void        setSrcHuid(const string &a_src_huid);
        string      getSrcHuid();

        void        setDstHuid(const string &);
        void        setLLAddr(const string &);
        void        setDev(const string &);
        void        setMetric(const string &);
        void        setRecheckTime(const string &);
        void        setRecheckTime(const int &);
        void        setRecheckAfter(const string &);
        void        setRecheckAfter(const int &);
        void        setRecheckFailMax(const string &);
        void        setRecheckFailMax(const int &);
        void        setLastRecv(const string &);
        void        setDownTime(const string &);
        void        setDownTime(const int &);
        void        setUpTime(const string &);
        void        setUpTime(const int32_t &);

        int         unsetInNeighTable();
        int         unsetInRouteTable();
        int         setInNeighTable();
        int         setInRouteTable();

        void        getNeighsChecking(
            vector<NeighCheckingObject *>   &a_out
        );

        NeighsObject *      getNeighsObject();

        virtual void        setStateDown();
        virtual void        setStateUp();

        // signal|splots implementation
        virtual void        registrySignals();
        virtual void        registrySlots();
        virtual void slot(
            const string    &signal_name,
            DomElement      *object
        );

        virtual int cmp(NeighObject &right);
        virtual bool operator>(NeighObject &right);
};

#endif

