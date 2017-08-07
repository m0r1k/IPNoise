#include "api.hpp"
#include "searchUsers.hpp"

ApiCommandSearchUsers::ApiCommandSearchUsers(Api *a_api)
    :   ApiCommand(a_api, "searchUsers")
{
}

ApiCommandSearchUsers::~ApiCommandSearchUsers()
{
}

void ApiCommandSearchUsers::process(
    const ApiCommandArgs &a_args)
{
    QString linkName;
    QString linkAddr;
    QString cmd;

    if (a_args.contains("link_name")){
        linkName = a_args["link_name"];
    }
    if (a_args.contains("link_addr")){
        linkAddr = a_args["link_addr"];
    }

    cmd += cmdStart(a_args);
    cmd +=     "<commands>";
    cmd +=         "<command type=\"searchUsers\"";
    cmd +=             " ver=\"0.01\"";
    cmd +=             " link_name=\"" + linkName + "\"";
    cmd +=             " link_addr=\"" + linkAddr + "\"";
    cmd +=         "/>";
    cmd +=     "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

