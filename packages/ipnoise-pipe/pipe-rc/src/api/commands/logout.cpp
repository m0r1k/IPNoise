#include "api.hpp"
#include "logout.hpp"

ApiCommandLogout::ApiCommandLogout(Api *a_api)
    :   ApiCommand(a_api, "logout")
{
}

ApiCommandLogout::~ApiCommandLogout()
{
}

void ApiCommandLogout::process(
    const ApiCommandArgs &a_args)
{
    QString logout;
    QString password;
    QString cmd;

    cmd += cmdStart(a_args);
    cmd +=     "<commands>";
    cmd +=         "<command type=\"logout\"";
    cmd +=             " ver=\"0.01\"";
    cmd +=         "/>";
    cmd +=     "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);

out:
    return;
}

