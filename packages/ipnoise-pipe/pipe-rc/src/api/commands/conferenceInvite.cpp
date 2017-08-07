#include "api.hpp"
#include "conferenceInvite.hpp"

ApiCommandConferenceInvite::ApiCommandConferenceInvite(Api *a_api)
    :   ApiCommand(a_api, "conferenceInvite")
{
}

ApiCommandConferenceInvite::~ApiCommandConferenceInvite()
{
}

void ApiCommandConferenceInvite::process(
    const ApiCommandArgs &a_args)
{
    QString     conf_id;
    QString     huid;
    QString     cmd;

    if (a_args.contains("conf_id")){
        conf_id = a_args["conf_id"];
    }

    if (a_args.contains("huid")){
        huid = a_args["huid"];
    }

    cmd += cmdStart(a_args);
    cmd +=     "<commands>";
    cmd +=         "<command type=\"conferenceInvite\"";
    cmd +=             " ver=\"0.01\"";
    cmd +=             " conf_id=\""    + conf_id    + "\"";
    cmd +=             " huid=\""       + huid       + "\"";
    cmd +=         "/>";
    cmd +=     "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

