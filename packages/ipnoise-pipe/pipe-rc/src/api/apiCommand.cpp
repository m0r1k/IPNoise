#include "api.hpp"
#include "apiCommand.hpp"

ApiCommand::ApiCommand(
    Api             *a_api,
    const QString   &a_name)
{
    m_api   = a_api;
    m_name  = a_name;
}

ApiCommand::~ApiCommand()
{
}

QString ApiCommand::getName()
{
    return m_name;
}

Api * ApiCommand::getApi()
{
    return m_api;
}

QString ApiCommand::cmdStart(
    const ApiCommandArgs &a_args,
    const QString        &a_attrs)
{
    QString src_sessid  = m_api->getSrcSessId();
    QString dst_sessid  = m_api->getDstSessId();
    QString seq         = m_api->getSessSeqStr();
    QString ack         = m_api->getSessAckStr();
    QString cmd;

    if (a_args.contains("src_sessid")){
        src_sessid = a_args["src_sessid"];
    }
    if (a_args.contains("dst_sessid")){
        dst_sessid = a_args["dst_sessid"];
    }
    if (a_args.contains("seq")){
        seq = a_args["seq"];
    }
    if (a_args.contains("ack")){
        ack = a_args["ack"];
    }

    cmd += "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
    cmd += "<ipnoise ver=\"0.01\"";
    cmd +=      " src_sessid=\"" + src_sessid + "\"";
    if (dst_sessid.size()){
        cmd +=  " dst_sessid=\"" + dst_sessid + "\"";
    }
    if (seq.size()){
        cmd +=  " seq=\"" + seq + "\"";
    }
    if (ack.size()){
        cmd +=  " ack=\"" + ack + "\"";
    }
    if (a_attrs.size()){
        cmd +=  " " + a_attrs;
    }
    cmd += ">";

    return cmd;
}

QString ApiCommand::cmdEnd(
    const ApiCommandArgs &)
{
    return "</ipnoise>";
}

