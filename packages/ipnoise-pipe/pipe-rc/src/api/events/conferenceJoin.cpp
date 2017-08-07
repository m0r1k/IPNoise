#include "api.hpp"
#include "conferenceJoin.hpp"

ApiEventConferenceJoin::ApiEventConferenceJoin(
    Api *a_api)
    :   ApiEvent(a_api, "conferenceJoin")
{
}

ApiEventConferenceJoin::~ApiEventConferenceJoin()
{
}

void ApiEventConferenceJoin::process()
{
}

QString ApiEventConferenceJoin::getConfId()
{
    return m_element.attribute("conf_id");
}

QString ApiEventConferenceJoin::getHuid()
{
    return m_element.attribute("huid");
}

