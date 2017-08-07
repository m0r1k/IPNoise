/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Jul 2010 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/client/handlerHttp/clientHandlerHttpObject.h"
#include "objects/clientObject.h"

ClientObject * ClientObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ClientObject(a_node, a_doc, a_tagname);
}

ClientObject::ClientObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   NetClientObject(a_node, a_doc, a_tagname)
{
    // do remove us after exit (will be called ClientObject::expired)
    setExpired("0");

    // device name for packet rx
    setRxDevName(CLIENTS_DEV_NAME);
    setState("connected");

    setPartialReadCb(
        ClientObject::partialReadCb,
        this
    );

    setConnectClosedCb(
        ClientObject::connectClosedCb,
        this
    );
};

ClientObject::~ClientObject()
{
};

void ClientObject::partialReadCb(
    NetClientObject     *,  // net_client,
    size_t,                 // current_offset,
    size_t,                 // current_length
    void                *ctx)
{
    ClientObject    *client     = (ClientObject *)ctx;
    ClientHandler   *handler    = NULL;

    if (client->content.empty()){
        goto out;
    }

    // search handler
    handler = client->getHandler();

    // setup client handler if need
    if (!handler){
        if (    ("\n"    == client->content)
            ||  ("\r\n"  == client->content))
        {
            // this is text based client
            client->setType("ipn_client_handler_telnet");
            handler = client->getDocument()
                ->createElement<ClientHandler>(client->getType());
        } else if ( ("GET "  == client->content.substr(0, 4))
                ||  ("POST " == client->content.substr(0, 5)))
        {
            // this is http based client
            client->setType("ipn_client_handler_http");
            handler = client->getDocument()
                ->createElement<ClientHandler>(client->getType());
        } else if ( ("<?xml "    ==  client->content.substr(0, 6))
                ||  ("<?XML "    ==  client->content.substr(0, 6))
                ||  ("<ipnoise " ==  client->content.substr(0, 9)))
        {
            // this is raw based client
            client->setType("ipn_client_handler_raw");
            handler = client->getDocument()
                ->createElement<ClientHandler>(client->getType());
        } else {
            // unknown client type
            client->setType("ipn_client_handler_unknown");
            handler = client->getDocument()
                ->createElement<ClientHandler>(client->getType());
        }

        // store handler
        client->appendChildSafe(handler);
        // init handler
        handler->do_init();
        // read data (mark as first read)
        handler->read_cb(client->content, ClientHandler::READ_STATE_FIRST_READ);
    } else {
        // read data
        handler->read_cb(client->content, ClientHandler::READ_STATE_READ);
    }

    client->content.clear();

out:
    return;
}

void ClientObject::connectClosedCb(
    NetClientObject         *,  // client
    size_t,                     // current_offset,
    size_t,                     // current_length,
    void                    *ctx)
{
    ClientObject    *client     = (ClientObject *)ctx;
    ClientHandler   *handler    = NULL;

    handler = client->getHandler();
    if (handler){
        handler->closed_cb();
    }

    // mark us as closed
    client->setState("closed");

    // request delete us
    client->setExpired("1");
}

string ClientObject::getRxDevName()
{
    return getAttribute("rx_dev_name");
}

void ClientObject::setRxDevName(const string &dev_name)
{
    assert(
            (CLIENTS_DEV_NAME   == dev_name)
        ||  (IPNOISE_DEV_NAME   == dev_name)
    );
    setAttributeSafe("rx_dev_name", dev_name);
}

void ClientObject::setSessId(const string &sessid)
{
    setAttributeSafe("sessid", sessid);
};

string ClientObject::getSessId()
{
    return getAttribute("sessid");
}

void ClientObject::setType(const string &type)
{
    setAttributeSafe("type", type);
};

string ClientObject::getType()
{
    return getAttribute("type");
}

ClientHandler * ClientObject::getHandler()
{
    ClientHandler *handler = NULL;
    handler = (ClientHandler *)getFirstChild();
    return handler;
}

ssize_t ClientObject::send(const string &buffer)
{
    return NetClientObject::write(buffer);
}

ssize_t ClientObject::write(const string &buffer)
{
    ssize_t ret = -1;

    ClientHandler *handler = NULL;
    handler = getHandler();

    if (!handler){
        goto out;
    }

    if ("ipn_client_handler_http" == getType()){
        ClientHandlerHttpObject *http = NULL;
        http = (ClientHandlerHttpObject *)handler;
        http->answer.resp_code    = 200;
        http->answer.resp_line    = "OK";
        http->answer.content_type = "text/xml";
        http->answer.content      = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
        http->answer.content      += buffer;
        http->answer.needClose(1);
        ret = http->process_answer();
    } else {
        ret = handler->write(buffer);
    }

out:
    return ret;
}

