/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev Mar 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include "libxml2/domDocument.h"
#include "objects/serverObject.h"

#include "clientHandlerHttpObject.h"

ClientHandlerHttpObject * ClientHandlerHttpObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new ClientHandlerHttpObject(a_node, a_doc, a_tagname);
}

ClientHandlerHttpObject::ClientHandlerHttpObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   ClientHandler(a_node, a_doc, a_tagname)
{
    request.do_init();
    answer.do_init();
};

ClientHandlerHttpObject::~ClientHandlerHttpObject()
{
};

void ClientHandlerHttpObject::registrySignals()
{
//    registrySignal(
//        "packet_incoming",
//        "ipn_skbuff"
//    );
}

void ClientHandlerHttpObject::registrySlots()
{
//    registrySlot(
//        "packet_outcoming",
//        "ipn_skbuff"
//    );
}

void ClientHandlerHttpObject::slot(
    const string    &,  // signal_name,
    DomElement      *)  // object
{
}

int ClientHandlerHttpObject::do_init()
{
    return 0;
}

string ClientHandlerHttpObject::dump(const char *descr)
{
    string ret = "";

    if (descr){
        ret += descr;
    }

    ret += request.dump();
    ret += answer.dump();

    return ret;
}

int ClientHandlerHttpObject::process_answer()
{
    int     ret     = -1;
    string  data    = answer.getData();

    ret = write(data);

    if (answer.needClose()){
        // close client
        getClient()->close();
        // request to delete
        getClient()->setExpired("1");
    }

    return ret;
}

void ClientHandlerHttpObject::process_req()
{
    SkBuffObject *skb = NULL;

    // answer's protocol same as requested
    answer.resp_proto = request.protocol;

    if (    request.method != "POST"
        &&  request.method != "GET")
    {
        answer.resp_proto   = request.protocol;
        answer.resp_code    = 400;
        answer.resp_line    = "Bad Request"
            ", only 'GET' and 'POST' methods are supported";
        goto out;
    }

    if (request.url == "/ipnoise/api"){
        // process api command
        skb = getDocument()->createElement<SkBuffObject>("ipn_skbuff");
        if (skb == NULL){
            PERROR_OBJ(this, "Cannot allocate new skb\n");
            goto error_500;
        }

        getClient()->setAttributeSafe("MORIK", "data was NOT send");

        skb->setInContent(request.content);
        skb->setInDevName(getClient()->getRxDevName());
        skb->setInDevIndex(getClient()->getIno());

        getDocument()->emitSignalAndDelObj("packet_incoming", skb);
        goto out_dont_send_answer;
    } else if (request.url == "/ipnoise/debug_xml"){
        // TODO XXX check authorization here
        DomElement *element = NULL;
        element = getDocument()->getRootElement();
        answer.resp_code    = 200;
        answer.resp_line    = "OK";
        answer.content_type = "text/xml";
        answer.content      = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
        answer.content      += element->serialize();
        answer.needClose(1);
    } else {
        // unknown url
        answer.resp_proto   = request.protocol;
        answer.resp_code    = 404;
        answer.resp_line    = "Not found";
        answer.content_type = "text/html";
        answer.content      = "<html><body><h1>404 Document or resource was not found</h1><body></html>";
        answer.needClose(1);
    }

out:
    process_answer();

out_dont_send_answer:
    return;

error_500:
    answer.resp_code = 500;
    answer.resp_line = "Internal Error";
    goto out;
}

void ClientHandlerHttpObject::closed_cb()
{
}

int ClientHandlerHttpObject::read_cb(
    const string    &a_data,
    ReadState       rd_state)
{
    int i, cur_pos = 0, err = 0;
    string line = "";

    if (rd_state == READ_STATE_FIRST_READ){
        request.do_init();
    }

    if (request.state <= Request::STATE_PARSE_HEADERS){
        for (cur_pos = 0; cur_pos < (int)a_data.size(); cur_pos++){
            char cur_char = a_data.at(cur_pos);
            switch (cur_char){
                case ('\n'):
                    if (!line.size()){
                        // end of headers
                        request.state = Request::STATE_PARSE_CONTENT;
                        cur_pos++;
                        goto next1;
                    }
                    if (request.state == Request::STATE_PARSE_REQ){
                        // first header line (requset)
                        request.parse_first_line(line);
                        request.state = Request::STATE_PARSE_HEADERS;
                    } else if (request.state == Request::STATE_PARSE_HEADERS){
                        request.parse_header_line(line);
                    }
                    line = "";
                    break;

                case ('\r'):
                    break;

                default:
                    line += a_data.at(cur_pos);
                    break;
            }
        };
    }

next1:
    if (request.state == Request::STATE_PARSE_CONTENT){
        for (i = cur_pos; i < (int)a_data.size(); i++){
            request.content += a_data.at(i);
        }
    }

    process_req();

    return err;
};

