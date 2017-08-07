/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <ipnoise-common/strings.h>

#include "inode.h"
#include "libxml2/domDocument.h"

#define min(x, y) ((x) < (y) ? (x) : (y))

#include "objects/serverObject.h"
#include "objects/userObject.h"

#include "libxml2/domElement.h"

/*
 *
 * Note:
 *
 * 1. XXXSafe methods should not call markAsDirty method
 *      Example:
 *          DomElement::setAttribute        - with call markAsDirty
 *          DomElement::setAttributeSafe    - without call markAsDirty
 *
 * 2. All system attrubutes, that will created here, must have "_" prefix,
 *      example: "_ino", "_expired", "_timer" etc.
 *
 */

static uint32_t  ino_next_id = 1;

DomElement::DomElement(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &_tagname)
{
    char buffer[1024]       = { 0x00 };
    m_node                  = a_node;
    m_doc                   = a_doc;
    m_dom_autorun_called    = false;

    if (m_doc == NULL){
        PERROR_OBJ(this, "Document is NULL\n");
        goto ret;
    }
    if (m_node == NULL){
        m_node = xmlNewDocNode(
            m_doc->m_doc,
            NULL,
            (const xmlChar*)_tagname.c_str(),
            NULL
        );
    }
    m_node->_private  = (void *)this;

    // setup ID
    snprintf(buffer, sizeof(buffer), "%x", ino_next_id);
    setAttributeSafe("_ino", buffer);

    // prepare next element ID
    ino_next_id += INODE_NEXT_ELEMENT_OFFSET;

ret:
    return;
}

DomElement::~DomElement()
{
}

bool DomElement::DomAutorunCalled()
{
    return m_dom_autorun_called;
}

void DomElement::DomAutorunCalled(
    const bool &a_val)
{
    m_dom_autorun_called = a_val;
}

string DomElement::serializeForPublic(
    const int &a_format_id)
{
    int i;
    vector <DomElement *> els;
    DomElement *element         = NULL;
    string serialized_content   = "";
    string ret                  = "";

    // serialize current document
    serialized_content = serialize(a_format_id);

    // create new element from serialized content
    element = getDocument()->createElementFromContent(serialized_content);

    // remove attributes and other system information
    // from all elements
    els.clear();
    element->getAllChilds(els);
    for (i = 0; i < (int)els.size(); i++){
        DomElement *el= NULL;
        el = els[i];
        el->delAttributeSafe("_ino");
    }
    element->delAttributeSafe("_ino");

    // serialize again
    ret = element->serialize(a_format_id);

    // delete element
    DomDocument::deleteElement(element);

    return ret;
}

string DomElement::serialize(
    const int &a_format_id)
{
    string ret = "";

    xmlBufferPtr buf = xmlBufferCreate();
    if (buf == NULL){
        PERROR_OBJ(this, "%s\n", "xmlBufferCreate() failed");
        goto ret;
    }

    xmlNodeDump(
        buf,            // the XML buffer output
        m_doc->m_doc,   // the document
        m_node,         // the current node
        -1,             // the imbrication level for indenting
        a_format_id     // text formatting style (0 - disabled)
    );

    ret = (char *)xmlBufferContent(buf);
    xmlBufferFree(buf);

ret:
    return ret;
}

/*
 *  Search current element owner
 *  (User what contain this DOM element)
 *
 */
UserObject *DomElement::getOwner()
{
    UserObject *user    = NULL;
    DomElement *element = this;

    element = getParentNodeByTagName("ipn_user");
    if (element){
        user = (UserObject *)element;
    }

    return user;
}

ServerObject *DomElement::getServer()
{
    ServerObject *server = NULL;

    server = (ServerObject *)getParentNodeByTagName(
        "ipn_server"
    );

    return server;
}

string DomElement::getOwnerHuid()
{
    string      huid;
    UserObject  *owner = NULL;

    owner = getOwner();
    if (owner){
        huid = owner->getHuid();
    }

    return huid;
}

uint32_t DomElement::getContainerVersion()
{
    return getAttributeFromHex("_container_ver");
}

void DomElement::setContainerVersion(
    const uint32_t &a_ver)
{
    setAttributeHexSafe("_container_ver", a_ver);
}

bool DomElement::isContainer()
{
    bool    ret = false;
    string  ver = getAttribute("_container_ver");

    if (not ver.empty()){
        ret = true;
    }

    return ret;
}

