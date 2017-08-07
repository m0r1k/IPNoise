#include "api.hpp"
#include "loginFailed.hpp"

ApiEventLoginFailed::ApiEventLoginFailed(
    Api *a_api)
    :   ApiEvent(a_api, "loginFailed")
{
}

ApiEventLoginFailed::~ApiEventLoginFailed()
{
}

void ApiEventLoginFailed::process()
{
}

