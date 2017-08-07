/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Feb 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "objects/ipnoiseObject.h"
#include "objects/usersObject.h"

// client commands
#include "objects/api/command/client/apiCommandClientPing.hpp"
#include "objects/api/command/client/apiCommandClientLogin.hpp"
#include "objects/api/command/client/apiCommandClientLogout.hpp"
#include "objects/api/command/client/apiCommandClientAddContact.hpp"
#include "objects/api/command/client/apiCommandClientAddItem.hpp"
#include "objects/api/command/client/apiCommandClientDelItem.hpp"
#include "objects/api/command/client/apiCommandClientGetItem.hpp"
#include "objects/api/command/client/apiCommandClientGetContactList.hpp"
#include "objects/api/command/client/apiCommandClientUpdateContact.hpp"
#include "objects/api/command/client/apiCommandClientUpdateItem.hpp"
#include "objects/api/command/client/apiCommandClientAddGroup.hpp"
#include "objects/api/command/client/apiCommandClientDelGroup.hpp"
#include "objects/api/command/client/apiCommandClientConferenceJoin.hpp"
#include "objects/api/command/client/apiCommandClientConferenceInvite.hpp"
#include "objects/api/command/client/apiCommandClientConferenceMsg.hpp"
#include "objects/api/command/client/apiCommandClientSearchUsers.hpp"

// ipnoise commands
#include "objects/api/command/ipnoise/apiCommandIPNoiseConferenceInvite.hpp"
#include "objects/api/command/ipnoise/apiCommandIPNoiseConferenceJoin.hpp"
#include "objects/api/command/ipnoise/apiCommandIPNoiseConferenceMsg.hpp"
#include "objects/api/command/ipnoise/apiCommandIPNoiseGetUserInfo.hpp"

// link commands
#include "objects/api/command/link/apiCommandLinkSearchRoute.hpp"
#include "objects/api/command/link/apiCommandLinkSearchUsers.hpp"

#include "objects/netCommandObject.h"

static NetCommands *s_commands_client  = NULL;
static NetCommands *s_commands_link    = NULL;
static NetCommands *s_commands_ipnoise = NULL;

NetCommandObject * NetCommandObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new NetCommandObject(a_node, a_doc, a_tagname);
}

NetCommandObject::NetCommandObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   NetCommandAndEventObject(a_node, a_doc, a_tagname)
{

    // client commands
    if (!s_commands_client){
        s_commands_client = new NetCommands();
        registerCommandsClient();
    }

    // link commands
    if (!s_commands_link){
        s_commands_link = new NetCommands();
        registerCommandsLink();
    }

    // IPNoise commands
    if (!s_commands_ipnoise){
        s_commands_ipnoise = new NetCommands();
        registerCommandsIPNoise();
    }
};

NetCommandObject::~NetCommandObject()
{
    if (s_commands_client){
        free(s_commands_client);
    }
    if (s_commands_link){
        free(s_commands_link);
    }
    if (s_commands_ipnoise){
        free(s_commands_ipnoise);
    }
};