void DomElement::getContainerInfo(
    Path    &a_container_path,
    string  &a_container_name)
{
    string::size_type   i    = 0;
    string              path = getElementTreeXPath();
    Splitter            parts(path, "/");

    a_container_path = Path(DEFAULT_CONTAINERS_DIR);
    a_container_name = "";

    for (i = 0; i < parts.size(); i++){
        string              part        = parts[i];
        string::size_type   pos_first   = string::npos;
        string::size_type   pos_last    = string::npos;

        // clear container name
        a_container_name = "";

        // check part size
        if (not part.size()){
            // skip empty path
            continue;
        }

        pos_first = part.find('[');
        pos_last  = part.rfind(']');

        if (    string::npos != pos_first
            &&  string::npos != pos_last
            &&  pos_last > pos_first)
        {
            a_container_name = part.substr(
                pos_first + 1,
                pos_last  - pos_first - 1
            );
        }

        // remove all after '['
        if (string::npos != pos_first){
            part.resize(pos_first);
        }

        if (not part.size()){
            PERROR("Cannot parse path: '%s'\n",
                path.c_str()
            );
            goto fail;
        }

        // add part to container path
        a_container_path.addPath(part);
    }

out:
    return;
fail:
    goto out;
}

void DomElement::saveContainer(
    const string &a_debug)
{
    DomElement *container   = NULL;
    string      path        = getElementTreeXPath();
    string      data;

    if (not path.size()){
        PERROR("Empty path while saveContainer (%s)\n",
            a_debug.c_str()
        );
        goto fail;
    }

    if (not isContainer()){
        PERROR("Attempt to save not container"
            " path: '%s' (%s)\n",
            path.c_str(),
            a_debug.c_str()
        );
        goto fail;
    }

    // clone
    container = this->clone();

    // remove _container_dirty flag
    container->delAttributeSafe("_container_dirty");

    // remove expired
    DomDocument::removeExpired(container, 1);

    // inform elements about shutdown
    DomDocument::processAllElementsFromEnd(
        container,
        DomDocument::element_shutdown_cb
    );

    // remove expired items (again),
    // because they may be arise after 'shutdown' callback
    DomDocument::removeExpired(container, 1);

    // process deeper containers
    DomDocument::processAllChildsFromEnd(
        container,
        DomElement::clear_containers
    );

    // serialize
    data += "<ipn_container>";
    data +=     "<ipn_container_data>";
    data +=         container->serializeForPublic(0);
    data +=     "</ipn_container_data>";
    data += "</ipn_container>";

    // delete cloned element
    DomDocument::deleteElementSafe(container);

    PDEBUG(20,
        "request save container: '%s',"
        " container version: '%u'\n"
        " info: '%s', "
        " data:\n'%s'\n",
        path.c_str(),
        getContainerVersion(),
        a_debug.c_str(),
        data.c_str()
    );

    // ok, now calculate save path
    {
        Path    container_path;
        Path    container_path_tmp;
        string  container_name;

        // get container info
        getContainerInfo(
            container_path,
            container_name
        );

        // create dir
        container_path.mkdirr();

        // add container name
        container_path.addPath(container_name);

        // create tmp path
        container_path_tmp = container_path.path() + ".tmp";

        // write container to file
        do {
            size_t  nums    = 0;
            FILE    *file   = container_path_tmp.fopen("w");

            if (not file){
                PERROR("Cannot open: '%s' for write\n",
                    container_path_tmp.path().c_str()
                );
                goto fail;
            }

            // save to file
            nums = fwrite(
                data.c_str(),
                data.size(),
                1,
                file
            );
            if (nums){
                // success, remove _container_dirty flag
                delAttributeSafe("_container_dirty");
                PDEBUG(10, "container saved: path: '%s',"
                    " name: '%s'\n",
                    container_path_tmp.path().c_str(),
                    container_name.c_str()
                );
            } else {
                PERROR("cannot save container: path: '%s',"
                    " name: '%s'\n",
                    container_path_tmp.path().c_str(),
                    container_name.c_str()
                );
            }
            fclose(file);

            // go out if error
            if (not nums){
                break;
            }

            // rename from tmp
            rename(
                container_path_tmp.path().c_str(),
                container_path.path().c_str()
            );
        } while (0);
    }

out:
    return;
fail:
    goto out;
}

void DomElement::clear_containers(
    DomElement *a_el,
    int32_t)   // deep
{
    vector<DomElement *>            res;
    vector<DomElement *>::iterator  res_it;

    if (not a_el->isContainer()){
        goto out;
    }

    // remove _container_dirty flag
    a_el->delAttributeSafe("_container_dirty");

    // remove _container_ver attribute
    a_el->delAttributeSafe("_container_ver");

    // remove container's childs
    a_el->getChilds(res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        DomElement *cur_el = *res_it;
        DomDocument::deleteElement(cur_el);
    }

out:
    return;
}

