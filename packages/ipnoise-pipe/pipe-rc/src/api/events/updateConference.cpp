#include "api.hpp"
#include "updateConference.hpp"

ApiEventUpdateConference::ApiEventUpdateConference(
    Api *a_api)
    :   ApiEvent(a_api, "updateConference")
{
}

ApiEventUpdateConference::~ApiEventUpdateConference()
{
}

void ApiEventUpdateConference::process()
{
}

QString ApiEventUpdateConference::getConfId()
{
    return m_element.attribute("conf_id");
}

QString ApiEventUpdateConference::getConfTopic()
{
    return m_element.attribute("conf_topic");
}

QString ApiEventUpdateConference::getConfFlags()
{
    return m_element.attribute("conf_flags");
}

