/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <ipnoise-common/log.h>
#include <ipnoise-common/path.hpp>

#include "libxml2/domDocument.h"

DomDocument::DomDocument(
    int32_t     _priveleges,
    xmlDocPtr   _doc)
{
    const xmlChar *ver = (const xmlChar *)"1.0";

    if (_doc){
        m_doc = _doc;
    } else {
        m_doc = xmlNewDoc(ver);
    }

    m_priveleges  = _priveleges;
    is_loaded   = 0;
    m_uptime    = 0;

    base                = NULL;
    dns_base            = NULL;
    _loaded_from_file   = "";

    if (m_priveleges & USE_EVENTS){
        // init libevent
        event_enable_debug_mode();
        base = event_base_new();
        if (!base) {
            PFATAL_OBJ(this,
                "Could not initialize libevent! (event_base_new)\n");
        }
        dns_base = evdns_base_new(
            base,
            0       // init name servers
        );
        if (!dns_base) {
            PFATAL_OBJ(this,
                "Could not initialize libevent! (evdns_base_new)\n");
        }

        struct timeval tv;
        timer = evtimer_new(
            base,
            DomDocument::timer_cb,
            (void*)this
        );
        evutil_timerclear(&tv);
        tv.tv_sec = 1;
        evtimer_add(timer, &tv);
    }

    initTags();

    state = RUNNING;
}

int32_t DomDocument::initTags()
{
    int32_t err = 0;
    TagsIt  tags_it;
    string  supported_events;
    string  supported_net;
    string  supported_sys;

    // ---------- events tags ----------

    REGISTER_EVENTS_ELEMENT<TmpObject>();

    // ---------- net tags ----------

    REGISTER_NET_ELEMENT("command");
    REGISTER_NET_ELEMENT("event");

    // ---------- system tags ----------

    REGISTER_SYS_ELEMENT<IPNoiseObject>();
    // containers
    REGISTER_SYS_ELEMENT("ipn_container");
    REGISTER_SYS_ELEMENT("ipn_container_data");
    // users
    REGISTER_SYS_ELEMENT<UserObject>();
    REGISTER_SYS_ELEMENT<UsersObject>();
    // contacts
    REGISTER_SYS_ELEMENT<ContactsObject>();
    REGISTER_SYS_ELEMENT<ContactObject>();
    REGISTER_SYS_ELEMENT<ContactItemsObject>();
    REGISTER_SYS_ELEMENT<ContactItemObject>();
    // items
    REGISTER_SYS_ELEMENT<ItemsObject>();
    REGISTER_SYS_ELEMENT<ItemObject>();
    // groups
    REGISTER_SYS_ELEMENT<GroupObject>();
    REGISTER_SYS_ELEMENT<GroupsObject>();
    REGISTER_SYS_ELEMENT<GroupItemObject>();
    // neighs
    REGISTER_SYS_ELEMENT<NeighsObject>();
    REGISTER_SYS_ELEMENT<NeighObject>();
    REGISTER_SYS_ELEMENT<NeighCheckingsObject>();
    REGISTER_SYS_ELEMENT<NeighCheckingObject>();
    // server
    REGISTER_SYS_ELEMENT<ServerObject>();
    // links
    REGISTER_SYS_ELEMENT<LinksObject>();
    REGISTER_SYS_ELEMENT<LinkObject>();
    // sessions
    REGISTER_SYS_ELEMENT<SessionsObject>();
    REGISTER_SYS_ELEMENT<SessionUnknownObject>();
    REGISTER_SYS_ELEMENT<SessionClientObject>();
    REGISTER_SYS_ELEMENT<SessionItemObject>();
    REGISTER_SYS_ELEMENT<SessionIPNoiseObject>();
    REGISTER_SYS_ELEMENT<SessionLoObject>();
    // skb
    REGISTER_SYS_ELEMENT<SkBuffObject>();
    REGISTER_SYS_ELEMENT("ipn_in_dom");
    REGISTER_SYS_ELEMENT("ipn_out_dom");
    // clients
    REGISTER_SYS_ELEMENT("ipn_clients");
    REGISTER_SYS_ELEMENT<ClientObject>();
    REGISTER_SYS_ELEMENT<AcceptedClientObject>();
    REGISTER_SYS_ELEMENT<ClientHandlerUnknownObject>();
    REGISTER_SYS_ELEMENT<ClientHandlerTelnetObject>();
    REGISTER_SYS_ELEMENT<ClientHandlerHttpObject>();
    REGISTER_SYS_ELEMENT<ClientHandlerRawObject>();
    // net
    REGISTER_SYS_ELEMENT<NetClientObject>();
    REGISTER_SYS_ELEMENT<NetClientHttpObject>();
    // conference
    REGISTER_SYS_ELEMENT<ConferencesObject>();
    REGISTER_SYS_ELEMENT<ConferenceObject>();
    REGISTER_SYS_ELEMENT<ConferenceItemsObject>();
    REGISTER_SYS_ELEMENT<ConferenceItemObject>();
    REGISTER_SYS_ELEMENT<ConferenceMsgsObject>();
    REGISTER_SYS_ELEMENT<ConferenceMsgObject>();
    // user's info
    REGISTER_SYS_ELEMENT("ipn_icon");
    REGISTER_SYS_ELEMENT("ipn_nickname");
    REGISTER_SYS_ELEMENT("ipn_real_nickname");
    // slots, signals
    REGISTER_SYS_ELEMENT<SignalsObject>();
    REGISTER_SYS_ELEMENT("ipn_signal");
    REGISTER_SYS_ELEMENT("ipn_signal_obj");
    REGISTER_SYS_ELEMENT("ipn_process_signal_obj");
    REGISTER_SYS_ELEMENT<SlotsObject>();
    REGISTER_SYS_ELEMENT("ipn_slot");
    REGISTER_SYS_ELEMENT("ipn_slot_obj");
    // commands, events
    REGISTER_SYS_ELEMENT("ipn_commands");
    REGISTER_SYS_ELEMENT("ipn_events");
    // packets
    REGISTER_SYS_ELEMENT<PacketsObject>();
    REGISTER_SYS_ELEMENT<PacketObject>();
    REGISTER_SYS_ELEMENT<PacketReceiversObject>();
    REGISTER_SYS_ELEMENT<PacketReceiverObject>();

    // events
    for (tags_it = m_tags_events.begin();
        tags_it != m_tags_events.end();
        tags_it++)
    {
        DomElement *el = tags_it->second;
        if (supported_events.size()){
            supported_events += ", ";
        }
        supported_events += el->getTagName();
    }

    // net
    for (tags_it = m_tags_net.begin();
        tags_it != m_tags_net.end();
        tags_it++)
    {
        DomElement *el = tags_it->second;
        if (supported_net.size()){
            supported_net += ", ";
        }
        supported_net += el->getTagName();
    }

    // sys
    for (tags_it = m_tags_sys.begin();
        tags_it != m_tags_sys.end();
        tags_it++)
    {
        DomElement *el = tags_it->second;
        if (supported_sys.size()){
            supported_sys += ", ";
        }
        supported_sys += el->getTagName();
    }

    PWARN("Supported tag(s):\n"
        "    event(s):  '%s'\n"
        "    net(s):    '%s'\n"
        "    sys(s):    '%s'\n",
        supported_events.c_str(),
        supported_net.c_str(),
        supported_sys.c_str()
    );

    return err;
}

