#include "api.hpp"
#include "searchUsersAnswer.hpp"

ApiEventSearchUsersAnswer::ApiEventSearchUsersAnswer(
    Api *a_api)
    :   ApiEvent(a_api, "searchUsersAnswer")
{
}

ApiEventSearchUsersAnswer::~ApiEventSearchUsersAnswer()
{
}

void ApiEventSearchUsersAnswer::process()
{
}

