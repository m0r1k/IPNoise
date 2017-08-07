#include "api.hpp"
#include "getContactList.hpp"

ApiCommandGetContactList::ApiCommandGetContactList(Api *a_api)
    :   ApiCommand(a_api, "getContactList")
{
}

ApiCommandGetContactList::~ApiCommandGetContactList()
{
}

void ApiCommandGetContactList::process(
    const ApiCommandArgs &a_args)
{
    QString cmd;

    cmd += cmdStart(a_args);
    cmd +=     "<commands>";
    cmd +=         "<command type=\"getContactList\"";
    cmd +=             " ver=\"0.01\"";
    cmd +=         "/>";
    cmd +=     "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