void DomDocument::saveDirtyContainers()
{
    DirtyContainersIt m_dirty_containers_it;

    while (m_dirty_containers.size()){
        m_dirty_containers_it = m_dirty_containers.begin();

        string      ino     = m_dirty_containers_it->first;
        string      debug   = m_dirty_containers_it->second;
        DomElement  *el     = NULL;

        m_dirty_containers.erase(m_dirty_containers_it);

        el = getElementByIno(ino);
        if (el){
            el->saveContainer(debug);
        } else {
            PERROR("Cannot get element by ino: 0x'%s'\n",
                ino.c_str()
            );
        }
    }
}

void DomDocument::timer_cb(
    evutil_socket_t     fd      __attribute__ ((unused)),
    short               event   __attribute__ ((unused)),
    void                *arg)
{
    DomDocument *doc = (DomDocument *)arg;
    assert(doc);

    // update uptime
    doc->m_uptime++;

    // call expired timers
    doc->callExpiredTimers();

    // remove expired items
    doc->removeExpired();

    // save dirty containers
    if (0 == (doc->m_uptime % 2)){
        doc->saveDirtyContainers();
    }

    if (doc->state == SHUTDOWN){
        // stop timer
        if (doc->timer){
            evtimer_del(doc->timer);
            doc->timer = NULL;
        }
        doc->do_shutdown();
        return;
    }

    {
        // deliver signals
        doc->deliverSignals();

        // Delete all signal_obj'ects what have not references
        doc->removeSignalObjects();
    }

    // reschedule
    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 1;
    evtimer_add(doc->timer, &tv);
}

int32_t DomDocument::getPriveleges()
{
    return m_priveleges;
}

int32_t DomDocument::processAllElementsFromEnd(
    DomElement  *a_element,
    void        (*a_cb)(DomElement *, int32_t))
{
    int32_t res = 0;
    res = processAllElementsFromEnd(
        a_element,
        a_cb,
        0,          // deep
        0           // process only childs
    );
    return res;
}

int32_t DomDocument::processAllChildsFromEnd(
    DomElement  *a_element,
    void        (*a_cb)(DomElement *, int32_t))
{
    int32_t res = 0;
    res = processAllElementsFromEnd(
        a_element,
        a_cb,
        0,          // deep
        1           // process only childs
    );
    return res;
}

int32_t DomDocument::processAllElementsFromEnd(
    DomElement      *a_element,
    void            (*a_cb)(DomElement *, int32_t),
    const int32_t   &a_deep,
    const int32_t   &a_process_only_childs)
{
    vector <DomElement *>           childs;
    vector <DomElement *>::iterator childs_i;

    childs.clear();
    a_element->getChilds(childs);
    for (childs_i = childs.begin();
        childs_i != childs.end();
        childs_i++)
    {
        processAllElementsFromEnd(
            *childs_i,
            a_cb,
            a_deep + 1,
            0       // process only childs
        );
    }

    // all childs processed, call callback function
    if (    a_process_only_childs
        &&  0 == a_deep)
    {
        // don't call callback if requested
    } else {
        a_cb(a_element, a_deep);
    }

    return 0;
}

void DomDocument::element_dump_cb(
    DomElement  *a_element,
    int32_t     a_deep)
{
    char buffer[1024];

    snprintf(buffer, sizeof(buffer),
        "Deep: '%d', %s",
        a_deep,
        a_element->serialize(0).c_str()
    );

    PWARN("%s\n", buffer);
}

void DomDocument::element_shutdown_cb(
    DomElement  *a_element,
    int32_t     a_deep        __attribute__ ((unused)))
{
    a_element->removeTimer();
    a_element->shutdown();
}

void DomDocument::element_expire_cb(
    DomElement  *a_element,
    int32_t     a_deep        __attribute__ ((unused)))
{
    a_element->expired();
}

void DomDocument::element_delete_safe_cb(
    DomElement  *a_element,
    int32_t     a_deep)
{
    DomDocument     *doc = a_element->getDocument();
    ElementsByInoIt m_elements_by_ino_it;

    // remove from 'ino' index
    m_elements_by_ino_it = doc->m_elements_by_ino.find(
        a_element->getIno()
    );
    if (doc->m_elements_by_ino.end() != m_elements_by_ino_it){
        doc->m_elements_by_ino.erase(m_elements_by_ino_it);
    }

    element_shutdown_cb(a_element, a_deep);
    element_expire_cb(a_element, a_deep);
    xmlUnlinkNode(a_element->m_node);
    xmlFreeNode(a_element->m_node);
}

void DomDocument::element_delete_cb(
    DomElement  *a_element,
    int32_t     a_deep)
{
    element_delete_safe_cb(a_element, a_deep);
}