void DomElement::markAsDirty(
    const string    &a_debug)
{
    DomElement *el = this;

    if (not getDocument()->isLoaded()){
        goto out;
    }

    while (el && not el->isContainer()){
        el = el->getParentNode();
    }

    if (el){
        uint32_t version = 0;
        // mark container dirty
        el->setAttributeSafe("_container_dirty", "true");
        // update container version
        version = el->getContainerVersion();
        el->setContainerVersion(version + 1);
        // add us in save queue
        getDocument()->markContainerDirty(el, a_debug);
    }

out:
    return;
}

/*
 *  Append CDATA element with provided information to current element
 *  (safe call without update container)
 *
 */
void DomElement::appendCDataBlockSafe(const string &data)
{
    xmlNodePtr new_node = NULL;

    new_node = xmlNewCDataBlock(
        m_doc->m_doc,
        (const xmlChar *)data.c_str(),
        int(data.size())
    );
    xmlAddChild(m_node, new_node);
}

/*
 *  Append CDATA element with provided information to current element
 *
 */
void DomElement::appendCDataBlock(const string &data)
{
    char buffer[1024] = { 0x00 };
    appendCDataBlockSafe(data);
    // Don't forget mark as dirty after any changes!
    snprintf(buffer, sizeof(buffer),
        "Call markAsDirty()"
        " after appendCDataBlock('%s');"
        " from element: '%s'\n",
        data.c_str(),
        getElementTreeXPath().c_str()
    );
    markAsDirty(buffer);
}

string DomElement::getData()
{
    string ret = "";
    xmlChar * content = NULL;

    content = xmlNodeGetContent(m_node);
    if (content){
        ret = (char *)content;
    }

    return ret;
}

void DomElement::setData(const string &a_data)
{
    const xmlChar * content = (const xmlChar *)a_data.c_str();

    xmlNodeSetContent(m_node, content);
}

string DomElement::dump(
    const int &a_format_id)
{
    string res;
    res = serialize(a_format_id);
    return res;
}

void DomElement::dump2stderr(
    const int &a_format_id)
{
    string res;
    res = dump(a_format_id);
    PDEBUG(5, "XML:\n%s\n", res.c_str());
}

DomDocument *DomElement::getDocument()
{
    return m_doc;
}

string DomElement::getTagName()
{
    return (const char*)m_node->name;
}

xmlElementType DomElement::getXmlType()
{
    return m_node->type;
}

DomElement *DomElement::prevSibling()
{
    DomElement *element = NULL;
    xmlNodePtr n        = NULL;

    n = m_node->prev;
    while (n != NULL && n->type != XML_ELEMENT_NODE){
        n = n->prev;
    }
    if (n){
        element = (DomElement *)n->_private;
    }

    return element;
}

DomElement *DomElement::nextSibling()
{
    DomElement *element = NULL;
    xmlNodePtr n        = NULL;

    n = m_node->next;
    while (n != NULL && n->type != XML_ELEMENT_NODE){
        n = n->next;
    }
    if (n){
        element = (DomElement *)n->_private;
    }

    return element;
}

/*  Static
 *  Get Element FULL XPATH
 *
 */
string DomElement::getElementTreeXPath(
    DomElement  *a_element,
    const int   &a_stop_if_owner)
{
    char buffer[256];
    string ret = "";
    vector<string>              paths;
    vector<string>::iterator    paths_i;
    DomElement *rootElement = NULL;

    // sometime world is ugly
    assert(a_element);

    // get root
    rootElement = a_element->getDocument()->getRootElement();
    assert(rootElement);

    for (;
        a_element && XML_ELEMENT_NODE == a_element->getXmlType();
        a_element = a_element->getParentNode())
    {
        int index           = 0;
        DomElement *sibling = NULL;

        if (a_stop_if_owner
            && "ipn_user" == a_element->getTagName())
        {
            break;
        }

        if ("ipn_skbuff" == a_element->getTagName())
        {
            break;
        }

        for (sibling = a_element->prevSibling();
            sibling != NULL;
            sibling = sibling->prevSibling())
        {
            if (sibling->getTagName() == a_element->getTagName()){
                ++index;
            }
        }

        string tagName = a_element->getTagName();
        // calculate element index
        snprintf(buffer, sizeof(buffer),
            "[%d]",
            index + 1
        );
        paths.insert(
            paths.begin(),
            string(tagName) + string(buffer)
        );
        if (rootElement == a_element){
            break;
        }
    }

    for (paths_i = paths.begin();
        paths_i != paths.end();
        paths_i++)
    {
        ret += string("/") + *paths_i;
    }

    return ret;
};

/*  Static
 *  Get Element XPATH inside owner user
 *
 */
string DomElement::getElementOwnerTreeXPath(
    DomElement *a_element)
{
    return getElementTreeXPath(a_element, 1);
};

string DomElement::getElementTreeXPath()
{
    DomElement *element = this;
    return DomElement::getElementTreeXPath(element);
}

/*  Get elements by XPath below, current element
 *
 */
