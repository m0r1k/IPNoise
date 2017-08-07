#include "api.hpp"
#include "conferenceMsg.hpp"

ApiCommandConferenceMsg::ApiCommandConferenceMsg(Api *a_api)
    :   ApiCommand(a_api, "conferenceMsg")
{
}

ApiCommandConferenceMsg::~ApiCommandConferenceMsg()
{
}

void ApiCommandConferenceMsg::process(
    const ApiCommandArgs &a_args)
{
    QString     conf_id;
    QString     msg;
    QString     cmd;

    if (a_args.contains("conf_id")){
        conf_id = a_args["conf_id"];
    }

    if (a_args.contains("msg")){
        msg = a_args["msg"];
    }

    if (!conf_id.size()){
        PERROR("Empty conf_id\n");
        goto fail;
    }

    if (!msg.size()){
        PERROR("Attempt to send empty message\n");
        goto fail;
    }

    cmd += cmdStart(a_args);
    cmd +=     "<commands>";
    cmd +=        "<command type=\"conferenceMsg\"";
    cmd +=             " ver=\"0.01\"";
    cmd +=             " conf_id=\"" + conf_id + "\"";
    cmd +=         ">";
    cmd +=             "<message><![CDATA[";
    cmd +=                 msg;
    cmd +=             "]]></message>";
    cmd +=         "</command>";
    cmd +=     "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);

out:
    return;
fail:
    goto out;
}