DomDocument::~DomDocument()
{
/*
    xmlNodePtr root = NULL;

    if (doc != NULL){
        // Get the root element node
        root = xmlDocGetRootElement(doc);
        if (root != NULL){
            // deleteObjects(root);
            DomDocument::processAllElementsFromEnd(
                root,
                DomDocument::element_delete_cb
            );
        }
        xmlFreeDoc(doc);
        doc = NULL;
    }
*/
}

int32_t DomDocument::autorunObjects(xmlNodePtr node)
{
    DomElement  *element    = NULL;
    xmlNodePtr  cur_node    = NULL;
    int32_t     err         = -1;

    if (!(m_priveleges & USE_EVENTS)){
        PERROR("Attempt to autorun objects"
            " from document what have not"
            " USE_EVENTS m_priveleges\n"
        );
        goto fail;
    }

    for (cur_node = node;
        cur_node != NULL;
        cur_node = cur_node->next)
    {
        do {
            if (cur_node->type != XML_ELEMENT_NODE){
                break;
            }
            element = (DomElement *)cur_node->_private;
            if (NULL == element){
                break;
            }
            if (not element->DomAutorunCalled()){
                element->DomAutorunCalled(true);
                err = element->autorun();
                if (err){
                    PERROR_OBJ(element, "Autorun failed\n");
                    goto out;
                }
            }
            err = autorunObjects(cur_node->children);
            if (err){
                goto out;
            }

        } while (0);
    }

    // all ok
    err = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int32_t DomDocument::autorunObjects(DomElement *element)
{
    return autorunObjects(element->m_node);
}

void DomDocument::registrySignals()
{
    TagsIt tags_it;

    // events
    for (tags_it = m_tags_events.begin();
        tags_it != m_tags_events.end();
        tags_it++)
    {
        DomElement *el = tags_it->second;
        el->registrySignals();
    }

    // net
    for (tags_it = m_tags_net.begin();
        tags_it != m_tags_net.end();
        tags_it++)
    {
        DomElement *el = tags_it->second;
        el->registrySignals();
    }

    // sys
    for (tags_it = m_tags_sys.begin();
        tags_it != m_tags_sys.end();
        tags_it++)
    {
        DomElement *el = tags_it->second;
        el->registrySignals();
    }
}

void DomDocument::registrySlots()
{
    TagsIt tags_it;

    // events
    for (tags_it = m_tags_events.begin();
        tags_it != m_tags_events.end();
        tags_it++)
    {
        DomElement *el = tags_it->second;
        el->registrySlots();
    }

    // net
    for (tags_it = m_tags_net.begin();
        tags_it != m_tags_net.end();
        tags_it++)
    {
        DomElement *el = tags_it->second;
        el->registrySlots();
    }

    // sys
    for (tags_it = m_tags_sys.begin();
        tags_it != m_tags_sys.end();
        tags_it++)
    {
        DomElement *el = tags_it->second;
        el->registrySlots();
    }
}

void DomDocument::initObjects(xmlNodePtr node)
{
    xmlNodePtr cur_node = NULL;

    for (cur_node = node;
        cur_node != NULL;
        cur_node = cur_node->next)
    {
        do {
            if (cur_node->type != XML_ELEMENT_NODE){
                break;
            }
            if (cur_node->_private != NULL){
                break;
            }
            createElement(cur_node, (const char*)cur_node->name);
            initObjects(cur_node->children);
        } while (0);
    }
}

DomElement * DomDocument::parseBalancedChunkMemory(
    const string &a_data)
{
    int32_t     res         = 0;
    xmlNodePtr  new_node    = NULL;
    DomElement  *new_el     = NULL;

    res = xmlParseBalancedChunkMemory(
        m_doc,          // doc
        NULL,           // the SAX handler bloc
        NULL,           // The user data from SAX
        0,              // depth (for loop detect)
        (const xmlChar *)a_data.c_str(), // data
        &new_node       // results
    );
    if (    res
        ||  not new_node)
    {
        goto fail;
    }

    // init objects model
    initObjects(new_node);

    // get container element
    new_el = (DomElement *)new_node->_private;

out:
    return new_el;
fail:
    goto out;
}

int32_t DomDocument::readFile(
    Path    &a_path,
    string  &a_data)
{
    int32_t err     = -1;
    FILE    *f      = NULL;
    string  data;

    // check what file exist
    if (not a_path.isFile()){
        goto fail;
    }

    // open file
    f = a_path.fopen("r");
    if (not f){
        goto fail;
    }

    // read content
    do {
        char    *lineptr    = NULL;
        size_t  n           = 0;
        ssize_t res         = 0;

        res = getline(&lineptr, &n, f);
        if (res <= (ssize_t)0){
            break;
        }

        if (lineptr){
            data += lineptr;
            free(lineptr);
        }
    } while (1);

    fclose(f);

    // all ok
    a_data  = data;
    err     = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int32_t DomDocument::loadContainer(
    DomElement  *a_el)
{
    int32_t     err = -1;
    Path        container_path;
    string      container_name;

    string      data;
    DomElement  *container_el       = NULL;
    DomElement  *container_data_el  = NULL;
    int32_t     res                 = 0;

    vector <DomElement *>           els;
    vector <DomElement *>::iterator els_it;

    // get container info
    a_el->getContainerInfo(
        container_path,
        container_name
    );
    container_path.addPath(container_name);

    // trying to read container data
    res = DomDocument::readFile(container_path, data);
    if (res){
        goto fail;
    }

    // trying parse container
    container_el = parseBalancedChunkMemory(data);
    if (not container_el){
        PERROR("Cannot parse container,"
            " container path '%s',"
            " container data: '%s'\n",
            container_path.path().c_str(),
            data.c_str()
        );
        goto fail;
    }

    if ("ipn_container" != container_el->getTagName()){
        PERROR("Invalid container tag name: '%s'"
            " (must be: 'ipn_container')"
            " for container path '%s',"
            " container data: '%s'\n",
            container_el->getTagName().c_str(),
            container_path.path().c_str(),
            data.c_str()
        );
        goto fail;
    }

    container_data_el = container_el->getElementByTagName(
        "ipn_container_data"
    );
    if (not container_data_el){
        PERROR("Cannot get ipn_container_data element"
            " for container path '%s',"
            " container data: '%s'\n",
            container_path.path().c_str(),
            data.c_str()
        );
        goto fail;
    }

    // get container data
    els.clear();
    container_data_el->getChilds(els);
    for (els_it = els.begin();
        els_it != els.end();
        els_it++)
    {
        DomElement *el = *els_it;

        // deattach from container
        DomDocument::unlinkElementSafe(el);

        // and attach to dom
        a_el->appendChildSafe(
            el,     // element for append
            0       // don't autorun objects
        );
    }

    // all ok
    err = 0;

out:
    if (container_el){
        DomDocument::deleteElementSafe(container_el);
        container_el = NULL;
    }
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void DomDocument::loadContainers(xmlNodePtr node)
{
    DomElement  *cur_element    = NULL;
    xmlNodePtr  cur_node        = NULL;

    if (!node){
        goto out;
    }

    for (cur_node = node;
        cur_node != NULL;
        cur_node = cur_node->next)
    {
        do {
            if (cur_node->type != XML_ELEMENT_NODE){
                break;
            }
            cur_element = (DomElement *)cur_node->_private;
            if (not cur_element){
                PERROR("internal error,"
                    " cannot get element\n");
                break;
            }
            // trying to load container
            loadContainer(cur_element);
            // process childrens
            if (cur_node->children){
                loadContainers(cur_node->children);
            }
        } while (0);
    }

out:
    return;
}

int32_t DomDocument::loadFromMemory(const string &mem)
{
    int32_t err = -1;
    xmlNodePtr root = NULL;

    // load from memory
    _loaded_from_file = "";

    if (m_doc){
        // TODO XXX FIXME
        // how about correct tree free?
        // we need free element by element
        xmlFreeDoc(m_doc);
        m_doc = NULL;
    }
    m_doc = xmlReadMemory(
        mem.c_str(),        // a pointer to a char array
        mem.size(),         // the size of the array
        NULL,               // the base URL to use for the document
        "UTF-8",            // the document encoding, or NULL
        0                   // a combination of xmlParserOption
    );

    if (!m_doc){
        PERROR("Cannot parse XML: '%s'\n", mem.c_str());
        goto fail;
    }

    // Get the root element node
    root = xmlDocGetRootElement(m_doc);

    // init objects model
    initObjects(root);

    if (m_priveleges & USE_EVENTS){
        // load containers
        loadContainers(root);

        // registry signals
        registrySignals();

        // registry slots
        registrySlots();

        // autorun objects
        err = autorunObjects(root);
        if (err){
            PERROR("autorunObjects failed\n");
            goto fail;
        }
    }

    // mark as loaded
    is_loaded = 1;

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >=0){
        err = -1;
    }
    goto out;
}

int32_t DomDocument::loadFromFile(const string &fname)
{
    int32_t err = -1;
    FILE    *f  = NULL;

    ssize_t res     = 0;
    string  content = "";

    f = fopen(fname.c_str(), "r");
    if (!f){
        PERROR("Cannot open: '%s' for read\n", fname.c_str());
        goto fail;
    }

    do {
        char    *lineptr    = NULL;
        size_t  n           = 0;

        res = getline(&lineptr, &n, f);
        if (res <= (ssize_t)0){
            break;
        }

        if (lineptr){
            content += lineptr;
            free(lineptr);
        }
    } while (1);

    fclose(f);

    // start load dom
    err = loadFromMemory(content.c_str());
    if (err){
        goto fail;
    }

    // store file name
    _loaded_from_file = fname;

    // all ok
    err = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

DomElement *DomDocument::getRootElement()
{
    DomElement *element = NULL;
    xmlNodePtr root     = xmlDocGetRootElement(m_doc);
    if (root == NULL){
        goto ret;
    }
    element = (DomElement *)root->_private;

ret:
    return element;
}

/*
 * Delete document elements without update container
 */
void DomDocument::deleteElementSafe(DomElement *element)
{
    assert(element);

    // close all open fuse clients
    element->closeAllFuseClients();

    DomDocument::processAllElementsFromEnd(
        element,
        DomDocument::element_delete_cb
    );
}

/*
 * Delete childs elements without update container
 */
void DomDocument::deleteChildsSafe(DomElement *element)
{
    assert(element);

    vector <DomElement *>           childs;
    vector <DomElement *>::iterator childs_i;

    childs.clear();
    element->getChilds(childs);
    for (childs_i = childs.begin();
        childs_i != childs.end();
        childs_i++)
    {
        DomDocument::deleteElementSafe(*childs_i);
    }
}

void DomDocument::deleteChilds(DomElement *element)
{
    assert(element);

    vector <DomElement *>           childs;
    vector <DomElement *>::iterator childs_i;

    childs.clear();
    element->getChilds(childs);
    for (childs_i = childs.begin();
        childs_i != childs.end();
        childs_i++)
    {
        DomDocument::deleteElement(*childs_i);
    }
}

void DomDocument::deleteElement(DomElement *element)
{
    deleteElementSafe(element);
}

/*
 *  Unlink element from dom without update container
 *
 *  Don't forget delete element for avoid memory leaks
 *
 */
void DomDocument::unlinkElementSafe(DomElement *element)
{
    xmlUnlinkNode(element->m_node);
}

/*
 *  Unlink element from dom
 *
 *  Don't forget delete element for avoid memory leaks
 *
 */
void DomDocument::unlinkElement(DomElement *element)
{
//    char buffer[1024] = { 0x00 };
    unlinkElementSafe(element);

//    // Don't forget mark as dirty after any changes!
//    snprintf(buffer, sizeof(buffer),
//        "Call updateContainer()"
//        " after unlink"
//        " from element: '%s'\n",
//        getElementTreeXPath().c_str()
//    );
//    markAsDirty(buffer);
}

/*
 * Delete all document elements
 */
void DomDocument::deleteRootElement()
{
    DomElement *element = NULL;

    element = getRootElement();
    if (element){
        deleteElementSafe(element);
    }

    // unset root element
    xmlDocSetRootElement(m_doc, NULL);
}

void DomDocument::setRootElement(DomElement *element)
{
    xmlNodePtr root = element->m_node;
    xmlDocSetRootElement(m_doc, root);
}

void DomDocument::removeExpired(int32_t remove_all)
{
    DomElement *element = NULL;

    element = getRootElement();
    if (element){
        removeExpired(element, remove_all);
    }
}

void DomDocument::callExpiredTimers()
{
    DomElement *element = NULL;

    element = getRootElement();
    if (element){
        callExpiredTimers(element);
    }
}

void DomDocument::removeExpired(
    DomElement  *a_element,
    int32_t     a_remove_all)
{
    int32_t i;
    vector <DomElement *> items;
    time_t cur_time = time(NULL);

again:
    // expired_time - "0" if we need delete element after exit
    // expired_time - not exist - if element should be not deleted
    // expired_time - "182982982" - if element should be deleted after
    //                this Unix time
    items.clear();
    a_element->getElementsByXpath(
        "//*[@_expired]",
        items
    );

    for (i = 0; i < (int32_t)items.size(); i++){
        DomElement  *item           = items[i];
        time_t      expired_time    = item->getExpired();

        if ((expired_time && (cur_time > expired_time))
            || a_remove_all)
        {
            DomDocument::deleteElement(item);
            goto again;
        }
    }
}

void DomDocument::callExpiredTimers(
    DomElement  *a_element)
{
    int32_t i;
    vector <DomElement *> items;
    time_t cur_time = time(NULL);

    items.clear();
    a_element->getElementsByXpath(
        "//*[@_timer]",
        items
    );
    for (i = 0; i < (int32_t)items.size(); i++){
        DomElement  *item       = items[i];
        time_t      timer_time  = item->getTimer();

        if (timer_time && (cur_time > timer_time)){
            item->removeTimer();
            item->timer();
        }
    }
}

struct event_base * DomDocument::getEvBase()
{
    return base;
}

struct evdns_base * DomDocument::getEvDnsBase()
{
    return dns_base;
}

void DomDocument::shutdown()
{
    if (state == SHUTDOWN){
        goto out;
    }

    state = SHUTDOWN;
out:
    return;
}

void DomDocument::do_shutdown()
{
    vector <DomElement *> items;
    DomElement *root_element = NULL;

    root_element = getRootElement();
    if (!root_element){
        goto out;
    }

    // inform elements about shutdown (we are going shutdow)
    DomDocument::processAllElementsFromEnd(
        root_element,
        DomDocument::element_shutdown_cb
    );

    // remove expired items (again), because they may be arise
    // after 'shutdown' callback
    DomDocument::removeExpired();

    // save to file
    if (_loaded_from_file.size()){
        save2file(_loaded_from_file.c_str());
    }

out:
    event_base_loopbreak(getEvBase());
    return;
}

xmlNodePtr DomDocument::createNodeFromContent(
    const string    &a_content)
{
    string              content     = a_content;
    xmlNodePtr          new_node    = NULL;
    string::size_type   start_pos   = string::npos;
    string::size_type   end_pos     = string::npos;

    // remove xml prefix like this
    // <?xml version="1.0" encoding="UTF-8" ?>
    start_pos = content.find("<?xml");
    if (start_pos != string::npos){
        end_pos = content.find("?>", start_pos);
        if (end_pos != string::npos){
            content.erase(start_pos, end_pos - start_pos + 2);
        }
    }

    xmlDocPtr new_doc = xmlParseMemory(content.c_str(), content.size());
    if (!new_doc){
        PERROR_OBJ(this, "Cannot create element from content: '%s'\n",
            content.c_str());
        goto fail;
    }

    new_node = new_doc->children;
    if (!new_node){
        PERROR_OBJ(this, "Cannot create element from content: '%s'\n",
            content.c_str());
        goto fail;
    }

    // init objects model
    initObjects(new_node);

out:
    return new_node;

fail:
    new_node = NULL;
    goto out;
}

DomElement *DomDocument::createElementFromContent(
    const string &a_content)
{
    DomElement  *element        = NULL;
    xmlNodePtr  new_node        = NULL;
    int32_t     old_priveleges  = m_priveleges;

    // drop all priveleges to USE_NET_OBJECTS before objects will be created,
    // it used for safe us from network commands.
    // all elements what created from content must have minimal priveleges
    // 20110708 morik
    m_priveleges = USE_NET_OBJECTS;

    new_node = createNodeFromContent(a_content);
    while (new_node != NULL
        && new_node->type != XML_ELEMENT_NODE)
    {
        new_node = new_node->next;
    }

    if (!new_node || !new_node->name){
        PERROR_OBJ(this,
            "Cannot create element from content: '%s'\n",
            a_content.c_str()
    );
        goto fail;
    }

    element = (DomElement *)new_node->_private;

out:
    // restore m_priveleges
    m_priveleges = old_priveleges;
    return element;

fail:
    goto out;
}

string DomDocument::serialize(int32_t format_id)
{
    string      ret         = "<?xml version=\"1.0\"?>\n";
    DomElement  *element    = NULL;

    element = getRootElement();
    if (element){
        ret += element->serialize(format_id);
    }

    return ret;
}

string DomDocument::serializeForPublic(int32_t format_id)
{
    string      ret         = "<?xml version=\"1.0\"?>\n";
    DomElement  *element    = NULL;

    element = getRootElement();
    if (element){
        DomDocument::removeExpired(element, 1);
        ret += element->serializeForPublic(format_id);
    }

    return ret;
}

void DomDocument::dump2stderr(int32_t format_id)
{
    string res = serialize(format_id);
    PWARN("XML:\n%s\n", res.c_str());
}

int32_t DomDocument::save2file(const string &fname)
{
    int32_t err  = -1;
    FILE    *f   = NULL;
    string  text = "";
    ssize_t res  = 0;

    if (fname.empty()){
        PERROR("Internal error, fname is empty\n");
        goto fail;
    }

    f = fopen(fname.c_str(), "w");
    if (f == NULL){
        PERROR("Cannot open file for write: '%s\n",
            fname.c_str()
        );
        goto fail;
    }

    text = serializeForPublic(1);
    res  = fwrite(text.c_str(), text.size(), 1, f);

    if (res > ssize_t(0)){
        err = 0;
    } else {
        PERROR("Cannot serialize DomDocument"
            " to file: '%s'\n", fname.c_str());
    }
    fclose(f);

ret:
    return err;
fail:
    err = -1;
    goto ret;
};

DomElement * DomDocument::getElementByIno(
    const string &a_ino)
{
    DomElement      *ret = NULL;
    ElementsByInoIt m_elements_by_ino_it;

    m_elements_by_ino_it = m_elements_by_ino.find(a_ino);
    if (m_elements_by_ino.end() != m_elements_by_ino_it){
        ret = m_elements_by_ino_it->second;
    }

    return ret;
}

DomElement * DomDocument::getElementByXpath(
    const string    &a_xpath)
{
    DomElement *el = NULL;
    el = getElementByXpath(NULL, a_xpath);
    return el;
}

DomElement * DomDocument::getElementByXpath(
    DomElement      *a_el,
    const string    &a_xpath)
{
    DomElement *ret = NULL;

    vector <DomElement *>           res;
    vector <DomElement *>::iterator res_it;

    res.clear();
    getElementsByXpath(a_el, a_xpath, res);
    if (res.size() == 1){
        ret = res[0];
    } else if (res.size() > 1){
        ret = res[0];
        string elements;
        for (res_it = res.begin();
            res_it != res.end();
            res_it++)
        {
            elements += "element: '";
            elements += (*res_it)->serialize(1);
            elements += "'\n";
        }
        PWARN("More that one element (%d)"
            " was found for XPATH: '%s'\n"
            " search path: '%s'\n"
            " elements dumps below:\n%s\n",
            res.size(),
            a_xpath.c_str(),
            a_el
                ?   a_el->serialize(1).c_str()
                :   "/",
            elements.c_str()
        );
    }
    return ret;
}

void DomDocument::getElementsByXpath(
    const string            &a_xpath,
    vector <DomElement *>   &a_out)
{
    getElementsByXpath(
        NULL,
        a_xpath,
        a_out
    );
}

void DomDocument::getElementsByXpath(
    DomElement              *a_el,
    const string            &a_xpath,
    vector <DomElement *>   &a_out)
{
    int32_t i, size = 0;
    xmlXPathContextPtr  xpathCtx;
    xmlXPathObjectPtr   xpathObj;
    xmlNodeSetPtr       nodes;
    xmlNodePtr          cur;
    DomElement          *curElement;
    string              xpath = a_xpath;

    // some time shit happens
    assert(a_xpath.size());

    // Create xpath evaluation context
    xpathCtx = xmlXPathNewContext(m_doc);
    if (NULL == xpathCtx){
        PERROR("Error: unable to create new XPath context\n");
        goto fail;
    }

    // setup node for search
    if (a_el){
        xpathCtx->node = a_el->m_node;
        // From the documentation of XPath 1.0
        // '//para' selects all the para descendants of the document root
        // To get all Identifier descendants of the current node you want
        // $node->findnodes('.//Identifier');
        // but since Identifier is a direct child of Response you can just write
        // $node->findnodes('Identifier');
        if (    2   <= xpath.size()
            &&  '/' == xpath.at(0)
            &&  '/' == xpath.at(1))
        {
            xpath = "." + xpath;
        }
    }

    // Evaluate xpath expression
//    xpathObj = xmlXPathEvalExpression(
    xpathObj = xmlXPathEval(
        (const xmlChar*)xpath.c_str(),
        xpathCtx
    );
    if (NULL == xpathObj){
        PERROR("Error: unable to evaluate xpath expression \"%s\"\n",
            xpath.c_str()
        );
        xmlXPathFreeContext(xpathCtx);
        goto fail;
    }

    nodes = xpathObj->nodesetval;
    if (!nodes){
        goto out;
    }

    size = nodes->nodeNr;

    for (i = 0; i < size; ++i){
        assert(nodes->nodeTab[i]);
        if (nodes->nodeTab[i]->type != XML_ELEMENT_NODE){
            continue;
        }
        cur         = nodes->nodeTab[i];
        curElement  = (DomElement *)cur->_private;
        if (curElement == NULL){
            continue;
        }
        a_out.push_back(curElement);
    }
    xmlXPathFreeContext(xpathCtx);

out:
    return;
fail:
    goto out;
}

int32_t DomDocument::isLoaded()
{
    return is_loaded;
}

/*
   temporary disabled 20110520

void DomDocument::deleteElement(DomElement *element)
{
    if (element == NULL){
        return;
    }
    if (element == getRootElement()){
        // it's root element delete document
        delete this;
    } else {
        // delete element and him objects
        deleteObjects(element->m_node->children);
        xmlUnlinkNode(element->m_node);
        xmlFreeNode(element->m_node);
    }
}
*/

// -------------------------------------------------- MORIK

DomElement *DomDocument::getSignal(
    const string    &signal_name,
    const string    &object_type,
    int32_t         create_if_not_exist)
{
    vector<DomElement *> childs;
    vector<DomElement *>::iterator childs_i;
    SignalsObject   *signals    = NULL;
    DomElement      *signal     = NULL;

    assert(signal_name.size());
    assert(object_type.size());

    IPNoiseObject *ipnoise = (IPNoiseObject *)getRootElement();
    assert(ipnoise);

    // get signals object
    signals = ipnoise->getSignalsObject();

    // search current signal, maybe already exist?
    childs.clear();
    signals->getChilds(childs);
    for (childs_i = childs.begin();
        childs_i != childs.end();
        childs_i++)
    {
        DomElement *element = *childs_i;

        string cur_signal_name = element->getAttribute("signal_name");
        string cur_object_type = element->getAttribute("object_type");

        if (    not cur_signal_name.empty()
            &&  not cur_object_type.empty())
        {
            if (    signal_name == cur_signal_name
                &&  object_type == cur_object_type)
            {
                // signal already registered
                signal = element;
                break;
            }
        }
        signal = NULL;
    }

    if (signal == NULL
        && create_if_not_exist)
    {
        signal = createElement("ipn_signal");
        signals->appendChildSafe(signal);
    }

    return signal;
}

void DomDocument::registrySignal(
    const string    &signal_name,
    const string    &object_type)
{
    DomElement *signal = NULL;

    // get signal and create it if not exist
    signal = getSignal(signal_name, object_type, 1);
    PWARN("registry signal: '%s', object_type: '%s'\n",
        signal_name.c_str(),
        object_type.c_str()
    );
    signal->setAttributeSafe("signal_name", signal_name);
    signal->setAttributeSafe("object_type", object_type);

    return;
}

void DomDocument::emitSignal(
    const string    &signal_name,
    DomElement      *object)
{
    int32_t i;
    char buffer[1024] = { 0x00 };
    vector<DomElement *>    res;
    vector<DomElement *>    items;
    vector<DomElement *>::iterator items_i;
    DomElement      *signal_obj  = NULL;
    DomElement      *signal      = NULL;
    IPNoiseObject   *ipnoise     = NULL;
    string          object_type;

    ipnoise = (IPNoiseObject *)getRootElement();
    assert(ipnoise);
    assert(object != NULL);

    object_type = object->getTagName();

    PDEBUG(20, "Attempt to emitSignal:"
        " signal_name: '%s', object_type: '%s'\n",
        signal_name.c_str(),
        object_type.c_str()
    );

    assert(not signal_name.empty());
    assert(not object_type.empty());

    // check what signal exist
    signal = getSignal(signal_name, object_type);
    if (!signal){
        PERROR("Signal not exist:"
            " signal_name: '%s', object_type: '%s'\n",
            signal_name.c_str(), object_type.c_str()
        );
        assert(0);
    }

    // 1. create signal object
    signal_obj = createElement("ipn_signal_obj");

    signal_obj->setAttributeSafe("data_ino", object->getIno());
    signal->appendChildSafe(signal_obj);

    // 2. do search all slots for this signal
    // and add signal_obj references
    snprintf(buffer, sizeof(buffer),
        "//ipn_slots/ipn_slot"
        "[@slot_name=\"%s\" and @object_type=\"%s\"]"
        "/ipn_slot_obj",
        signal_name.c_str(),
        object_type.c_str()
    );
    items.clear();
    getElementsByXpath(buffer, items);
    PDEBUG(20, "Was found '%d' available slots"
        " for signal: '%s', object_type: '%s'\n",
        int(items.size()),
        signal_name.c_str(),
        object_type.c_str()
    );

    for (i = 0; i < int(items.size()); i++){
        DomElement *process_signal_obj  = NULL;
        DomElement *slot_obj            = items[i];
        assert(slot_obj != NULL);

        process_signal_obj = ipnoise->getDocument()->createElement(
            "ipn_process_signal_obj"
        );
        process_signal_obj->setAttributeSafe(
            "from_ino",
            signal_obj->getIno()
        );
        slot_obj->appendChildSafe(process_signal_obj);
    }

    // do sync deliver
    deliverSignals();
    return;
}

void DomDocument::emitSignalAndDelObj(
    const string    &signal_name,
    DomElement      *object)
{
    assert(object != NULL);
    object->deleteAfterDeliver(1);
    emitSignal(signal_name, object);
}

void DomDocument::registrySlot(
    DomElement      *element,
    const string    &slot_name,
    const string    &object_type)
{
    char buffer[1024] = { 0x00 };
    vector<DomElement *> res;
    vector<DomElement *>::iterator res_i;

    DomElement *signal      = NULL;
    DomElement *slot        = NULL;
    DomElement *slot_obj    = NULL;

    signal = getSignal(slot_name, object_type);
    if (!signal){
        PERROR("Signal not exist: signal_name: '%s',"
            " object_type: '%s'\n",
            slot_name.c_str(), object_type.c_str()
        );
        goto fail;
    }

    slot = getSlot(slot_name, object_type, 1);
    slot->setAttributeSafe("slot_name", slot_name);
    slot->setAttributeSafe("object_type", object_type);

    // check what not exist
    snprintf(buffer, sizeof(buffer),
        "slot_obj[to=\"%s\"]",
        element->getAttribute("_ino").c_str()
    );

    res.clear();
    slot->getElementsByXpath(buffer, res);
    if (res.size()){
        goto ret;
    }

    // create
    slot_obj = createElement("ipn_slot_obj");
    slot_obj->setAttributeSafe("to_ino", element->getAttribute("_ino"));
    slot->appendChildSafe(slot_obj);

ret:
    return;
fail:
    goto ret;
}

void DomDocument::deliverSignals()
{
    int32_t i;
    char buffer[1024] = { 0x00 };
    vector<DomElement *>    res;
    vector<DomElement *>    items;
    IPNoiseObject   *ipnoise            = NULL;
    DomElement      *process_signal_obj = NULL;
    string          processing          = "";
    ipnoise = (IPNoiseObject *)getRootElement();
    assert(ipnoise);

again:
    // search signal object for processing
    process_signal_obj = NULL;

    snprintf(buffer, sizeof(buffer),
        "//ipn_slots/ipn_slot/ipn_slot_obj/ipn_process_signal_obj"
    );
    items.clear();
    ipnoise->getElementsByXpath(buffer, items);

    for (i = 0; i < int32_t(items.size()); i++){
        DomElement *cur_item = items[i];

        // already in progress?
        processing = cur_item->getAttribute("processing");
        if (not processing.empty()){
            // delivering signal to this slot already in progress, skip it
            continue;
        }
        process_signal_obj = cur_item;

        // mark what we are processing this signal
        process_signal_obj->setAttributeSafe("processing", "1");
        break;
    }

    if (process_signal_obj){
        string      sig_ino             = "";
        string      deliver_id          = "";
        string      slot_name           = "";
        string      object_type         = "";
        DomElement *deliver             = NULL;
        DomElement *signal_obj          = NULL;
        DomElement *slot                = NULL;
        DomElement *slot_obj            = NULL;
        DomElement *data_element        = NULL;

        // get slot_obj
        slot_obj = process_signal_obj->getParentNode();
        assert(slot_obj);

        // get slot
        slot = slot_obj->getParentNode();
        assert(slot);

        // get slot(signal) name
        slot_name = slot->getAttribute("slot_name");
        assert(slot_name.size() > 0);

        // get object type
        object_type = slot->getAttribute("object_type");
        assert(object_type.size() > 0);

        // get deliver ID
        deliver_id = slot_obj->getAttribute("to_ino");
        assert(deliver_id.size() > 0);

        // get deliver
        snprintf(buffer, sizeof(buffer),
            "//*[@_ino=\"%s\"]",
            deliver_id.c_str()
        );
        deliver = ipnoise->getElementByXpath(buffer);

        // get signal object ID
        sig_ino = process_signal_obj->getAttribute("from_ino");
        assert(sig_ino.size() > 0);

        // try to search signal_obj
        snprintf(buffer, sizeof(buffer),
            "//*[@_ino=\"%s\"]",
            sig_ino.c_str()
        );

        signal_obj = ipnoise->getElementByXpath(buffer);
        if (!signal_obj){
            PWARN("Signal was removed, but no all slots was processed,"
                " xpath: '%s', content: '%s'\n",
                buffer,
                ipnoise->serialize().c_str()
            );
            // signal was removed
            goto signal_was_removed;
        }

        // try to search signal data element
        snprintf(buffer, sizeof(buffer),
            "//*[@_ino=\"%s\"]",
            signal_obj->getAttribute("data_ino").c_str()
        );
        data_element = ipnoise->getElementByXpath(buffer);

        //PDEBUG(20, "attept to deliver signal\n"
        //    "deliver:       '%x'\n"
        //    "slot_name:     '%s'\n"
        //    "signal_obj:    '%x'\n"
        //    "object_type:   '%s'\n"
        //    "data_element:  '%x'\n"
        //    "signal_obj->getAttribute('data_ino'): '%s'\n",
        //    (int32_t)deliver,
        //    slot_name.c_str(),
        //    (int32_t)signal_obj,
        //    object_type.c_str(),
        //    (int32_t)data_element,
        //    signal_obj->getAttribute("data_ino").c_str()
        //);

        PDEBUG(20, "search deliver\n");
        if (deliver                 // destination slot
            && slot_name.size()     // signal name
            && signal_obj           // signal object
            && object_type.size()   // type of argument
            && data_element)        // signal argument
        {
            //PDEBUG(20, "call deliver,"
            //    " slot_name: '%s', data_element: '0x%x'\n",
            //    slot_name.c_str(),
            //    int32_t(data_element)
            //);

            deliver->slot(
                slot_name.c_str(),
                data_element
            );
        }
        PDEBUG(20, "after call deliver\n");

signal_was_removed:

        // signal handled, remove from slot
        // it MUST be before deliver for avoid dead locks
        DomDocument::deleteElementSafe(process_signal_obj);

        // search other slots what waiting this signal
        snprintf(buffer, sizeof(buffer),
            "//ipn_slots/ipn_slot/ipn_slot_obj/"
            "ipn_process_signal_obj[@from_ino=\"%s\"]",
            sig_ino.c_str()
        );
        res.clear();
        ipnoise->getElementsByXpath(buffer, res);

        if (!res.size()){
            // there are no more slots what wait this signal,
            // delete signal obj and signal data if requested
            if (signal_obj){
                // it's last slot, delete signal obj
                DomDocument::deleteElementSafe(signal_obj);
            }

            // delete signal data if requested
            if (data_element && data_element->deleteAfterDeliver()){
                DomDocument::deleteElementSafe(data_element);
            }
        }

        goto again;
    }
}

/*
 * Delete all signal_obj'ects what have not references
 *
 */
void DomDocument::removeSignalObjects()
{
    int32_t i;
    char buffer[1024] = { 0x00 };
    vector<DomElement *>    res;
    vector<DomElement *>    items;
    IPNoiseObject *ipnoise = NULL;

    ipnoise = (IPNoiseObject *)getRootElement();
    assert(ipnoise);

again:
    // 1. If signal object have not depend slots, remove it
    snprintf(buffer, sizeof(buffer),
        "//ipn_signals/ipn_signal/ipn_signal_obj"
    );
    items.clear();
    ipnoise->getElementsByXpath(buffer, items);
    for (i = 0; i < int32_t(items.size()); i++){
        DomElement *signal_obj = items[i];
        assert(signal_obj != NULL);

        snprintf(buffer, sizeof(buffer),
            "//ipn_slots/ipn_slot/ipn_process_signal_obj[@from=\"%s\"]",
            signal_obj->getIno().c_str()
        );
        res.clear();
        ipnoise->getElementsByXpath(buffer, res);
        if (!res.size()){
            // this signal object have not pending slots,
            // so remove it
            DomDocument::deleteElementSafe(signal_obj);
            goto again;
        }
    }
    return;
}

DomElement *DomDocument::getSlot(
    const string    &slot_name,
    const string    &object_type,
    int32_t         create_if_not_exist)
{
    vector<DomElement *> childs;
    vector<DomElement *>::iterator childs_i;
    SlotsObject     *slots      = NULL;
    DomElement      *slot       = NULL;
    IPNoiseObject   *ipnoise    = NULL;

    ipnoise = (IPNoiseObject *)getRootElement();
    assert(ipnoise);
    assert(slot_name.size());
    assert(object_type.size());

    // get slots object
    slots = ipnoise->getSlotsObject();

    // search current slot, maybe already exist?
    childs.clear();
    slots->getChilds(childs);
    for (childs_i = childs.begin();
        childs_i != childs.end();
        childs_i++)
    {
        DomElement *element = *childs_i;

        string cur_slot_name = element->getAttribute("slot_name");
        string cur_object_type = element->getAttribute("object_type");

        if (    not cur_slot_name.empty()
            &&  not cur_object_type.empty())
        {
            if (    slot_name   == cur_slot_name
                &&  object_type == cur_object_type)
            {
                // slot already registered
                slot = element;
                break;
            }
        }
        slot = NULL;
    }

    if (slot == NULL
        && create_if_not_exist)
    {
        slot = createElement("ipn_slot");
        slots->appendChildSafe(slot);
    }
    return slot;
}

void DomDocument::markContainerDirty(
    DomElement      *a_el,
    const string    &a_debug)
{
    if (not a_el->isContainer()){
        PERROR("markContainerDirty called,"
            " but element is not container\n"
        );
        goto out;
    }

    m_dirty_containers[a_el->getIno()] = a_debug;

out:
    return;
}

