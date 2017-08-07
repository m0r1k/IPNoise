#include "api.hpp"
#include "conferenceInvited.hpp"

ApiEventConferenceInvited::ApiEventConferenceInvited(
    Api *a_api)
    :   ApiEvent(a_api, "conferenceInvited")
{
}

ApiEventConferenceInvited::~ApiEventConferenceInvited()
{
}

void ApiEventConferenceInvited::process()
{
}

QString ApiEventConferenceInvited::getConfId()
{
    return m_element.attribute("conf_id");
}

QString ApiEventConferenceInvited::getHuid()
{
    return m_element.attribute("huid");
}

