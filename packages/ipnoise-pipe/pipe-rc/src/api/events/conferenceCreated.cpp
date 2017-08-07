#include "api.hpp"
#include "conferenceCreated.hpp"

ApiEventConferenceCreated::ApiEventConferenceCreated(
    Api *a_api)
    :   ApiEvent(a_api, "conferenceCreated")
{
}

ApiEventConferenceCreated::~ApiEventConferenceCreated()
{
}

void ApiEventConferenceCreated::process()
{
}

QString ApiEventConferenceCreated::getConfId()
{
    return m_element.attribute("conf_id");
}

QString ApiEventConferenceCreated::getConfName()
{
    return m_element.attribute("conf_topic");
}

