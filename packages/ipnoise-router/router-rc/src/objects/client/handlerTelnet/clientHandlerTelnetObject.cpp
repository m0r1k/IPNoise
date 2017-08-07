/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Nov 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"

#include "objects/serverObject.h"
#include "objects/netEventObject.h"
#include <ipnoise-common/strings.h>
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandAddGroup.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandAddItem.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandConnect.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandDelGroup.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandDelItem.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandGetGroup.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandGetItem.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandGetContactList.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandHelp.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandStatus.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandLogin.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandLogout.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandQuit.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandSearchItems.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandUpdateGroup.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandUpdateItem.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandConferenceJoin.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandConferenceInvite.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandConferenceMsg.h"

// Debug
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandDevelDump.h"
#include "objects/client/handlerTelnet/command/clientHandlerTelnetCommandDevelStop.h"

// Telnet RFC 854
#define IAC                     "\xff"
#define DO                      "\xfd"
#define SB                      "\xfa"  // start sub option
#define SE                      "\xf0"  // end of sub option
#define WILL                    "\xfb"
#define WILLNOT                 "\xfc"
#define TERMINAL                "\x18"
#define ERASELINE               "\xf8"

#define SEND_YOUR_TERM_TYPE     "\x01"
#define ECHO                    "\x01"
#define HERE_IS_MY_TERM_TYPE    "\x00"
#define REMOTE_FLOW_CONTROL     "\x21"
#define SUPPRESS_GO_AHEAD       "\x03"
#define REMOTE_FLOW_CONTROL     "\x21"

ClientHandlerTelnetObject * ClientHandlerTelnetObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ClientHandlerTelnetObject(a_node, a_doc, a_tagname);
}

ClientHandlerTelnetObject::ClientHandlerTelnetObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   ClientHandler(a_node, a_doc, a_tagname)
{
    // mark commands as unsent
    do_terminal_type            = -1;
    do_suppress_go_ahead        = -1;
    do_echo                     = -1;
    do_remote_flow_control      = -1;

    // mark remote options as unsupported
    will_terminal_type          = -1;
    will_suppress_go_ahead      = -1;
    will_echo                   = -1;
    will_remote_flow_control    = -1;

    mode        = MODE_COMMAND_ERROR;
    history     = new History(this);
};

ClientHandlerTelnetObject::~ClientHandlerTelnetObject()
{
    TELNET_COMMANDS::iterator commands_i;

    for (commands_i = commands.begin();
        commands_i != commands.end();
        commands_i++)
    {
        ClientHandlerTelnetCommand *command = *commands_i;
        delete command;
    }
    delete history;
};