void DomElement::getElementsByXpath(
    const string            &a_xpath,
    vector <DomElement *>   &a_out)
{
    getDocument()->getElementsByXpath(
        this,
        a_xpath,
        a_out
    );
}

/*  Get first element by XPath, below current element
 *
 */
DomElement * DomElement::getElementByXpath(
    const string &xpath)
{
    DomElement *el = NULL;
    el = getDocument()->getElementByXpath(this, xpath);
    return el;
}

/*  Create recursive copy of element (New Document will created)
 *
 */
DomElement *DomElement::clone()
{
    string content = serialize(0);
    return getDocument()->createElementFromContent(content);
}

map <string, string> DomElement::getAllAttributes()
{
    xmlAttr *attribute      = NULL;
    xmlAttr *cur_attribute  = NULL;

    map <string, string> ret;

    attribute       = m_node->properties;
    cur_attribute   = attribute;

    while (cur_attribute){
        xmlBufferPtr buffer = xmlBufferCreate();
        if (!buffer){
            PERROR_OBJ(this, "%s\n", "Failed to create xml buffer");
            goto fail;
        }

        xmlNodeBufGetContent(buffer, cur_attribute->children);
        const char *key     = (const char *)cur_attribute->name;
        const char *value   = (char *)xmlBufferContent(buffer);
        ret[key] = value;
        xmlBufferFree(buffer);

        cur_attribute = cur_attribute->next;
        if (cur_attribute == attribute){
            break;
        }
    }

out:
    return ret;

fail:
    goto out;
}

string DomElement::getAttribute(const string &name)
{
    string      ret     = "";
    const char  *res    = NULL;

    res = (const char *)xmlGetProp(
        m_node,
        (const xmlChar *)name.c_str()
    );
    if (res){
        ret = res;
    }
    return ret;
}

int32_t DomElement::getAttributeInt(
    const string &a_name)
{
    return atoi(getAttribute(a_name).c_str());
}

uint32_t DomElement::getAttributeFromHex(
    const string &a_name)
{
    uint32_t res = 0;
    string   val = getAttribute(a_name);

    sscanf(val.c_str(), "%x", &res);
    return res;
}

/*
 *  Set attribute without container update
 *  (need for call from update_container context)
 *
 */
void DomElement::setAttributeSafe(
    const string    &a_name,
    const string    &a_value)
{
    xmlSetProp(m_node,
        (const xmlChar *)a_name.c_str(),
        (const xmlChar *)a_value.c_str()
    );
}

void DomElement::setAttributeHexSafe(
    const string    &a_name,
    const uint32_t  &a_value)
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer), "%x", a_value);
    setAttributeSafe(a_name, buffer);
}

void DomElement::setAttributeHex(
    const string    &a_name,
    const uint32_t  &a_value)
{
    char buffer[1024] = { 0x00 };

    setAttributeHexSafe(a_name, a_value);

    // Don't forget mark as dirty after any changes!
    snprintf(buffer, sizeof(buffer),
        "Call markAsDirty()"
        " after setAttribute('%s', '%d');"
        " from element: '%s'\n",
        a_name.c_str(), a_value,
        getElementTreeXPath().c_str()
    );
    markAsDirty(buffer);
}

void DomElement::setAttributeSafe(
    const string    &a_name,
    const int32_t   &a_value)
{
    char buffer[512] = { 0x00 };
    snprintf(buffer, sizeof(buffer), "%d", a_value);
    setAttributeSafe(a_name, buffer);
}

void DomElement::setAttribute(
    const string    &a_name,
    const string    &a_value)
{
    char buffer[1024] = { 0x00 };

    setAttributeSafe(a_name, a_value);

    // Don't forget mark as dirty after any changes!
    snprintf(buffer, sizeof(buffer),
        "Call markAsDirty()"
        " after setAttribute('%s', '%s');"
        " from element: '%s'\n",
        a_name.c_str(), a_value.c_str(),
        getElementTreeXPath().c_str()
    );
    markAsDirty(buffer);
}

void DomElement::setAttribute(
    const string    &a_name,
    const int       &a_value)
{
    char buffer[1024] = { 0x00 };

    setAttributeSafe(a_name, a_value);

    // Don't forget mark as dirty after any changes!
    snprintf(buffer, sizeof(buffer),
        "Call markAsDirty()"
        " after setAttribute('%s', '%d');"
        " from element: '%s'\n",
        a_name.c_str(), a_value,
        getElementTreeXPath().c_str()
    );
    markAsDirty(buffer);
}

void DomElement::delAttributeSafe(
    const string &a_name)
{
    xmlUnsetProp(m_node, (const xmlChar *)a_name.c_str());
}

