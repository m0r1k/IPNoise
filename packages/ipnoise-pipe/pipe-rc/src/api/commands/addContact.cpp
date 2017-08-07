#include "api.hpp"
#include "addContact.hpp"

ApiCommandAddContact::ApiCommandAddContact(Api *a_api)
    :   ApiCommand(a_api, "addContact")
{
}

ApiCommandAddContact::~ApiCommandAddContact()
{
}

void ApiCommandAddContact::process(
    const ApiCommandArgs &a_args)
{
    ApiCommandArgsConstIt it;
    QString cmd;

    cmd += cmdStart(a_args);
    cmd +=   "<commands>";
    cmd +=       "<command type=\"addContact\"";
    cmd +=           " ver=\"0.01\"";
    cmd +=       ">";
    cmd +=         "<contact>";
    for (it = a_args.begin(); it != a_args.end(); it++){
        QString key_full    = it.key();
        QString prefix      = "info_";
        if (prefix == key_full.midRef(0, prefix.size())){
            QString key = key_full.midRef(prefix.size())
                .toString();
            QString value = it.value();
            cmd += "<"+key+">";
            cmd += "<![CDATA[" + value + "]]>";
            cmd += "</"+key+">";
        }
    }
    cmd +=           "<groups>";
    for (it = a_args.begin(); it != a_args.end(); it++){
        QString key_full    = it.key();
        QString prefix      = "group_";
        if (prefix == key_full.midRef(0, prefix.size())){
            QString id = key_full.midRef(prefix.size())
                .toString();
            QString action = it.value();
            if ("add" == action){
                cmd += "<group id=\""+id+"\"/>";
            }
        }
    }
    cmd +=           "</groups>";
    cmd +=         "</contact>";
    cmd +=       "</command>";
    cmd +=   "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

