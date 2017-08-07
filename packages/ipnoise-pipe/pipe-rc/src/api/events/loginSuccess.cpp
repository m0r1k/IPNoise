#include "loginSuccess.hpp"

ApiEventLoginSuccess::ApiEventLoginSuccess(
    Api *a_api)
    :   ApiEvent(a_api, "loginSuccess")
{
}

ApiEventLoginSuccess::~ApiEventLoginSuccess()
{
}

void ApiEventLoginSuccess::process()
{
}

