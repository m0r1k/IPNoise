/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <algorithm>
#include <string>
#include <vector>
#include <map>

using namespace std;

// fuse
#ifndef FUSE_USE_VERSION
#define FUSE_USE_VERSION 26
#define _FILE_OFFSET_BITS 64
#include "fuse_lowlevel.h"
#endif

class DomElement;

class LogItem;
class FuseClient;
class SortItem;
class UserObject;
class ServerObject;

typedef vector <LogItem>    LOG_ITEMS;
typedef vector <fuse_req_t> PENDING_REQS;

// log fd, vector of log messages
typedef map <int, FuseClient *> FUSE_CLIENTS;

#ifndef DOM_ELEMENT_H
#define DOM_ELEMENT_H

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include <ipnoise-common/path.hpp>
#include "inode.h"

class DomDocument;

class LogItem
{
    public:
        string  msg;
};

class FuseClient
{
    public:
        FuseClient(){
            m_ino_subtype = 0;
        }
        ~FuseClient(){};

        LOG_ITEMS       log_items;
        PENDING_REQS    pending_reqs;

        // type
        void setType(const int a_ino_subtype){
            LogItem log_item;
            if (INODE_SUBTYPE_LOG == a_ino_subtype){
                // mark what is log handler
                log_item.msg = "log was inited\n";
                log_items.push_back(log_item);
            }
            m_ino_subtype = a_ino_subtype;
        }
        int getType(){
            return m_ino_subtype;
        }

        // fd
        void setFd(const int &a_fd){
            m_fd = a_fd;
        }
        int getFd(){
            return m_fd;
        }

    private:
        int     m_ino_subtype;
        int     m_fd;
};

class DomElement
{
    public:
        DomElement(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname
        );
        virtual ~DomElement();

        virtual DomElement * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname
        ) = 0;

        string      getData();
        void        setData(const string &);

        string      dump(const int &format_id = 2);
        void        dump2stderr(const int &format_id = 2);

        DomElement  *clone();

        string      serialize(const int &format_id = 0);
        string      serializeForPublic(const int &format_id = 0);

        string                  getIno();
        int                     getInoInt();
        void                    deleteChilds();
        void                    deleteChildsSafe();
        string                  getAttribute(const string &);
        int32_t                 getAttributeInt(const string &);
        uint32_t                getAttributeFromHex(const string &);
        map <string, string>    getAllAttributes();
        string                  getTagName();
        DomDocument     *       getDocument();
        DomElement      *       getParentNode();
        DomElement      *       getParentNodeByTagName(
            const string &
        );
        string                  getOwnerHuid();
        UserObject      *       getOwner();
        ServerObject    *       getServer();
        xmlElementType          getXmlType();
        DomElement              *prevSibling();
        DomElement              *nextSibling();

        // containers
        uint32_t        getContainerVersion();
        void            setContainerVersion(const uint32_t &);
        void            saveContainer(const string &a_debug = "");
        bool            isContainer();

        void            getChilds(vector<DomElement *> &);
        void            getAllChilds(vector<DomElement *> &);
        void            getElementsByTagName(
            const string            &,
            vector<DomElement *>    &
        );
        DomElement *    getElementByTagName(const string &);
        void            getAllChilds(
            DomElement              *,
            vector <DomElement *>   &
        );
        DomElement *    getFirstChild();

        void            getElementsByXpath(
            const string            &,
            vector <DomElement *>   &
        );
        DomElement *    getElementByXpath(const string &);
        string          getElementTreeXPath();

        static void     clear_containers(
            DomElement *a_el,
            int32_t     // deep
        );

        static string getElementTreeXPath(
            DomElement      *,
            const int       &a_stop_if_owner = 0
        );

        void getContainerInfo(
            Path    &a_container_path,
            string  &a_container_name
        );

        static string getElementOwnerTreeXPath(DomElement *);

        // will call before element will packed in container
        virtual void prepareForContainer();

        // autorun object after dom model will create
        // (from dom or memory)
        // or when new created element will be attached in DOM
        virtual int32_t autorun();

        // will called when app going down
        virtual void    shutdown();

        // will called when item is expired and before delete
        virtual void    expired();

        // will called when timer expired
        virtual void    timer();

        // mark object as dirty
        virtual void markAsDirty(const string &a_debug = "");

        // signal-slot implementation
        int deleteAfterDeliver(const int delete_after_deliver = -1);
        virtual void registrySignals();
        virtual void registrySlots();
        virtual void slot(
            const string    &,
            DomElement      *
        );

        // fuse
        virtual string  getFuseName();
        virtual string  getFuseFullName();
        virtual string  parseFuseName(const char *);

