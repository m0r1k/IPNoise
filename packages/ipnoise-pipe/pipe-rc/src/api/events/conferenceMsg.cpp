#include "api.hpp"
#include "conferenceMsg.hpp"

ApiEventConferenceMsg::ApiEventConferenceMsg(
    Api *a_api)
    :   ApiEvent(a_api, "conferenceMsg")
{
}

ApiEventConferenceMsg::~ApiEventConferenceMsg()
{
}

void ApiEventConferenceMsg::process()
{
}

QString ApiEventConferenceMsg::getConfId()
{
    return m_element.attribute("conf_id");
}

QString ApiEventConferenceMsg::getMsgId()
{
    QString         ret;
    QDomElement     message_el;

    message_el = m_api->getElementByTagName(
        m_element,
        "message"
    );
    if (message_el.isNull()){
        goto out;
    }
    ret = message_el.attribute("id");

out:
    return ret;
}

QString ApiEventConferenceMsg::getMsgValue()
{
    QString         ret;
    QDomElement     message_el;

    message_el = m_api->getElementByTagName(
        m_element,
        "message"
    );
    if (message_el.isNull()){
        goto out;
    }
    ret = message_el.text();

out:
    return ret;
}

QString ApiEventConferenceMsg::getMsgType()
{
    QString         ret;
    QDomElement     message_el;

    message_el = m_api->getElementByTagName(
        m_element,
        "message"
    );
    if (message_el.isNull()){
        goto out;
    }
    ret = message_el.attribute("type");

out:
    return ret;
}