void NetCommandObject::registerCommandsClient()
{
    ApiCommand *command = NULL;

    // ping
    {
        command = new ApiCommandClientPing();
        (*s_commands_client)[command->getName()] = command;
    }

    // login
    {
        command = new ApiCommandClientLogin();
        (*s_commands_client)[command->getName()] = command;
    }

    // logout
    {
        command = new ApiCommandClientLogout();
        (*s_commands_client)[command->getName()] = command;
    }

    // addContact
    {
        command = new ApiCommandClientAddContact();
        (*s_commands_client)[command->getName()] = command;
    }

    // addItem
    {
        command = new ApiCommandClientAddItem();
        (*s_commands_client)[command->getName()] = command;
    }

    // delItem
    {
        command = new ApiCommandClientDelItem();
        (*s_commands_client)[command->getName()] = command;
    }

    // getItem
    {
        command = new ApiCommandClientGetItem();
        (*s_commands_client)[command->getName()] = command;
    }

    // getContactList
    {
        command = new ApiCommandClientGetContactList();
        (*s_commands_client)[command->getName()] = command;
    }

    // updateContact
    {
        command = new ApiCommandClientUpdateContact();
        (*s_commands_client)[command->getName()] = command;
    }

    // updateItem
    {
        command = new ApiCommandClientUpdateItem();
        (*s_commands_client)[command->getName()] = command;
    }

    // addGroup
    {
        command = new ApiCommandClientAddGroup();
        (*s_commands_client)[command->getName()] = command;
    }

    // delGroup
    {
        command = new ApiCommandClientDelGroup();
        (*s_commands_client)[command->getName()] = command;
    }

    // ConferenceJoin
    {
        command = new ApiCommandClientConferenceJoin();
        (*s_commands_client)[command->getName()] = command;
    }

    // ConferenceInvite
    {
        command = new ApiCommandClientConferenceInvite();
        (*s_commands_client)[command->getName()] = command;
    }

    // ConferenceMsg
    {
        command = new ApiCommandClientConferenceMsg();
        (*s_commands_client)[command->getName()] = command;
    }

    // SearchUsers
    {
        command = new ApiCommandClientSearchUsers();
        (*s_commands_client)[command->getName()] = command;
    }
}

void NetCommandObject::registerCommandsLink()
{
    ApiCommand *command = NULL;

    // searchRoute
    {
        command = new ApiCommandLinkSearchRoute();
        (*s_commands_link)[command->getName()] = command;
    }

    // searchUsers
    {
        command = new ApiCommandLinkSearchUsers();
        (*s_commands_link)[command->getName()] = command;
    }
}

void NetCommandObject::registerCommandsIPNoise()
{
    ApiCommand *command = NULL;

    // conferenceInvite
    {
        command = new ApiCommandIPNoiseConferenceInvite();
        (*s_commands_ipnoise)[command->getName()] = command;
    }

    // conferenceJoin
    {
        command = new ApiCommandIPNoiseConferenceJoin();
        (*s_commands_ipnoise)[command->getName()] = command;
    }

    // conferenceMsg
    {
        command = new ApiCommandIPNoiseConferenceMsg();
        (*s_commands_ipnoise)[command->getName()] = command;
    }

    // getUserInfo
    {
        command = new ApiCommandIPNoiseGetUserInfo();
        (*s_commands_ipnoise)[command->getName()] = command;
    }
}

// ------------------- process commands  ------------------------------------80

int NetCommandObject::processCommand(SkBuffObject *skb)
{
    int err = -1;
    ApiCommand      *command = NULL;
    string          cmd_type = getAttribute("type");
    NetCommandsIt   commands_i;
    string          indev;

    PDEBUG_OBJ(this, 5, "NetCommandObject::processCommand"
        " cmd_type: '%s', indev_name: '%s'\n",
        cmd_type.c_str(),
        skb->getAttribute("indev_name").c_str()
    );

    if (!cmd_type.size()){
        err = errorUnsupportedCommand(skb);
        goto out;
    }

    // search handler
    indev = skb->getAttribute("indev_name");
    if (indev == CLIENTS_DEV_NAME){
        commands_i = s_commands_client->find(cmd_type);
        if (commands_i != s_commands_client->end()){
            command = commands_i->second;
        }
    } else if ( (indev == IPNOISE_DEV_NAME)
            ||  (indev == LO_DEV_NAME))
    {
        commands_i = s_commands_ipnoise->find(cmd_type);
        if (commands_i != s_commands_ipnoise->end()){
            command = commands_i->second;
        }
    } else {
        // may be link command?
        commands_i = s_commands_link->find(cmd_type);
        if (commands_i != s_commands_link->end()){
            command = commands_i->second;
        }
    }

    if (!command){
        // handler not found
        err = errorUnsupportedCommand(skb);
        goto out;
    }

    // call handler
    err = command->process(this, skb);

out:
    return err;
}