#include "objects/connectionObjectAbstract.h"
#include "objects/groupItemObjectAbstract.h"
#include "objects/groupObjectAbstract.h"
#include "objects/groupsObjectAbstract.h"
#include "objects/itemObjectAbstract.h"
#include "objects/neighCheckingObjectAbstract.h"
#include "objects/neighObjectAbstract.h"
#include "objects/neighsObjectAbstract.h"
#include "objects/netCommandObjectAbstract.h"
#include "objects/netEventObjectAbstract.h"
#include "objects/serverObjectAbstract.h"
#include "objects/skBuffObjectAbstract.h"
#include "objects/linkObjectAbstract.h"
#include "objects/userObjectAbstract.h"

        friend class DomDocument;

        // log
        FUSE_CLIENTS    fuse_clients;
        FuseClient      *openFuseClient();
        void            closeAllFuseClients();
        void            processLog(
            const int   &,
            fuse_req_t  req = NULL
        );
        void            closeFuseClient(FuseClient *);
        int             isFuseClientOpen(FuseClient *);
        void            pLog(LogItem &);
        void            pDebug(const string &);
        string          readLog(const int &);
        static int fuse_reply_buf_limited(
            fuse_req_t  a_req,
            const char  *a_buf,
            size_t      a_bufsize,
            off_t       a_off,
            size_t      a_maxsize
        );

        bool isAttached();
        bool DomAutorunCalled();
        bool DomRegistrySignalsCalled();
        bool DomRegistrySlotsCalled();
        void DomAutorunCalled(const bool &a_val);
        void DomRegistrySignalsCalled(const bool &a_val);
        void DomRegistrySlotsCalled(const bool &a_val);

        // TODO move to protected!
        // methods what don't call markAsDirty {
        void appendChildSafe(
            DomElement  *a_element,
            int32_t     a_call_autorun = 1
        );
        void setAttributeSafe(
            const string &a_name,
            const string &a_value
        );
        void setAttributeSafe(
            const string    &a_name,
            const int32_t   &a_value
        );
        void setAttributeHexSafe(
            const string    &a_name,
            const uint32_t  &a_value
        );
        void setAttributeHex(
            const string    &a_name,
            const uint32_t  &a_value
        );

        void delAttributeSafe(const string &a_name);
        void unsetAttributeSafe(const string &a_name);
        void appendCDataBlockSafe(const string &a_data);


//del        void deleteElementSafe();
//del        void deleteChildsSafe();
//del        void unlinkSafe();

        // methods what don't call markAsDirty }

        // methods what call markAsDirty {
        // you may redefine it for avoid markAsDirty() call
        // (call ****Safe methods instead)
        virtual void appendChild(DomElement *a_element);
        virtual void setAttribute(
            const string    &a_name,
            const string    &a_value
        );
        virtual void setAttribute(
            const string    &a_name,
            const int32_t   &a_value
        );
        virtual void delAttribute(const string &a_name);
        virtual void unsetAttribute(const string &a_name);
        virtual void appendCDataBlock(const string &a_data);

//del        virtual void deleteElement();
//del        virtual void deleteChilds();
//del        virtual void unlink();

        // methods what call markAsDirty }

    protected:
        // methods what don't call markAsDirty {
        // methods what don't call markAsDirty }

        // methods what call markAsDirty {
        // you may redefine it for avoid markAsDirty() call
        // (call ****Safe methods instead)
        // methods what call markAsDirty }

    public:
        string          parseTime(const string &a_unix_time);
        time_t          getExpired();
        void            unsetExpired();
        void            setExpired(const string &a_time);
        time_t          getTimer();
        void            setTimer(const string &a_time);
        void            removeTimer();
        int             sortChilds();
        virtual int     sortChildsSafe();
        virtual int     cmp(DomElement *a_right);

    private:
        xmlNodePtr      m_node;
        DomDocument     *m_doc;
        bool            m_dom_autorun_called;
};

class SortItem
{
    public:
        DomElement  *m_element;

        bool operator<(const class SortItem &a_right) const {
            return (m_element->cmp(a_right.m_element) == +1)
                ? true
                : false;
        }
};


#endif

