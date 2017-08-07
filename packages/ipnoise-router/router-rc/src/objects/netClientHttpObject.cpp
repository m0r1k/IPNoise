/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev May 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

#include <ipnoise-common/strings.h>
#include "objects/netClientHttpObject.h"

NetClientHttpObject * NetClientHttpObject::create_object(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
{
    return new NetClientHttpObject(a_node, a_doc, a_tagname);
}

NetClientHttpObject::NetClientHttpObject(
    xmlNodePtr      a_node,
    DomDocument     *a_doc,
    const string    &a_tagname)
    :   NetClientObject(a_node, a_doc, a_tagname)
{
    setRequestMethod("GET");
    setRequestProtocol("HTTP/1.1");

    setRequestContent("");
    setRequestContentType("text/html");

    http_partial_read_cb        = NULL;
    http_partial_read_cb_ctx    = NULL;

    http_connected_cb           = NULL;
    http_connected_cb_ctx       = NULL;

    http_connect_closed_cb      = NULL;
    http_connect_closed_cb_ctx  = NULL;

    http_answer_cb              = NULL;
    http_answer_cb_ctx          = NULL;
};

NetClientHttpObject::~NetClientHttpObject()
{
};

// ----------------- requested callbacks ------------------------------------80

// partial read callback
void NetClientHttpObject::setHttpPartialReadCb(
    void (*_http_partial_read_cb)(
        NetClientHttpObject     *client,
        size_t                  current_offset,
        size_t                  current_length,
        void                    *ctx
    ),
    void *_http_partial_read_cb_ctx)
{
    http_partial_read_cb     = _http_partial_read_cb;
    http_partial_read_cb_ctx = _http_partial_read_cb_ctx;
}

// connection established callback
void NetClientHttpObject::setHttpConnectedCb(
    void (*_http_connected_cb)(
        NetClientHttpObject     *client,
        size_t                  current_offset,
        size_t                  current_length,
        void                    *ctx
    ),
    void *_http_connected_cb_ctx)
{
    http_connected_cb      = _http_connected_cb;
    http_connected_cb_ctx  = _http_connected_cb_ctx;
}

// connection closed callback
void NetClientHttpObject::setHttpConnectClosedCb(
    void (*_http_connect_closed_cb)(
        NetClientHttpObject     *client,
        size_t                  current_offset,
        size_t                  current_length,
        void                    *ctx
    ),
    void *_http_connect_closed_cb_ctx)
{
    http_connect_closed_cb      = _http_connect_closed_cb;
    http_connect_closed_cb_ctx  = _http_connect_closed_cb_ctx;
}

// http answer callback
void NetClientHttpObject::setHttpAnswerCb(
    void (*_http_answer_cb)(
        NetClientHttpObject     *client,
        size_t                  current_offset,
        size_t                  current_length,
        void                    *ctx
    ),
    void *_http_answer_cb_ctx)
{
    http_answer_cb      = _http_answer_cb;
    http_answer_cb_ctx  = _http_answer_cb_ctx;
}

// ------------------- our callbacks ----------------------------------------80

void NetClientHttpObject::partial_read_cb(
    NetClientObject         *, // *client
    size_t                  current_offset,
    size_t                  current_length,
    void                    *ctx)
{
    assert(ctx);
    char buffer[1024] = { 0x00 };

    ContentIt content_i;
    NetClientHttpObject *http_client = (NetClientHttpObject *)ctx;

    snprintf(buffer, sizeof(buffer), "[http_partial_read_cb]"
        " current_offset: '%u', current_length: '%u'\n",
        current_offset,
        current_length
    );

again:
    if (!http_client->answer_headers_was_read){
        string line = "";
        for (content_i = http_client->content.begin();
            content_i != http_client->content.end();
            content_i++)
        {
            if (*content_i == '\n'){
                if (line == ""){
                    // end of headers
                    http_client->answer_headers_was_read = 1;
                    http_client->content.erase(
                        http_client->content.begin(),
                        content_i + 1
                    );
                    goto again;
                }

                if (!http_client->answer_status_line.size()){
                    // store status line
                    http_client->setAnswerStatusLine(line);
                } else {
                    // store header
                    string::size_type pos = string::npos;
                    string key      = "";
                    string value    = "";

                    pos = line.find(":");
                    if (pos != string::npos){
                        key = line.substr(0, pos);
                        // skip spaces
                        while (++pos < line.size()){
                            if (line.at(pos) != ' '){
                                break;
                            }
                        }
                        value = line.substr(pos);
                        http_client->setAnswerHeader(key, value);
                    }
                }
                line = "";
            } else {
                line += *content_i;
            }
        }
        http_client->content.erase(http_client->content.begin(), content_i);
    } else {
        // read body
        if (http_client->http_partial_read_cb){
            http_client->http_partial_read_cb(
                http_client,
                current_offset,
                current_length,
                http_client->http_partial_read_cb_ctx
            );
        }
        size_t content_length        = http_client->content.size();
        size_t content_length_header = http_client->getAnswerContentLength();
        if (content_length >= content_length_header){
            http_client->close();
        }
    }
}

void NetClientHttpObject::connected_cb(
    NetClientObject         *, // *client
    size_t                  current_offset,
    size_t                  current_length,
    void                    *ctx)
{
    assert(ctx);
    char buffer[1024] = { 0x00 };
    string request = "";

    HEADERS             headers;
    HEADERS::iterator   headers_i;

    NetClientHttpObject *http_client = (NetClientHttpObject *)ctx;

    snprintf(buffer, sizeof(buffer), "[http_connected_cb]"
        " current_offset: '%u', current_length: '%u'\n",
        current_offset,
        current_length
    );

    if (http_client->http_connected_cb){
        http_client->http_connected_cb(
            http_client,
            current_offset,
            current_length,
            http_client->http_connected_cb_ctx
        );
    };

    // create request
    request += http_client->getRequestMethod();
    request += " " + http_client->url_info["path"];
    request += " " + http_client->url_info["protocol"];
    request += "\n";

    // add headers
    headers = http_client->getRequestHeaders();
    for (headers_i = headers.begin();
        headers_i != headers.end();
        headers_i++)
    {
        request += headers_i->first + ":" + headers_i->second + "\n";
    }

    request += "\n";

    request += http_client->getRequestContent();

    http_client->write(request);
}

void NetClientHttpObject::connect_closed_cb(
    NetClientObject         *, // *client
    size_t                  current_offset,
    size_t                  current_length,
    void                    *ctx)
{
    assert(ctx);
    char buffer[1024] = { 0x00 };
    NetClientHttpObject *http_client = (NetClientHttpObject *)ctx;

    snprintf(buffer, sizeof(buffer), "[http_connect_closed_cb]"
        " current_offset: '%u', current_length: '%u'\n",
        current_offset,
        current_length
    );

    if (http_client->http_connect_closed_cb){
        http_client->http_connect_closed_cb(
            http_client,
            current_offset,
            current_length,
            http_client->http_connect_closed_cb_ctx
        );
    };

    // parse answer and call http answer call back
    if (http_client->http_answer_cb){
        http_client->http_answer_cb(
            http_client,
            current_offset,
            current_length,
            http_client->http_answer_cb_ctx
        );
    };

}

// --------------------------------------------------------------------------80

int NetClientHttpObject::setAnswerProtocol(string _protocol)
{
    int err = 0;

    answer_protocol = _protocol;

    return err;
}

string NetClientHttpObject::getAnswerProtocol()
{
    return answer_protocol;
}

int NetClientHttpObject::setAnswerRespCode(string _resp_code)
{
    int err = 0;

    answer_resp_code = _resp_code;

    return err;
}

string NetClientHttpObject::getAnswerRespCode()
{
    return answer_resp_code;
}

int NetClientHttpObject::setAnswerRespDescr(string _resp_descr)
{
    int err = 0;

    answer_resp_descr = _resp_descr;

    return err;
}

string NetClientHttpObject::getAnswerHeader(string name)
{
    return answer_headers[name];
}

string NetClientHttpObject::getAnswerContent()
{
    string ret = "";
    int i;

    for (i = 0; i < (int)content.size(); i++){
        ret += (char)content.at(i);
    }

    return ret;
}

string NetClientHttpObject::getAnswerContentType()
{
    return getAnswerHeader("Content-type");
}

size_t NetClientHttpObject::getAnswerContentLength()
{
    return (size_t)(atoi(getAnswerHeader("Content-length").c_str()));
}

string NetClientHttpObject::getAnswerRespDescr()
{
    return answer_resp_descr;
}

int NetClientHttpObject::setAnswerStatusLine(string line)
{
    int err = 0;

    answer_status_line = line;

    Splitter arguments(answer_status_line, " ");
    if (arguments.size() > 0){
        setAnswerProtocol(arguments[0]);
    }
    if (arguments.size() > 1){
        setAnswerRespCode(arguments[1].c_str());
    }
    if (arguments.size() > 2){
        setAnswerRespDescr(arguments[2]);
    }

    return err;
}

int NetClientHttpObject::setRequestHeader(string name, string value)
{
    int err = -1;

    if (!name.size()){
        goto fail;
    }

    request_headers[name] = value;

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

int NetClientHttpObject::unsetRequestHeader(string name)
{
    int err = -1;
    HEADERS::iterator headers_i;

    if (!name.size()){
        goto fail;
    }

    headers_i = request_headers.find(name);
    if (headers_i != request_headers.end()){
        request_headers.erase(headers_i);
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

HEADERS NetClientHttpObject::getRequestHeaders()
{
    return request_headers;
}

int NetClientHttpObject::setRequestContent(string content)
{
    int err = 0;
    request_content = content;
    return err;
}

string NetClientHttpObject::getRequestContent()
{
    return request_content;
}

int NetClientHttpObject::setRequestContentType(string content_type)
{
    return setRequestHeader("Content-Type", content_type);
}

string NetClientHttpObject::getRequestContentType()
{
    return getRequestHeader("Content-Type");
}

int NetClientHttpObject::setAnswerHeader(string name, string value)
{
    int err = -1;

    if (!name.size()){
        goto fail;
    }

    answer_headers[name] = value;

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

int NetClientHttpObject::unsetAnswerHeader(string name)
{
    int err = -1;
    HEADERS::iterator headers_i;

    if (!name.size()){
        goto fail;
    }

    headers_i = answer_headers.find(name);
    if (headers_i != answer_headers.end()){
        answer_headers.erase(headers_i);
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

HEADERS NetClientHttpObject::getAnswerHeaders()
{
    return answer_headers;
}

int NetClientHttpObject::setRequestMethod(string _method)
{
    int err = -1;

    if (    _method != "GET"
        &&  _method != "POST")
    {
        PERROR_OBJ(this, "Unsupported method: '%s'\n", _method.c_str());
        goto fail;
    }

    request_method = _method;

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

string NetClientHttpObject::getRequestMethod()
{
    return request_method;
}

int NetClientHttpObject::setRequestProtocol(string _protocol)
{
    int err = -1;

    if (_protocol != "HTTP/1.1"){
        PERROR_OBJ(this, "Unsupported protocol: '%s'\n", _protocol.c_str());
        goto fail;
    }

    request_protocol = _protocol;

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

string NetClientHttpObject::getRequestProtocol()
{
    return request_protocol;
}

int NetClientHttpObject::send_request(string url)
{
    int res, err = -1;
    int port_int = 0;

    // clear answer
    answer_headers_was_read = 0;
    answer_status_line      = "";
    answer_headers.clear();

    // setup default values
    url_info["protocol"]    = "HTTP/1.1";
    url_info["domain"]      = "localhost";
    url_info["port"]        = "80";
    url_info["path"]        = "/";

    utils::parse_url(url, url_info);

    port_int = atoi(url_info["port"].c_str());

    // setup callbacks
    setPartialReadCb(
        NetClientHttpObject::partial_read_cb,
        (void *)this
    );
    setConnectedCb(
        NetClientHttpObject::connected_cb,
        (void *)this
    );
    setConnectClosedCb(
        NetClientHttpObject::connect_closed_cb,
        (void *)this
    );

    // setup "Host" header
    setRequestHeader("Host", url_info["domain"]);

    // do connect
    res = connect(url_info["domain"], port_int);
    if (res){
        PERROR_OBJ(this, "Connect to '%s:%d' failed\n",
            url_info["domain"].c_str(),
            port_int
        );
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

string NetClientHttpObject::getRequestHeader(string name)
{
    string ret = "";
    HEADERS::iterator request_headers_i;

    request_headers_i = request_headers.find(name);
    if (request_headers_i != request_headers.end()){
        ret = request_headers_i->second;
    }
    return ret;
}

int NetClientHttpObject::get(string url)
{
    // setup method
    setRequestMethod("GET");

    // all ok
    return send_request(url);
}

int NetClientHttpObject::post(string url)
{
    // setup method
    setRequestMethod("POST");

    // do request
    return send_request(url);
}

