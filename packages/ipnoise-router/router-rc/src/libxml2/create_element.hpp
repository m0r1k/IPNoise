#ifndef CREATE_ELEMENT_HPP
#define CREATE_ELEMENT_HPP

/*
template <typename T = Object>
T * _checkAccess(T *a_el)
{
    T       *el        = a_el;
    string  tagname    = el->getTagName();
    int32_t found      = 0;

    // check 'events' privs
    if (    m_priveleges & USE_EVENTS
        &&  m_tags_events.end()
            != m_tags_events.find(tagname))
    {
        found = 1;
    }

    // check 'net' privs
    if (    m_priveleges & USE_NET_OBJECTS
        &&  m_tags_net.end()
                == m_tags_net.find(tagname))
    {
        found = 1;
    }

    // check 'sys' privs
    if (    m_priveleges & USE_SYS_OBJECTS
        &&  m_tags_sys.end()
                == m_tags_sys.find(tagname))
    {
        found = 1;
    }

    if (not found){
        PERROR("unsupported tag: '%s'\n",
            tagname.c_str()
        );
        goto fail;
    }

    // all ok

out:
    return el;
fail:
    if (el){
       DomDocument::deleteElementSafe(el);
    }
    el = NULL;
    goto out;
}

template <typename T = Object>
T * createElement(
    xmlNodePtr      a_node,
    const string    &a_tagname)
{
    T *el = NULL;

    // create element
    el = new T(a_node, this, a_tagname);

    el = _checkAccess(el);
    return el;
}

template <typename T = Object>
T * createElement(const string &a_tagname){
    return createElement<T>(NULL, a_tagname);
}

template <typename T = Object>
T * createElement(xmlNodePtr a_node)
{
    T *el = NULL;

    // create element
    el = new T(a_node, this);

    el = _checkAccess(el);
    return el;
}

template <typename T = Object>
T * createElement(){
    return createElement<T>(NULL);
}

*/

// ---- for dom init ----

template <typename T = DomElement>
T * createElement(xmlNodePtr a_node)
{
    T       *el = NULL;
    TagsIt  tags_it;
    string  tagname;

    // get tag name
    {
        T *tmp = new T(NULL, this);
        tagname = tmp->getTagName();
        DomDocument::deleteElementSafe(tmp);
    }

    if (not tagname.size()){
        PERROR("Cannot create element\n");
        goto fail;
    }

    // check 'events' privs
    tags_it = m_tags_events.find(tagname);
    if (    m_priveleges & USE_EVENTS
        &&  m_tags_events.end() != tags_it)
    {
        el = (T *)tags_it->second->create_object(
            a_node, this, tagname
        );
        goto out;
    }

    // check 'net' privs
    tags_it = m_tags_net.find(tagname);
    if (    m_priveleges & USE_NET_OBJECTS
        &&  m_tags_net.end() != tags_it)
    {
        el = (T *)tags_it->second->create_object(
            a_node, this, tagname
        );
        goto out;
    }

    // check 'sys' privs
    tags_it = m_tags_sys.find(tagname);
    if (    m_priveleges & USE_SYS_OBJECTS
        &&  m_tags_sys.end() != tags_it)
    {
        el = (T *)tags_it->second->create_object(
            a_node, this, tagname
        );
        goto out;
    }

    if (not el){
        PERROR("unsupported tag: '%s'\n",
            tagname.c_str()
        );
        goto fail;
    }

    // all ok

out:
    return el;
fail:
    goto out;
}

template <typename T = DomElement>
T * createElement(
    xmlNodePtr      a_node,
    const string    &a_tagname)
{
    T       *el = NULL;
    TagsIt  tags_it;

    // get tag name
    if (not a_tagname.size()){
        PERROR("Cannot create element\n");
        goto fail;
    }

    // check 'events' privs
    tags_it = m_tags_events.find(a_tagname);
    if (    m_priveleges & USE_EVENTS
        &&  m_tags_events.end() != tags_it)
    {
        el = (T *)tags_it->second->create_object(
            a_node, this, a_tagname
        );
        goto out;
    }

    // check 'net' privs
    tags_it = m_tags_net.find(a_tagname);
    if (    m_priveleges & USE_NET_OBJECTS
        &&  m_tags_net.end() != tags_it)
    {
        el = (T *)tags_it->second->create_object(
            a_node, this, a_tagname
        );
        goto out;
    }

    // check 'sys' privs
    tags_it = m_tags_sys.find(a_tagname);
    if (    m_priveleges & USE_SYS_OBJECTS
        &&  m_tags_sys.end() != tags_it)
    {
        el = (T *)tags_it->second->create_object(
            a_node, this, a_tagname
        );
        goto out;
    }

    if (not el){
        PERROR("unsupported tag: '%s'\n",
            a_tagname.c_str()
        );
        goto fail;
    }

    // all ok

out:
    return el;
fail:
    goto out;
}

template <typename T = DomElement>
T * createElement()
{
    return createElement<T>(NULL);
}

template <typename T = DomElement>
T * createElement(const string &a_tagname)
{
    return createElement<T>(NULL, a_tagname);
}

#endif

