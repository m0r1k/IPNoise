#include "api.hpp"
#include "conferenceJoin.hpp"

ApiCommandConferenceJoin::ApiCommandConferenceJoin(Api *a_api)
    :   ApiCommand(a_api, "conferenceJoin")
{
}

ApiCommandConferenceJoin::~ApiCommandConferenceJoin()
{
}

void ApiCommandConferenceJoin::process(
    const ApiCommandArgs &a_args)
{
    QString     conf_id;
    QString     conf_topic;
    QString     flags;
    QString     conf_flags;
    QString     cmd;

    if (a_args.contains("conf_id")){
        conf_id = a_args["conf_id"];
    }

    if (a_args.contains("flags")){
        flags = a_args["flags"];
    }

    if (a_args.contains("conf_flags")){
        conf_flags = a_args["conf_flags"];
    }

    cmd += cmdStart(a_args);
    cmd +=     "<commands>";
    cmd +=         "<command type=\"conferenceJoin\"";
    cmd +=             " ver=\"0.01\"";
    cmd +=             " conf_id=\""    + conf_id    + "\"";
    cmd +=             " flags=\""      + flags      + "\"";
    cmd +=             " conf_flags=\"" + conf_flags + "\"";
    cmd +=         "/>";
    cmd +=     "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

