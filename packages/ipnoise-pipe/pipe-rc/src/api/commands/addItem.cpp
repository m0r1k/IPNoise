#include "api.hpp"
#include "addItem.hpp"

ApiCommandAddItem::ApiCommandAddItem(Api *a_api)
    :   ApiCommand(a_api, "addItem")
{
}

ApiCommandAddItem::~ApiCommandAddItem()
{
}

void ApiCommandAddItem::process(
    const ApiCommandArgs &a_args)
{
    ApiCommandArgsConstIt it;
    QString cmd;

    cmd += cmdStart(a_args);
    cmd +=   "<commands>";
    cmd +=       "<command type=\"addItem\"";
    cmd +=           " ver=\"0.01\"";
    cmd +=       ">";
    cmd +=         "<item huid=\"\">";
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
    cmd +=           "<links>";
    for (it = a_args.begin(); it != a_args.end(); it++){
        QString key_full    = it.key();
        QString prefix      = "link_";
        if (prefix == key_full.midRef(0, prefix.size())){
            QString name = key_full.midRef(prefix.size())
                .toString();
            QString addr = it.value();
            cmd += "<link";
            cmd += " name=\""    + name +"\"";
            cmd += " address=\"" + addr +"\"";
            cmd += "/>";
        }
    }
    cmd +=           "</links>";
    cmd +=         "</item>";
    cmd +=       "</command>";
    cmd +=   "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

