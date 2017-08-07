#ifndef REGISTER_ELEMENT_HPP
#define REGISTER_ELEMENT_HPP

// generic for all of elements types

template<typename T = Object>
T * _REGISTER_ELEMENT(
    T       *a_el,
    Tags    &a_tags)
{
    T       *el     = a_el;
    string  tagname = a_el->getTagName();

    // check what not registered
    if (a_tags.end() != a_tags.find(tagname)){
        PERROR("tag: '%s' already registered\n",
            tagname.c_str()
        );
        DomDocument::deleteElementSafe(el);
        el = NULL;
        goto out;
    }

    // store element
    a_tags[tagname] = el;

out:
    return el;
}

template<typename T = Object>
T * REGISTER_ELEMENT(Tags &a_tags)
{
    T *el = NULL;

    // create element
    el = new T(NULL, this);

    return _REGISTER_ELEMENT(el, a_tags);
}

template<typename T = Object>
T * REGISTER_ELEMENT(
    Tags            &a_tags,
    const string    &a_tagname)
{
    T *el = NULL;

    // create element
    el = new T(NULL, this, a_tagname);

    return _REGISTER_ELEMENT(el, a_tags);
}

// type - events

template <typename T = Object>
T * REGISTER_EVENTS_ELEMENT()
{
    return REGISTER_ELEMENT<T>(m_tags_events);
}

template <typename T = Object>
T * REGISTER_EVENTS_ELEMENT(const string &a_tagname)
{
    return REGISTER_ELEMENT<T>(
        m_tags_events,
        a_tagname
    );
}

// type - net

template <typename T = Object>
T * REGISTER_NET_ELEMENT()
{
    return REGISTER_ELEMENT<T>(m_tags_net);
}

template <typename T = Object>
T * REGISTER_NET_ELEMENT(const string &a_tagname)
{
    return REGISTER_ELEMENT<T>(
        m_tags_net,
        a_tagname
    );
}

// type - sys

template <typename T = Object>
T * REGISTER_SYS_ELEMENT()
{
    return REGISTER_ELEMENT<T>(m_tags_sys);
}

template <typename T = Object>
T * REGISTER_SYS_ELEMENT(const string &a_tagname)
{
    return REGISTER_ELEMENT<T>(
        m_tags_sys,
        a_tagname
    );
}

#endif