void DomElement::delAttribute(
    const string &a_name)
{
    char buffer[1024] = { 0x00 };

    delAttributeSafe(a_name);

    // Don't forget mark as dirty after any changes!
    snprintf(buffer, sizeof(buffer),
        "Call markAsDirty()"
        " after delAttribute('%s');"
        " from element: '%s'\n",
        a_name.c_str(),
        getElementTreeXPath().c_str()
    );
    markAsDirty(buffer);
}

void DomElement::unsetAttributeSafe(
    const string &a_name)
{
    delAttribute(a_name);
}

void DomElement::unsetAttribute(
    const string &a_name)
{
    char buffer[1024] = { 0x00 };

    unsetAttributeSafe(a_name);

    // Don't forget mark as dirty after any changes!
    snprintf(buffer, sizeof(buffer),
        "Call markAsDirty()"
        " after unsetAttribute('%s');"
        " from element: '%s'\n",
        a_name.c_str(),
        getElementTreeXPath().c_str()
    );
    markAsDirty(buffer);
}

DomElement * DomElement::getParentNodeByTagName(
    const string &a_tag_name)
{
    DomElement *ret     = NULL;
    DomElement *element = NULL;

    if (a_tag_name.empty()){
        goto out;
    }

    // from current
    element = this;

    do {
        if (a_tag_name == element->getTagName()){
            ret = element;
            break;
        }
        element = element->getParentNode();
    } while (element);

out:
    return ret;
}

DomElement *DomElement::getParentNode()
{
    DomElement *ret     = NULL;
    xmlNodePtr parent   = NULL;

    // broken DomElement
    assert(m_node);

    parent = m_node->parent;
    if (m_node == parent){
        // we have not parent (parent it is we)
        goto out;
    }

    if (parent != NULL){
        ret = (DomElement *)parent->_private;
    }

out:
    return ret;
}

/*
 * Return first element child
 *
 * Must return only XML_ELEMENT_NODE
 */
DomElement *DomElement::getFirstChild()
{
    DomElement *ret     = NULL;
    xmlNodePtr child    = NULL;

    child = xmlFirstElementChild(m_node);
    while (child != NULL && child->type != XML_ELEMENT_NODE){
        child = child->next;
    }
    if (child){
        ret = (DomElement *)child->_private;
    }

    return ret;
}

/*
 * Return element childs from deep level 1
 *
 */
void DomElement::getChilds(vector<DomElement *> &a_out)
{
    DomElement *first_child = NULL;
    DomElement *child       = NULL;

    // must return only XML_ELEMENT_NODE
    first_child = getFirstChild();
    child       = first_child;
    do {
        if (child == NULL){
            break;
        }
        a_out.push_back(child);
        child = child->nextSibling();
    } while (child != first_child);
}

/*
 * Return element childs from deep level 1
 *
 */
void DomElement::getElementsByTagName(
    const string            &a_tag,
    vector<DomElement *>    &a_out)
{
    DomElement *first_child = NULL;
    DomElement *child       = NULL;

    // getFirstChild() must return only XML_ELEMENT_NODE
    first_child = getFirstChild();
    child       = first_child;
    do {
        if (child == NULL){
            break;
        }
        if (a_tag == child->getTagName()){
            a_out.push_back(child);
        }
        child = child->nextSibling();
    } while (child != first_child);
}

/*
 * Return element child from deep level 1
 *
 */
DomElement * DomElement::getElementByTagName(
    const string &a_tag)
{
    vector <DomElement *>childs;
    DomElement *first_child = NULL;
    DomElement *child       = NULL;
    DomElement *ret         = NULL;

    // getFirstChild must return only XML_ELEMENT_NODE
    first_child = getFirstChild();
    child       = first_child;
    do {
        if (child == NULL){
            break;
        }
        if (a_tag == child->getTagName()){
            childs.push_back(child);
        }
        child = child->nextSibling();
    } while (child != first_child);

    if (childs.size() > 1){
        PWARN_OBJ(this,
            "more that one child was found for tag: '%s'\n"
            "Dump below: '%s'\n",
            a_tag.c_str(),
            serialize().c_str()
        );
        ret = childs[0];
    } else if (childs.size() == 1){
        ret = childs[0];
    }

    return ret;
}

void DomElement::deleteChilds()
{
    DomDocument::deleteChilds(this);
}

void DomElement::deleteChildsSafe()
{
    DomDocument::deleteChildsSafe(this);
}

/*
 * Return ALL element childs
 *
 */
void DomElement::getAllChilds(
    DomElement              *a_element,
    vector <DomElement *>   &a_childs)
{
    vector <DomElement *>           res;
    vector <DomElement *>::iterator res_i;

    res.clear();
    a_element->getChilds(res);
    for (res_i = res.begin();
        res_i != res.end();
        res_i++)
    {
        DomElement *child = *res_i;
        a_childs.push_back(child);
        getAllChilds(child, a_childs);
    }
}

