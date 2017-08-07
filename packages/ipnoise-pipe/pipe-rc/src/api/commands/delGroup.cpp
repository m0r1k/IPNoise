#include "api.hpp"
#include "delGroup.hpp"

ApiCommandDelGroup::ApiCommandDelGroup(Api *a_api)
    :   ApiCommand(a_api, "delGroup")
{
}

ApiCommandDelGroup::~ApiCommandDelGroup()
{
}

void ApiCommandDelGroup::process(
    const ApiCommandArgs &a_args)
{
    QString     id;
    QString     strategy = "0";
    QString     cmd;

    if (a_args.contains("id")){
        id = a_args["id"];
    }

    if (a_args.contains("strategy")){
        strategy = a_args["strategy"];
    }

    cmd += cmdStart(a_args);
    cmd +=   "<commands>";
    cmd +=       "<command type=\"delGroup\" ver=\"0.01\">";
    cmd +=          "<group";
    cmd +=              " id=\""        + id        + "\"";
    cmd +=              " strategy=\""  + strategy  + "\"";
    cmd +=          "/>";
    cmd +=       "</command>";
    cmd +=   "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

