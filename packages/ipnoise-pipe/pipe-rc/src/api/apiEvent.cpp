#include "apiEvent.hpp"

ApiEvent::ApiEvent(
    Api             *a_api,
    const QString   &a_name)
{
    m_api   = a_api;
    m_name  = a_name;
}

ApiEvent::~ApiEvent()
{
}

QString ApiEvent::getName()
{
    return m_name;
}

Api * ApiEvent::getApi()
{
    return m_api;
}

void ApiEvent::process(QDomElement &a_event)
{
    m_element = a_event;
    process();
}

QDomElement ApiEvent::getElement()
{
    return m_element;
}

