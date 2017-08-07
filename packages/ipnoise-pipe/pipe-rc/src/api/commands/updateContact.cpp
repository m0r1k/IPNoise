#include "api.hpp"
#include "updateContact.hpp"

ApiCommandUpdateContact::ApiCommandUpdateContact(Api *a_api)
    :   ApiCommand(a_api, "updateContact")
{
}

ApiCommandUpdateContact::~ApiCommandUpdateContact()
{
}

void ApiCommandUpdateContact::process(
    const ApiCommandArgs &a_args)
{
    ApiCommandArgsConstIt   it;
    QString                 id;
    QString                 cmd;

    if (a_args.contains("id")){
        id = a_args["id"];
    }

    cmd += cmdStart(a_args);
    cmd +=   "<commands>";
    cmd +=     "<command type=\"updateContact\" ver=\"0.01\">";
    cmd +=       "<contact";
    cmd +=         " id=\"" + id + "\"";
    cmd +=         ">";
    cmd +=           "<items>";
    for (it = a_args.begin(); it != a_args.end(); it++){
        QString key_full    = it.key();
        QString prefix      = "item_";
        if (prefix == key_full.midRef(0, prefix.size())){
            QString action = key_full.midRef(prefix.size())
                .toString();
            QString huid = it.value();
            cmd += "<item";
            cmd +=   " huid=\""   + huid      +"\"";
            cmd +=   " action=\"" + action    +"\"";
            cmd += "/>";
        }
    }
    cmd +=           "</items>";
    cmd +=       "</contact>";
    cmd +=     "</command>";
    cmd +=   "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