int ClientHandlerTelnetObject::do_init()
{
    int err = -1;
    ClientHandlerTelnetCommand *command = NULL;

    // client api commands
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandAddGroup(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandAddItem(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandDelGroup(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandDelItem(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandGetGroup(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandGetItem(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandGetContactList(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandLogin(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandLogout(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandSearchItems(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandUpdateGroup(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandUpdateItem(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandConferenceJoin(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandConferenceInvite(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandConferenceMsg(this);
    commands.push_back(command);

    // system commands
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandHelp(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandConnect(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandStatus(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandQuit(this);
    commands.push_back(command);

    // devel commands
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandDevelDump(this);
    commands.push_back(command);
    command = (ClientHandlerTelnetCommand *)new ClientHandlerTelnetCommandDevelStop(this);
    commands.push_back(command);

    // all ok
    err = 0;

    return err;
}

int ClientHandlerTelnetObject::write(const string &buffer)
{
    int i;
    string msg = "";

    for (i = 0; i < (int)buffer.size(); i++){
        if (buffer.at(i) == '\n'){
            msg += "\r\n";
        } else {
            msg += buffer.at(i);
        }
    }
    return ClientHandler::write(msg);
};

string ClientHandlerTelnetObject::parse_telnet(const string &a_data)
{
    char out_buffer[1024]   = { 0x00 };
    const char *cmd         = NULL;

    string data = a_data;
    string out  = "";

    // process answer
    for (int i = 0; i < (int)data.size(); i++){
        do {
            if (data.at(i) != IAC[0]){
                out += data.at(i);
                break;
            }
            i++; if (i > (int)data.size()){ break; }
            if (    (data.at(i) == WILL[0])
                ||  (data.at(i) == WILLNOT[0]))
            {
                // remote part have inform us what
                // option will or willnot supported
                int state = 0;
                if (data.at(i) == WILL[0]){
                    state = 1;
                } else {
                    state = -1;
                }
                i++; if (i > (int)data.size()){ break; }
                if (data.at(i) == TERMINAL[0]){
                    will_terminal_type = state;
                } else if (data.at(i) == SUPPRESS_GO_AHEAD[0]){
                    will_suppress_go_ahead = state;
                } else if (data.at(i) == ECHO[0]){
                    will_echo = state;
                } else if (data.at(i) == REMOTE_FLOW_CONTROL[0]){
                    will_remote_flow_control = state;
                }
            } else if (data.at(i) == DO[0]){
                i++; if (i > (int)data.size()){ break; }
                if (data.at(i) == ECHO[0]){
                    cmd = IAC WILL ECHO;
                    write(cmd);
                } else if (data.at(i) == SUPPRESS_GO_AHEAD[0]){
                    cmd = IAC WILL SUPPRESS_GO_AHEAD;
                    write(cmd);
                } else {
                    // inform what we are will not support any option
                    snprintf(out_buffer, sizeof(out_buffer),
                        IAC WILLNOT "%c",
                        data.at(i)
                    );
                    write(out_buffer);
                }
            } else if (data.at(i) == SB[0]){
                // start of sub option
                do {
                    i++; if (i > (int)data.size()){ break; }
                    if (data.at(i) == IAC[0]){
                        i++; if (i > (int)data.size()){ break; }
                        if (data.at(i) == SE[0]){
                            // end of sub option
                            data = a_data.substr(0, i - 1);
                            break;
                        }
                    } else if (data.at(i) == TERMINAL[0]){
                        i++; if (i > (int)data.size()){ break; }
                        if (data.at(i) == HERE_IS_MY_TERM_TYPE[0]){
                            i++; if (i > (int)data.size()){ break; }
                            terminal_type = data.substr(i);
                        }
                    }
                } while (i < (int)data.size());
            }
        } while (0);
    }

    if (do_terminal_type == -1){
        // requst what terminal type command avalibale
        cmd = IAC DO TERMINAL;
        write(cmd);
        do_terminal_type = 1;
    }
    if (do_suppress_go_ahead == -1){
        cmd = IAC WILL SUPPRESS_GO_AHEAD;
        write(cmd);
        do_suppress_go_ahead = 1;
    }
    if (do_remote_flow_control == -1){
        cmd = IAC DO REMOTE_FLOW_CONTROL;
        write(cmd);
        do_remote_flow_control = 1;
    }

    if (do_echo == -1){
        cmd = IAC WILL ECHO;
        write(cmd);
        do_echo = 1;
    }

    if (will_remote_flow_control == 1){
        cmd = IAC SB REMOTE_FLOW_CONTROL IAC SE;
        write(cmd);
        will_remote_flow_control++;
    }

    if (will_terminal_type == 1
        && !terminal_type.size())
    {
        // request terminal type
        cmd = IAC SB TERMINAL SEND_YOUR_TERM_TYPE IAC SE;
        write(cmd);
    }

    return out;
}

int ClientHandlerTelnetObject::process_command(string &command)
{
    int argc = 0;
    int found = 0, i, err = 0;
    TELNET_COMMANDS::iterator commands_i;
    string msg;

    Splitter arguments(command.c_str(), " ");
    char **args = new char * [arguments.size() + 1];

    if (!command.size()){
        goto out;
    }

    {
        // store save value
        HistoryString cmd;
        cmd.setValue(command);
        cmd.setSaveValue(command);
        // store in history
        history->addItem(cmd);
    }

    // it happens not freq, so we can call 'save' directly
    history->save();

    argc = arguments.size();
    for (i = 0; i < argc; i++){
        args[i] = strdup(arguments[i].c_str());
    }
    args[i] = NULL;

    for (commands_i = commands.begin();
        commands_i != commands.end();
        commands_i++)
    {
        ClientHandlerTelnetCommand *c = *commands_i;
        if (!strcmp(c->getName(), args[0])){
            found = 1;
            c->run(argc, args);
            break;
        }
    }

    for (i = 0; i < argc; i++){
        free(args[i]);
    }

    delete [] args;

    if (!found){
        write("command");
        write(" '");
        write(command);
        write("'");
        write(" was not found\n");
    }

out:
    return err;
}

int ClientHandlerTelnetObject::process_mode_edit(
    const string    &a_data,
    string          &prefix)
{
    int i;
    int err = 0;
    int len = (int)a_data.size();

    TELNET_COMMANDS::iterator commands_i;

    if (len >= 1 && a_data.at(0) == '\x7f'){
        // back space
        string cmd_line = history->getCurItemValue();
        if (cmd_line.size() > 0){
            history->delCurItemColPos();
        }
    } else if (len >= 1 && a_data.at(0) == '\x1b'){
        if (len >= 2 && a_data.at(1) == '\x5b'){
            if (len >= 3 && a_data.at(2) == '\x41'){
                // up key
                history->up();
            } else if (len >= 3 && a_data.at(2) == '\x42'){
                // down key
                history->down();
            } else if (len >= 3 && a_data.at(2) == '\x43'){
                // right key
                history->upCurItemColPos();
            } else if (len >= 3 && a_data.at(2) == '\x44'){
                // left key
                history->downCurItemColPos();
            }
        }
    } else if (len >= 1
        && ((a_data.at(0) == '\r') || (a_data.at(0) == '\n')))
    {
        write("\n");
        string cmd_line = history->getCurItemValue();
        process_command(cmd_line);
        write(prefix);
    } else if (len >= 1 && a_data.at(0) == '\t'){
        // tab
        vector <string>             matched_commands;
        vector <string>::iterator   matched_commands_i;

        string cmd_line = history->getCurItemValue();

        for (commands_i = commands.begin();
            commands_i != commands.end();
            commands_i++)
        {
            ClientHandlerTelnetCommand *command = *commands_i;
            const char *cmd_name = command->getName();
            if (!strncmp(cmd_name, cmd_line.c_str(), cmd_line.size())){
                matched_commands.push_back(cmd_name);
            }
        }

        {
            // metach commands
            for (matched_commands_i = matched_commands.begin();
                matched_commands_i != matched_commands.end();
                matched_commands_i++)
            {
                // add matched commands to output
                write("\n");
                write(*matched_commands_i);
            }
        }

        {
            // autofill commands
            if (matched_commands.size() > 1){
                // and search generic prefix for all matched commands
                string &cmd             = *matched_commands.begin();
                string prefix           = "";
                string generic_prefix   = "";

                for (i = 0; i < (int)cmd.size(); i++){
                    int all_commands_have_this_prefix = 1;
                    prefix += cmd.at(i);
                    for (matched_commands_i = matched_commands.begin();
                        matched_commands_i != matched_commands.end();
                        matched_commands_i++)
                    {
                        string::size_type pos = string::npos;
                        pos = matched_commands_i->find(prefix, 0);
                        if (pos == string::npos){
                            // have found not generic prefix, break
                            all_commands_have_this_prefix = 0;
                            break;
                        }
                    }
                    if (!all_commands_have_this_prefix){
                        break;
                    }

                    // store generic prefix
                    generic_prefix = prefix;
                }

                if (generic_prefix.size()){
                    cmd_line = generic_prefix;
                    history->setCurItemValue(cmd_line);
                }
            } else if (matched_commands.size() == 1){
                // only one command was found, so it is prefix
                cmd_line = *matched_commands.begin();
                history->setCurItemValue(cmd_line);
            }
        }

        write("\n");
        write("\r");
        write(prefix);
        write(cmd_line);
    } else if (len >= 1){
        history->addCurItemColPos(a_data.at(0));
    }

    return err;
}

void ClientHandlerTelnetObject::closed_cb()
{
}

int ClientHandlerTelnetObject::read_cb(
    const string    &a_data,
    ReadState       rd_state)
{
    int err         = 0;
    int res         = 0;
    string exec_cmd = "";
    string prefix   = "\r> ";
    string data     = "";
    string cmd_line = history->getCurItemValue();

    // remove telnet commands
    data = parse_telnet(a_data);

    if (rd_state == READ_STATE_FIRST_READ){
        write("Setup client protocol: text/telnet.\n");
        write("Welcome to the IPNoise API telnet console!\n");
        write("Use help for show help page, use TAB for commands autocomplete\n");
        write("\n");
        write(prefix);
        mode = MODE_COMMAND_EDIT;
        goto out;
    }

    switch (mode){
        case (MODE_COMMAND_EDIT):
            res = process_mode_edit(data, prefix);
            if (res < 0){
                err = res;
                goto fail;
            }
            break;

        default:
            break;
    };

out:
    return err;

fail:
    goto out;
};

void ClientHandlerTelnetObject::http_api_wait_cb(
    NetClientHttpObject     *http_client,
    size_t,                 // current_offset
    size_t,                 // current_length
    void                    *ctx)
{
    assert(ctx);

    process_answer(http_client, ctx);

    // request to delete us
    http_client->setExpired("1");
}

void ClientHandlerTelnetObject::http_api_wait_closed_cb(
    NetClientHttpObject     *http_client,
    size_t                  current_offset,
    size_t                  current_length,
    void                    *ctx)
{
    assert(ctx);
    char buffer[1024] = { 0x00 };

    ClientHandlerTelnetObject *handler = (ClientHandlerTelnetObject *)ctx;

    snprintf(buffer, sizeof(buffer), "[http_api_wait_closed_cb]"
        " current_offset: '%u', current_length: '%u', content: '%s'\n",
        current_offset,
        current_length,
        http_client->getAnswerContent().c_str()
    );
    PDEBUG_OBJ(handler, 9, "%s", buffer);

    // do reopen
    if ("closed" != handler->getClient()->getState()){
        handler->open_http_client_wait();
    }
}

/*
 *  history_line_hide - Hide current history line from user's screen
 */
void ClientHandlerTelnetObject::history_line_hide()
{
    string cur_history_line = "";

    // get current history line
    cur_history_line = history->getCurItemValue();
    for (int i = 0; i < (int)cur_history_line.size(); i++){
        // clear typed line
        write("\x08");
    }

    // clear "> " symbols
    write("\x08\x08");
}

/*
 *  history_line_show - Show current history line on user's screen
 */
void ClientHandlerTelnetObject::history_line_show()
{
    string cur_history_line = "";

    // get current history line
    cur_history_line = history->getCurItemValue();

    // show "> " symbols
    write("> ");

    // show current history line
    write(cur_history_line);
}

void ClientHandlerTelnetObject::process_answer(
    NetClientHttpObject     *http_client,
    void                    *ctx)
{
    assert(ctx);
    char buffer[1024] = { 0x00 };
    DomElement                  *element        = NULL;
    DomElement                  *tmp_element    = NULL;
    ClientHandlerTelnetObject   *handler        = NULL;
    vector<DomElement *> items;
    vector<DomElement *>::iterator items_i;
    string content = "";

    // get handler
    handler = (ClientHandlerTelnetObject *)ctx;

    // get answer content and check what it is exist
    content = http_client->getAnswerContent();
    if (!content.size()){
        goto out;
    }

    // convert DOM from answer content
    element = handler->getDocument()->createElementFromContent(content);

    if (!element){
        PERROR_OBJ(handler, "Cannot create element from content: '%s'\n",
            http_client->getAnswerContent().c_str());
        handler->write("Client open failed, cannot parse answer\n");
        goto fail;
    }

    {
        // search "tmp" element
        tmp_element = handler->getDocument()->getRootElement()
            ->getElementByTagName("ipn_tmp");
        if (tmp_element){
            // append element to "tmp"
            tmp_element->appendChildSafe(element);
        }
    }

    {
        // check answer root element
        string root_tag = "";
        root_tag = element->getTagName();
        if (root_tag != "ipnoise"){
            PERROR_OBJ(handler, "invalid root element type: '%s'"
                " must be 'ipnoise', content: '%s'\n",
                root_tag.c_str(),
                element->serialize(2).c_str()
            );
            handler->write("Client open failed, cannot parse answer\n");
            goto fail;
        }
    }

    // search events
    items.clear();
    element->getElementsByXpath("//events/event", items);

    if (not items.empty()){
        handler->history_line_hide();
    }

    for (items_i = items.begin();
        items_i != items.end();
        items_i++)
    {
        string type    = "";
        string sessid  = "";

        NetEventObject *event = (NetEventObject *)*items_i;
        assert(event);

        type   = event->getAttribute("type");
        sessid = element->getAttribute("sessid");

        // write information about events
        handler->write("[ event ] ");

        if (type == "openSessionSuccess"){
            handler->apiSetSessionId(sessid);
            handler->write("Client session was open successful, sessid: '"
                + sessid + "'\n");
        } else if (type == "openSessionFailed"){
            handler->write("Cannot open client session\n");

        } else if (type == "loginSuccess"){
            handler->write("Login success\n");
            // open "client wait" (it will inform us about new async events)
            handler->open_http_client_wait();
        } else if (type == "loginFailed"){
            handler->write("Login failed\n");

        } else if (type == "logoutSuccess"){
            handler->write("Logout success\n");

        } else if (type == "logoutFailed"){
            handler->write("Logout failed\n");

        } else if (type == "addItemSuccess"){
            handler->write("Item was added successful\n");

        } else if (type == "addItemFailed"){
            handler->write("Cannot add item\n");

        } else if (type == "getItemSuccess"){
            snprintf(buffer, sizeof(buffer),
                "Item info was got successful:\n%s\n",
                element->serializeForPublic(1).c_str());
            handler->write(buffer);

        } else if (type == "getItemFailed"){
            handler->write("Cannot get item info\n");

        } else if (type == "delItemSuccess"){
            handler->write("Item was deleted successful\n");

        } else if (type == "delItemFailed"){
            handler->write("Cannot delete item\n");

        } else if (type == "conferenceJoin"){
            string conf_id  = event->getAttribute("conf_id");
            string huid     = event->getAttribute("huid");
            handler->write("User with huid: '"+huid+"'"
                + " successfull joined"
                + " to conference: '"+ conf_id + "'\n");

        } else if (type == "conferenceJoinFailed"){
            string conf_id = event->getAttribute("conf_id");
            handler->write("Join to conference: '" + conf_id + "' failed\n");

        } else if (type == "conferenceInvited"){
            string conf_id  = event->getAttribute("conf_id");
            string huid     = event->getAttribute("huid");
            handler->write("User with huid: '"+huid+"'"
                + " successfull invited"
                + " to conference: '"+ conf_id + "'\n");

        } else if (type == "conferenceCreated"){
            string conf_id      = event->getAttribute("conf_id");
            string conf_flags   = event->getAttribute("conf_flags");
            handler->write("Was created new conference: '" + conf_id + "'"
                +" flags: '" + conf_flags + "' \n");

        } else if (type == "conferenceMsg"){
            vector<DomElement *>            msgs;
            vector<DomElement *>::iterator  msgs_it;
            DomElement  *msg = NULL;
            string      str;
            string      conf_id = event->getAttribute("conf_id");

            msg = event->getElementByTagName("message");
            str += "New msg: '"     + msg->getData()          + "',"
                " at conference: '" + conf_id                 + "',"
                " msg ID: '"        + msg->getAttribute("id") + "'\n";
            handler->write(str);
        } else if (type == "getContactListSuccess"){
            handler->write(event->serialize(1) + "\n");
        } else {
            PERROR_OBJ(handler, "Unknown event type: '%s', content: '%s'\n",
                type.c_str(),
                element->serialize(1).c_str()
            );
            handler->write(
                "Was received unsupported event type: '" + type + "'\n"
            );
        }
    }

    if (not items.empty()){
        handler->history_line_show();
    }

out:
    if (element){
        DomDocument::deleteElement(element);
        element = NULL;
    }
    return;

fail:
    goto out;
}

void ClientHandlerTelnetObject::do_http_command_cb(
    NetClientHttpObject     *http_client,
    size_t,                 // current_offset,
    size_t,                 // current_length,
    void                    *ctx)
{
    assert(ctx);
    process_answer(http_client, ctx);

    // request to delete us
    http_client->setExpired("1");
}

int ClientHandlerTelnetObject::open_http_client_wait()
{
    int err = 0;
    char buffer[1024] = { 0x00 };

    vector<DomElement *>            res;
    vector<DomElement *>::iterator  res_it;

    NetClientHttpObject *http_api_wait = NULL;
    string url = "";

    getElementsByTagName(
        "ipn_net_client_http",
        res
    );

    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        NetClientHttpObject *client = (NetClientHttpObject *)*res_it;

        if (    ("true"     == client->getAttribute("is_api_wait"))
            &&  ("closed"   == client->getState()))
        {
            DomDocument::deleteElement(client);
            continue;
        }

        if ("true" == client->getAttribute("is_api_wait")){
            // already open
            goto out;
        }
    }

    // open api_wait connection
    http_api_wait = getDocument()
        ->createElement<NetClientHttpObject>("ipn_net_client_http");

    http_api_wait->setAttributeSafe("is_api_wait", "true");

    if (!http_api_wait){
        PERROR_OBJ(this, "Cannot create 'net_client_http' element\n");
        write("Cannot open 'api_wait' connection\n");
        goto fail;
    }

#ifdef HOSTOS
    // use HOSTOS sockets
    http_api_wait->useHostOS();
#endif

    // attach to current element
    appendChildSafe(http_api_wait);

    http_api_wait->setHttpPartialReadCb(
        ClientHandlerTelnetObject::http_api_wait_cb,
        (void *)this
    );

    http_api_wait->setHttpConnectClosedCb(
        ClientHandlerTelnetObject::http_api_wait_closed_cb,
        (void *)this
    );

    // set body
    snprintf(buffer, sizeof(buffer),
        "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"
        "<ipnoise ver=\"0.01\" sessid=\"%s\">"
        "  <commands>"
        "    <command type=\"ping\" ver=\"0.01\"/>"
        "  </commands>"
        "</ipnoise>",
        apiGetSessionId().c_str()
    );

    http_api_wait->setRequestContent(buffer);
    http_api_wait->setRequestContentType("text/html");

    // get url
    url = "http://" + apiGetHostPort() + "/ipnoise/api";

    http_api_wait->post(url);

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int ClientHandlerTelnetObject::do_http_command(string &command)
{
    int res, err = -1;
    NetClientHttpObject *net_client_http = NULL;

    string url = "";

    // create network client
    net_client_http = getDocument()
        ->createElement<NetClientHttpObject>("ipn_net_client_http");
    if (!net_client_http){
        PERROR_OBJ(this, "Cannot create 'net_client_http' element\n");
        write("Cannot open 'api' connection\n");
        goto fail;
    }

#ifdef HOSTOS
    // use HOSTOS sockets
    net_client_http->useHostOS();
#endif

    // attach to current element
    appendChildSafe(net_client_http);

    // setup callbacks
    net_client_http->setHttpAnswerCb(
        ClientHandlerTelnetObject::do_http_command_cb,
        (void *)this
    );

    // prepare url
    url = "http://" + apiGetHostPort() + "/ipnoise/api";

    // set body
    net_client_http->setRequestContent(command);
    net_client_http->setRequestContentType("text/xml");

    // get url
    res = net_client_http->post(url);
    if (res){
        write("Connect failed to: '" + apiGetHostPort() + "'\n");
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// ----------------------- IPNoise client state -----------------------

string ClientHandlerTelnetObject::apiGetSessionId()
{
    return getAttribute("api_sessid");
}

void ClientHandlerTelnetObject::apiSetSessionId(string &_sessid)
{
    setAttributeSafe("api_sessid", _sessid);
}

string ClientHandlerTelnetObject::apiGetHostPort()
{
    return getAttribute("api_host_port");
}

void ClientHandlerTelnetObject::apiSetHostPort(string &_host_port)
{
    setAttributeSafe("api_host_port", _host_port);
}

// ----------------------- Signals, Slots -----------------------------------80

void ClientHandlerTelnetObject::slot(
    const string    &,  // signal_name,
    DomElement      *)  // object
{
}

void ClientHandlerTelnetObject::registrySignals()
{
    getDocument()->registrySignal(
        "packet_incoming",
        "ipn_skbuff"
    );
}

void ClientHandlerTelnetObject::registrySlots()
{
    getDocument()->registrySlot(
        this,
        "packet_incoming",
        "ipn_skbuff"
    );

    getDocument()->registrySlot(
        this,
        "packet_outcoming",
        "ipn_skbuff"
    );
}