/*
 * Return ALL element childs
 *
 */
void DomElement::getAllChilds(vector<DomElement *> &a_out)
{
    getAllChilds(this, a_out);
}

/*
 * Return true if element was attached to DOM
 */
bool DomElement::isAttached()
{
    bool ret = false;

    DomElement *element         = NULL;
    DomElement *root_element    = NULL;

    root_element = getDocument()->getRootElement();
    if (root_element){
        // check what we attached
        element = this;
        while (element){
            if (element == root_element){
                ret = true;
                break;
            }
            element = element->getParentNode();
        }
    }

    return ret;
}

void DomElement::appendChildSafe(
    DomElement  *a_element,
    int32_t     a_call_autorun)
{
    // add child
    xmlAddChild(m_node, a_element->m_node);

    // if we are attached, call elements autorun
    // autorun him and him childs
    if (a_call_autorun && isAttached()){
        getDocument()->autorunObjects(a_element);
    }
}

void DomElement::appendChild(
    DomElement *a_element)
{
    char buffer[1024] = { 0x00 };
    appendChildSafe(a_element);

    // Don't forget mark as dirty after any changes!
    snprintf(buffer, sizeof(buffer),
        "Call markAsDirty()"
        " after appendChild('%s');"
        " from element: '%s'\n",
        a_element->getElementTreeXPath().c_str(),
        getElementTreeXPath().c_str()
    );
    markAsDirty(buffer);
}

void DomElement::registrySignals()
{
}

void DomElement::registrySlots()
{
}

void DomElement::slot(
    const string    &, // signal_name,
    DomElement      *) // object)
{
}

string DomElement::getIno()
{
    return getAttribute("_ino");
}

int DomElement::getInoInt()
{
    int     ret = 0;
    string  ino = getIno();
    if (not ino.empty()){
        sscanf(ino.c_str(), "%x", &ret);
    }
    return ret;
}

int DomElement::deleteAfterDeliver(
    int a_delete_after_deliver)
{
    int ret = 0;
    string attr        = "";
    string attr_name   = "delete_after_deliver";

    if (-1 == a_delete_after_deliver){
        // read request
        attr = getAttribute(attr_name);
        ret = (attr.empty()) ? 0 : 1;
        goto out;
    }

    if (0 == a_delete_after_deliver){
        unsetAttribute(attr_name);
        ret = 0;
    } else {
        setAttributeSafe(attr_name, "1");
        ret = 1;
    }

out:
    return ret;
}

int32_t DomElement::autorun()
{
    return 0;
}

void DomElement::shutdown()
{
}

void DomElement::expired()
{
}

void DomElement::timer()
{
}

void DomElement::prepareForContainer()
{
}

/*
 * Sort element childs from deep level 1 without update container
 *
 */
int DomElement::sortChildsSafe()
{
    vector < SortItem > childs;
    vector < SortItem >::iterator childs_i;

    vector < SortItem > sorted_childs;
    vector < SortItem >::iterator sorted_childs_i;

    DomElement *first_child = NULL;
    int changed             = 0;

    // first of all - deattach all childs
    first_child = getFirstChild();
    while (first_child){
        SortItem item;
        item.m_element = first_child;
        DomDocument::unlinkElementSafe(first_child);
        childs.push_back(item);
        sorted_childs.push_back(item);
        first_child = getFirstChild();
    }

    if (!childs.size()){
        goto ret;
    }

    // sort all elements
    sort(sorted_childs.begin(), sorted_childs.end());

    // append elements back to dom
    for (childs_i = sorted_childs.begin();
        childs_i != sorted_childs.end();
        childs_i++)
    {
        SortItem &item = *childs_i;
        appendChildSafe(item.m_element);
    }

    // check what elements have change positions
    // and call update container if it is

    for (childs_i       = childs.begin(),
        sorted_childs_i = sorted_childs.begin();
        childs_i != childs.end();
        childs_i++, sorted_childs_i++)
    {
        SortItem    &child          = *childs_i;
        SortItem    &sorted_child   = *sorted_childs_i;
        if (child.m_element != sorted_child.m_element){
            changed = 1;
            break;
        }
    }

ret:
    return changed;
}

time_t DomElement::getExpired()
{
    string  expired;
    time_t  expired_time = 0;

    expired = getAttribute("_expired");

    if (not expired.empty()){
        expired_time = (time_t)strtoull(expired.c_str(), NULL, 10);
    }

    return expired_time;
}

