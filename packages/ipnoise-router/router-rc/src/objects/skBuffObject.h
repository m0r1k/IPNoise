/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

using namespace std;

class SkBuffObject;

#ifndef SK_BUFF_OBJECT_H
#define SK_BUFF_OBJECT_H

#include <ipnoise-common/log.h>
#include "clientObject.h"
#include "objects/object.h"

class DomDocument;

class SkBuffObject
    :   public Object
{
    public:
        SkBuffObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_skbuff"
        );
        virtual ~SkBuffObject();

        // internal
        virtual SkBuffObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_skbuff"
        );
        int32_t         autorun();
        virtual void    registrySignals();

        // generic
        void    delFromLL();
        string  toString(const string &descr = "");
        void    dump2stderr(const string &descr = "");

        void setInContent(const string &data);
        void setInContent(string &data);

        DomElement *getInDom(int delete_if_exist    = 0);
        DomElement *getOutDom(int delete_if_exst   = 0);

        // must be called before packet will received|send
        void prepare();

        string          getFromLL();
        string          getInDevName();
        void            setInDevName(const string &);
        string          getInDevIndex();
        void            setInDevIndex(const string &);

        string          getFrom();
        void            setFrom(const string &);

        string          getTo();
        void            setTo(const string &);
        string          getToLL();
        void            setToLL(const string &);
        string          getOutDevName();
        void            setOutDevName(const string &);

        ClientObject *  getClient();

    private:
        DomElement  *in_dom;
        DomElement  *out_dom;

};

#endif

