#include "api.hpp"
#include "updateItem.hpp"

ApiCommandUpdateItem::ApiCommandUpdateItem(Api *a_api)
    :   ApiCommand(a_api, "updateItem")
{
}

ApiCommandUpdateItem::~ApiCommandUpdateItem()
{
}

void ApiCommandUpdateItem::process(
    const ApiCommandArgs &a_args)
{
    int32_t     i;
    char        buffer[512] = { 0x00 };
    QString     huid;
    QString     cmd;
    QString     links;

    if (a_args.contains("huid")){
        huid = a_args["huid"];
    }

    cmd += cmdStart(a_args);
    cmd +=  "<commands>";
    cmd +=      "<command type=\"updateItem\" ver=\"0.01\">";
    cmd +=          "<item";
    cmd +=              " huid=\"" + huid + "\"";
    cmd +=          ">";

    // add links
    i = 0;
    do {
        QString link_name;
        QString link_addr;

        // link name
        snprintf(buffer, sizeof(buffer),
            "link%d_name", i
        );
        if (not a_args.contains(buffer)){
            break;
        }
        link_name = a_args[buffer];

        // link addr
        snprintf(buffer, sizeof(buffer),
            "link%d_addr", i
        );
        if (not a_args.contains(buffer)){
            break;
        }
        link_addr = a_args[buffer];

        // store
        links += "<link";
        links +=    " name=\""    + link_name + "\"";
        links +=    " address=\"" + link_addr + "\"";
        links += "/>";

        // search next link
        i++;
    } while (0);

    if (links.size()){
        cmd += "<links>";
        cmd +=   links;
        cmd += "</links>";
    }

    cmd +=          "</item>";
    cmd +=      "</command>";
    cmd +=  "</commands>";
    cmd += cmdEnd(a_args);

    getApi()->queue(cmd);
}