string DomElement::parseTime(const string &a_unix_time)
{
    string       time    = a_unix_time;
    ServerObject *server = NULL;

    server = getServer();

    // time - "182982982" - Unix time
    // time - "+30"       - now + 30 seconds
    if (    time.size()
        &&  '+' == time.at(0))
    {
        uint32_t    tmp_time        = 0;
        string      tmp_time_str    = time.substr(1);
        char        buffer[512]     = { 0x00 };
        sscanf(tmp_time_str.c_str(), "%u", &tmp_time);
        tmp_time += server->getCurTime();
        snprintf(buffer, sizeof(buffer), "%u", tmp_time);
        time = buffer;
    }
    return time;
}

void DomElement::unsetExpired()
{
    delAttributeSafe("_expired");
}

void DomElement::setExpired(const string &a_unix_time)
{
    string time = parseTime(a_unix_time);
    // expired_time - "0" if we need delete element after exit
    // expired_time - not exist - if element should be not deleted
    // expired_time - "182982982" - if element should be deleted after
    //                this Unix time
    // expired_time - "+30" - delete after now + 30 seconds
    setAttributeSafe("_expired", time);
}

time_t DomElement::getTimer()
{
    string  timer;
    time_t  timer_time = 0;

    timer = getAttribute("_timer");
    if (not timer.empty()){
        timer_time = (time_t)strtoull(timer.c_str(), NULL, 10);
    }

    return timer_time;
}

void DomElement::setTimer(const string &a_unix_time)
{
    string time = parseTime(a_unix_time);
    // time - "182982982" - call after this Unix time
    // time - "+30"       - call after now + 30 seconds
    setAttributeSafe("_timer", time);
}

void DomElement::removeTimer()
{
    delAttributeSafe("_timer");
}

/*
 * Sort element childs from deep level 1
 *
 */
int DomElement::sortChilds()
{
    char buffer[1024] = { 0x00 };
    int changed = 0;

    changed = sortChildsSafe();
    if (changed){
        // Don't forget mark as dirty after any changes!
        snprintf(buffer, sizeof(buffer),
            "Call markAsDirty()"
            " after sortChilds()"
            " from element: '%s'\n",
            getElementTreeXPath().c_str()
        );
        markAsDirty(buffer);
    }

    return changed;
}


int DomElement::cmp(class DomElement *a_right)
{
    int ret = 0;

    int id          = getInoInt();
    int right_id    = a_right->getInoInt();

    if (right_id != id){
        // id not equal
        if (right_id > id){
            ret = +1;
        } else {
            ret = -1;
        }
    }

    return ret;
}

// ------------------------ Fuse filesystem ---------------------------------

string DomElement::parseFuseName(const char *a_fuse_name)
{
    return a_fuse_name;
}

string DomElement::getFuseName()
{
    return getTagName();
}

string DomElement::getFuseFullName()
{
    char buffer[1024]       = { 0x00 };
    int index               = 0;
    DomElement *parent      = NULL;
    DomElement *cur_element = NULL;
    DomElement *sibling     = NULL;
    int i, count = 0;

    vector<DomElement *> items;

    parent = getParentNode();
    if (parent){
        items.clear();
        parent->getChilds(items);
    }

    for (i = 0; i < int(items.size()); i++){
        cur_element = items[i];
        if (cur_element->getFuseName() == getFuseName()){
            count++;
        }
    }

    cur_element = this;
    for (sibling = cur_element->prevSibling();
        sibling != NULL;
        sibling = sibling->prevSibling())
    {
        if (sibling->getFuseName() == cur_element->getFuseName()){
            ++index;
        }
    }

    if (count > 1){
        snprintf(buffer, sizeof(buffer), "%s[%d]",
            getFuseName().c_str(),
            index + 1
        );
    } else {
        snprintf(buffer, sizeof(buffer), "%s",
            getFuseName().c_str()
        );
    }

    return string(buffer);
}


void DomElement::closeAllFuseClients()
{
    FuseClient  *fuse_client = NULL;
    FUSE_CLIENTS::iterator fuse_clients_i;

    while (fuse_clients.size()){
        fuse_clients_i  = fuse_clients.begin();
        fuse_client     = fuse_clients_i->second;
        closeFuseClient(fuse_client);
    }
}

void DomElement::closeFuseClient(FuseClient *client)
{
    FUSE_CLIENTS::iterator fuse_clients_i;
    int fd;

    if (!client){
        goto out;
    }

    fd = client->getFd();

    fuse_clients_i = fuse_clients.find(fd);
    if (fuse_clients_i != fuse_clients.end()){
        if (client->getType() == INODE_SUBTYPE_LOG){
            // close log inode
        }
        delete client;
        client = NULL;
        fuse_clients.erase(fuse_clients_i);
    }

out:
    return;
}

int DomElement::isFuseClientOpen(FuseClient *a_client)
{
    int fd, ret = 0;
    FUSE_CLIENTS::iterator fuse_clients_i;

    if (!a_client){
        goto out;
    }

    fd = a_client->getFd();
    fuse_clients_i = fuse_clients.find(fd);
    if (fuse_clients_i != fuse_clients.end()){
        ret = 1;
    }

out:
    return ret;
}

