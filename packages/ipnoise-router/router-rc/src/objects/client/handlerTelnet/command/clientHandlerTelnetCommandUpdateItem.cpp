#include <getopt.h>

#include <ipnoise-common/log.h>
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

// for GROUP_TYPE_ROOT
#include "objects/groupsObject.h"

#include "clientHandlerTelnetCommandUpdateItem.h"

ClientHandlerTelnetCommandUpdateItem::ClientHandlerTelnetCommandUpdateItem(
    ClientHandlerTelnetObject *handler)
    :   ClientHandlerTelnetCommand(handler)
{
    _handler = handler;
};

ClientHandlerTelnetCommandUpdateItem::~ClientHandlerTelnetCommandUpdateItem()
{
};

const char *ClientHandlerTelnetCommandUpdateItem::getName()
{
    return "api_updateItem";
};

int ClientHandlerTelnetCommandUpdateItem::run(int argc, char* const* argv)
{
    PDEBUG(5, "updateitem command received\n");
    char buffer[1024]   = { 0x00 };
    int show_xml        = 0;
    int show_usage      = 0;
    int show_prototype  = 0;
    int send_command    = 0;
    int res, err        = 0;
    string command      = "";

    PLACEHOLDERS::iterator placeholders_i;
    PLACEHOLDERS           placeholders;

    // init placeholders
    placeholders["sessid"]              = _handler->apiGetSessionId();
    placeholders["huid"] = "2210:0:7309:227d:3030:fb23:816a:cc5d";
    placeholders["nickname"]            = "some updated nick name";
    placeholders["icon"]                = "/public/muramba_updated.jpg";
    snprintf(buffer, sizeof(buffer), "%d",
        GroupsObject::GROUP_TYPE_ROOT);
    placeholders["group_del_id"]        = buffer;
    snprintf(buffer, sizeof(buffer), "%d",
        GroupsObject::GROUP_TYPE_USER_DEFINED);
    placeholders["group_add_id"]        = buffer;
    placeholders["link_name"]      = "";
    placeholders["link_addr"]      = "";

    // init command prototype
    res = do_init(argv[0],
        "<ipnoise ver=\"0.01\" sessid=\"[[sessid]]\">\n"
        "    <commands>\n"
        "       <command type=\"updateItem\" ver=\"0.01\">\n"
        "           <item huid=\"[[huid]]\">\n"
        "               <nickname>[[nickname]]</nickname>\n"
        "               <icon>[[icon]]</icon>\n"
        "           </item>\n"
        "           <groups_add>\n"
        "               <group id=\"[[group_add_id]]\"/>\n"
        "           </groups_add>\n"
        "           <groups_del>\n"
        "               <group id=\"[[group_del_id]]\"/>\n"
        "           </groups_del>\n"
        "           <links>\n"
        "               <link name=\"[[link_name]]\""
                            " address=\"[[link_addr]]\"/>\n"
        "           </links>\n"
        "       </command>\n"
        "    </commands>\n"
        "</ipnoise>\n"
    );
    if (res){
        err = res;
        goto fail;
    }

    // get args
    while(1) {
        int option_index = 0;
        static struct option long_options[] = {
            {"help",                0, 0, 'h'},
            {"prototype",           0, 0, 'p'},
            {"xml",                 0, 0, 'x'},
            {"sessid",              1, 0, '0'},
            {"huid",                1, 0, '1'},
            {"nickname",            1, 0, '2'},
            {"icon",                1, 0, '3'},
            {"group_add_id",        1, 0, '4'},
            {"group_del_id",        1, 0, '5'},
            {"link_addr",      1, 0, '6'},
            {"link_name",      1, 0, '7'},
            {"send",                0, 0, 's'},
            {0, 0, 0, 0}
        };

        int option = getopt_long (argc, argv,
            "hxp0:1:2:3:4:5:6:7:s",
            long_options, &option_index
        );

        if (option == -1){
            break;
        }

        switch (option) {
            case 'h':
                show_usage = 1;
                break;

            case 'x':
                show_xml = 1;
                break;

            case 'p':
                show_prototype = 1;
                break;

            case '0':
                placeholders["sessid"] = optarg;
                break;

            case '1':
                placeholders["huid"] = optarg;
                break;

            case '2':
                placeholders["nickname"] = optarg;
                break;

            case '3':
                placeholders["icon"] = optarg;
                break;

            case '4':
                placeholders["group_add_id"] = optarg;
                break;

            case '5':
                placeholders["group_del_id"] = optarg;
                break;

            case '6':
                placeholders["link_addr"] = optarg;
                break;

            case '7':
                placeholders["link_name"] = optarg;
                break;

            case 's':
                send_command = 1;
                break;
        }
    }

    // check another args
    if (optind < argc){
        snprintf(buffer, sizeof(buffer),
            "Warning: argument(s) will be ignored: ");
        _handler->write(buffer);
        while (optind < argc){
            snprintf(buffer, sizeof(buffer), "'%s' ", argv[optind++]);
            _handler->write(buffer);
        }
        snprintf(buffer, sizeof(buffer), "\n");
        _handler->write(buffer);
    }

    if (show_usage){
        snprintf(buffer, sizeof(buffer),
            "Usage:     %s [options]                                        \n"
            "options:                                                       \n"
            "         -h, --help            This help                       \n"
            "         -p, --xml             Just show prototype             \n"
            "         -x, --xml             Just show xml, don't send       \n"
            "             --sessid          API session ID                  \n"
            "             --huid            item's huid (item for add)      \n"
            "             --nickname        item's nick name (item for add) \n"
            "             --icon            item's icon (item for add)      \n"
            "             --group_add_id    group id for add                \n"
            "             --group_del_id    group id for del                \n"
            "             --link_addr       link address                    \n"
            "             --link_name       link name                       \n"
            "         -s, --send            Send command                    \n"
            "\n",
            argv[0]
        );
        _handler->write(buffer);
        goto out;
    }

    if (show_prototype){
        command = getCommand();
        _handler->write("Prototype:\n");
        _handler->write(command);
    }

    // replace placeholders
    command = getCommand(placeholders);

    if (show_xml){
        _handler->write(command);
    }

    if (send_command){
        _handler->do_http_command(command);
    }

out:
    return err;

fail:
    if (!err){
        err = -1;
    }
    goto out;

};

