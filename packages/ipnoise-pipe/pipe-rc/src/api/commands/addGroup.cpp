#include "api.hpp"
#include "addGroup.hpp"

ApiCommandAddGroup::ApiCommandAddGroup(Api *a_api)
    :   ApiCommand(a_api, "addGroup")
{
}

ApiCommandAddGroup::~ApiCommandAddGroup()
{
}

void ApiCommandAddGroup::process(
    const ApiCommandArgs &a_args)
{
    QString     name;
    QString     parent;
    QString     cmd;

    if (a_args.contains("name")){
        name = a_args["name"];
    }

    if (a_args.contains("parent")){
        parent = a_args["parent"];
    }

    cmd += cmdStart(a_args);
    cmd +=   "<commands>";
    cmd +=       "<command type=\"addGroup\" ver=\"0.01\">";
    cmd +=          "<group";
    cmd +=              " name=\""      + name      + "\"";
    cmd +=              " parent=\""    + parent    + "\"";
    cmd +=              "/>";
    cmd +=       "</command>";
    cmd +=   "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

