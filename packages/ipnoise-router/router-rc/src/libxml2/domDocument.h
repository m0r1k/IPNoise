/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <string>
#include <vector>
#include <map>

using namespace std;

class DomDocument;

#ifndef DOM_DOCUMENT_H
#define DOM_DOCUMENT_H

#include <libxml/parser.h>

#include <ipnoise-common/log.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include <assert.h>

// libevent
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/dns.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#include "elements.hpp"

typedef map<string, string>         DirtyContainers;
typedef DirtyContainers::iterator   DirtyContainersIt;

typedef map<string, DomElement *>   ElementsByIno;
typedef ElementsByIno::iterator     ElementsByInoIt;

typedef map<string, DomElement *>   Tags;
typedef Tags::iterator              TagsIt;

class DomDocument
{
    public:
        enum {
            USE_NET_OBJECTS     = 1  << 0,
            USE_SYS_OBJECTS     = 1  << 1,
            USE_EVENTS          = 1  << 2,
            USE_ALL_PRIVS       = (1 << 3) - 1
        };
        DomDocument(
            int32_t     _priveleges = USE_NET_OBJECTS,
            xmlDocPtr   _doc        = NULL
        );
        virtual ~DomDocument();

        #include "create_element.hpp"
        #include "register_element.hpp"

        static void element_dump_cb(
            DomElement  *a_element,
            int32_t     a_deep
        );
        static void element_delete_cb(
            DomElement  *a_element,
            int32_t     a_deep
        );
        static void element_delete_safe_cb(
            DomElement *a_element,
            int32_t     a_deep
        );
        static void element_expire_cb(
            DomElement *a_element,
            int32_t     a_deep
        );
        static void element_shutdown_cb(
            DomElement  *a_element,
            int32_t     a_deep
        );
        static int32_t processAllElementsFromEnd(
            DomElement  *a_element,
            void        (*a_cb)(DomElement *, int32_t)
        );
        static int32_t processAllChildsFromEnd(
            DomElement  *a_element,
            void        (*a_cb)(DomElement *, int32_t)
        );
        static int32_t processAllElementsFromEnd(
            DomElement      *a_element,
            void            (*a_cb)(DomElement *, int32_t),
            const int32_t   &a_deep,
            const int32_t   &a_process_only_childs
        );

        int32_t     initTags();
        int32_t     isLoaded();
        void        shutdown();
        void        removeExpired(int32_t a_remove_all = 0);
        static void removeExpired(
            DomElement  *,
            int32_t     a_remove_all = 0
        );
        void        callExpiredTimers();
        void        callExpiredTimers(DomElement *);

        static void timer_cb(
            evutil_socket_t     fd,
            short               event,
            void                *arg
        );

        xmlNodePtr createNodeFromContent(const string &a_content);
        DomElement * createElementFromContent(const string &content);

        DomElement  *   getRootElement();
        void            setRootElement(DomElement *);
        void            deleteRootElement();
        static void     deleteElement(DomElement *);
        static void     deleteElementSafe(DomElement *);
        static void     deleteChilds(DomElement *);
        static void     deleteChildsSafe(DomElement *);
        static void     unlinkElementSafe(DomElement *);
        static void     unlinkElement(DomElement *);

        static int32_t  readFile(
            Path    &a_path,
            string  &a_data
        );

        string      serialize(int32_t format_id = 0);
        string      serializeForPublic(int32_t format_id = 0);

        void            initObjects(xmlNodePtr node);
        int32_t         loadFromMemory(const string &);
        int32_t         loadFromFile(const string &);
        DomElement *    parseBalancedChunkMemory(
            const string &
        );
        int32_t         loadContainer(DomElement  *);
        void            loadContainers(xmlNodePtr);
        int32_t         save2file(const string &fname);
        int32_t         getPriveleges();
        void            saveDirtyContainers();

        void            dump2stderr(int32_t format_id = 0);

        void    markContainerDirty(
            DomElement      *a_el,
            const string    &a_debug = ""
        );

        DomElement *    getElementByIno(const string &);
        void            getElementsByXpath(
            const string            &,
            vector <DomElement *>   &
        );
        void            getElementsByXpath(
            DomElement              *,
            const string            &,
            vector <DomElement *>   &
        );
        DomElement *    getElementByXpath(
            const string    &
        );
        DomElement *    getElementByXpath(
            DomElement      *,
            const string    &
        );

        struct event_base * getEvBase();
        struct evdns_base * getEvDnsBase();

        // signal-slot implementation
        DomElement *getSignal(
            const   string  &slot_name,
            const   string  &object_type,
            int32_t         create_if_not_exist = 0
        );
        DomElement *getSlot(
            const string    &slot_name,
            const string    &object_type,
            int32_t         create_if_not_exist = 0
        );
        virtual void registrySignal(
            const string    &signal_name,
            const string    &object_type
        );
        virtual void registrySlot(
            DomElement      *element,
            const string    &signal_name,
            const string    &object_type
        );
        virtual void emitSignal(
            const string    &signal_name,
            DomElement      *object
        );
        virtual void emitSignalAndDelObj(
            const string    &signal_name,
            DomElement      *object
        );

    friend class DomElement;

    protected:
        void deliverSignals();
        void removeSignalObjects();

    private:
        string      _loaded_from_file;

        xmlDocPtr   m_doc;
        void        registrySignals();
        void        registrySlots();
        int32_t     autorunObjects(xmlNodePtr node);
        int32_t     autorunObjects(DomElement *element);

        int32_t     m_priveleges;
        int32_t     is_loaded;
        int32_t     m_uptime;

        struct event        *timer;
        struct event_base   *base;
        struct evdns_base   *dns_base;

        void do_shutdown();
        enum States {
            ERROR   = 0,
            RUNNING,
            SHUTDOWN
        } state;

        DirtyContainers     m_dirty_containers;
        ElementsByIno       m_elements_by_ino;

        Tags                m_tags_events;
        Tags                m_tags_sys;
        Tags                m_tags_net;
};

#endif