FuseClient * DomElement::openFuseClient()
{
    int fd = 1;
    FUSE_CLIENTS::iterator fuse_clients_i;
    FuseClient  *fuse_client = NULL;
    LOG_ITEMS   log_items;

    fuse_client = new FuseClient();

    // search free virtual descriptor
    do {
        fuse_clients_i = fuse_clients.find(fd);
        if (fuse_clients_i == fuse_clients.end()){
            // we have found free fd
            break;
        }
        fd++;
    } while (1);

    // init fd
    fuse_client->setFd(fd);
    fuse_clients[fd] = fuse_client;
    return fuse_client;
}

void DomElement::pLog(LogItem &log_item)
{
    DomElement *parent = NULL;
    FUSE_CLIENTS::iterator fuse_clients_i;
    FuseClient *fuse_client = NULL;

    for (fuse_clients_i = fuse_clients.begin();
        fuse_clients_i != fuse_clients.end();
        fuse_clients_i++)
    {
        fuse_client = fuse_clients_i->second;
        if (fuse_client->getType() == INODE_SUBTYPE_LOG){
            // it is log inode
            fuse_client->log_items.push_back(log_item);
            // process log
            processLog(fuse_clients_i->first);
        }
    }

    // inform parent's elements
    parent = getParentNode();
    if (parent){
        parent->pLog(log_item);
    }
}

void DomElement::pDebug(const string &a_msg)
{
    LogItem log_item;
    log_item.msg = a_msg;
    pLog(log_item);
}

int DomElement::fuse_reply_buf_limited(
    fuse_req_t  a_req,
    const char  *a_buf,
    size_t      a_bufsize,
    off_t       a_off,
    size_t      a_maxsize)
{
    int ret = -1;
    if (a_off < a_bufsize){
        ret = fuse_reply_buf(
            a_req,
            a_buf + a_off,
            min(a_bufsize - a_off, a_maxsize)
        );
    } else {
        ret = fuse_reply_buf(a_req, NULL, 0);
    }
    return ret;
}

string DomElement::readLog(const int &a_fd)
{
    string ret = "";

    FUSE_CLIENTS::iterator  fuse_clients_i;
    LOG_ITEMS::iterator     log_items_i;
    PENDING_REQS::iterator  pending_reqs_i;

    FuseClient *fuse_client = NULL;

    fuse_clients_i = fuse_clients.find(a_fd);
    if (fuse_clients_i != fuse_clients.end()){
        fuse_client = fuse_clients_i->second;
        if (fuse_client->getType() != INODE_SUBTYPE_LOG){
            // it is not log node
            goto out;
        }
        LOG_ITEMS &log_items = fuse_client->log_items;

        if (log_items.size()){
            log_items_i         = log_items.begin();
            LogItem &log_item   = *log_items_i;
            ret += log_item.msg;
            log_items.erase(log_items_i);
        }
    }

out:
    return ret;
}

void DomElement::processLog(
    const int   &a_fd,
    fuse_req_t  a_req)
{
    string ret = "";

    FUSE_CLIENTS::iterator  fuse_clients_i;
    LOG_ITEMS::iterator     log_items_i;
    PENDING_REQS::iterator  pending_reqs_i;
    FuseClient *fuse_client = NULL;

    fuse_clients_i = fuse_clients.find(a_fd);
    if (fuse_clients_i != fuse_clients.end()){
        // ok, we have this client
        fuse_client = fuse_clients_i->second;
        if (fuse_client->getType() != INODE_SUBTYPE_LOG){
            // it is not log node
            goto out;
        }

        LOG_ITEMS &log_items = fuse_client->log_items;

        // store request as "pending" we will send
        // as soon as new log data will be available
        if (a_req){
            fuse_client->pending_reqs.push_back(a_req);
        }
        for (log_items_i = log_items.begin();
            log_items_i != log_items.end();
            log_items_i++)
        {
            LogItem &log_item = *log_items_i;
            ret += log_item.msg;
        }

        if (ret.size()){
            // search all pending requests and send data to they
            for (pending_reqs_i = fuse_client->pending_reqs.begin();
                pending_reqs_i != fuse_client->pending_reqs.end();
                pending_reqs_i++)
            {
                fuse_req_t req = *pending_reqs_i;
                fuse_reply_buf(req, ret.c_str(), ret.size());
            }
            if (fuse_client->pending_reqs.size()){
                // I hope we have send log messages successful,
                // not clear are and clear oending requests
                log_items.clear();
                fuse_client->pending_reqs.clear();
            }
        }
    }
out:
    return;
}


