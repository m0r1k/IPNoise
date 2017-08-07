#include "api.hpp"
#include "conferenceMsgDelivered.hpp"

ApiEventConferenceMsgDelivered::ApiEventConferenceMsgDelivered(
    Api *a_api)
    :   ApiEvent(a_api, "conferenceMsgDelivered")
{
}

ApiEventConferenceMsgDelivered::~ApiEventConferenceMsgDelivered()
{
}

void ApiEventConferenceMsgDelivered::process()
{
}

QString ApiEventConferenceMsgDelivered::getConfId()
{
    return m_element.attribute("conf_id");
}

QString ApiEventConferenceMsgDelivered::getMsgId()
{
    return m_element.attribute("msg_id");
}

