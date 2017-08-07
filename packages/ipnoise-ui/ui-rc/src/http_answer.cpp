#include "http_request.hpp"
#include "http_answer.hpp"

HttpAnswer::HttpAnswer()
{
    do_init();
}

HttpAnswer::HttpAnswer(const HttpRequest &a_req)
    :   HttpRequest(a_req)
{
    do_init();
}

HttpAnswer::~HttpAnswer()
{
}

void HttpAnswer::do_init()
{
    setAnswerHeader("Content-type", "text/html");

    m_need_disconnect = 1;
}

void HttpAnswer::needDisconnect(
    int32_t a_val)
{
    m_need_disconnect = a_val;
}

int32_t HttpAnswer::needDisconnect()
{
    return m_need_disconnect;
}

void HttpAnswer::setAnswerHeader(
    const string    &a_key,
    const string    &a_val)
{
    string key = a_key;
    transform(
        key.begin(),
        key.end(),
        key.begin(),
        ::tolower
    );
    m_answer_headers[key] = PROP_STRING(a_val);
}

// -------------------------- status code -----------------------------------80

void HttpAnswer::setAnswerStatusCode(
    const int32_t   &a_code)
{
    m_answer_status_code = a_code;
}

int32_t HttpAnswer::getAnswerStatusCode()
{
    return m_answer_status_code;
}

// -------------------------- proto string ----------------------------------80

void HttpAnswer::setAnswerProto(
    const string    &a_proto)
{
    m_answer_proto = a_proto;
}

string HttpAnswer::getAnswerProto()
{
    return m_answer_proto;
}

// -------------------------- status string ---------------------------------80

void HttpAnswer::setAnswerStatusString(
    const string    &a_string)
{
    m_answer_status_string = a_string;
}

string HttpAnswer::getAnswerStatusString()
{
    return m_answer_status_string;
}

// -------------------------- body ------------------------------------------80

void HttpAnswer::setAnswerBody(
    const string    &a_string)
{
    char buffer[512] = { 0x00 };

    m_answer_body = a_string;
    snprintf(buffer, sizeof(buffer),
        "%d",
        (int32_t)m_answer_body.size()
    );
    setAnswerHeader("Content-length", buffer);
}

string HttpAnswer::getAnswerBody()
{
    return m_answer_body;
}

// -------------------------- answer api ------------------------------------80

void HttpAnswer::setAnswerApiStatus(
    const string &a_status)
{
    PropMapSptr api = getCreateAnswerApi();
    api->add("status", a_status);
}

void HttpAnswer::setAnswerApiDescr(
    const string &a_descr)
{
    PropMapSptr api = getCreateAnswerApi();
    api->add("descr", a_descr);
}

PropMapSptr HttpAnswer::getAnswerApi()
{
    return m_answer_api;
}


PropMapSptr HttpAnswer::getCreateAnswerApi()
{
    PropMapSptr api = getAnswerApi();
    if (!api){
        api = PROP_MAP();
        m_answer_api = api;
    }

    return api;
}

PropMapSptr HttpAnswer::getAnswerParams()
{
    PropMapSptr params;
    if (m_answer_api){
        params = dynamic_pointer_cast<PropMap>(
            m_answer_api->get("params")
        );
    }
    return params;
}

PropMapSptr HttpAnswer::getCreateAnswerParams()
{
    PropMapSptr api     = getCreateAnswerApi();
    PropMapSptr params  = dynamic_pointer_cast<PropMap>(
        api->get("params")
    );

    if (!params){
        params = PROP_MAP();
        api->add("params", params);
    }

    return params;
}

void HttpAnswer::setAnswerParam(
    const string &a_name,
    const string &a_val)
{
    PropMapSptr     params = getCreateAnswerParams();
    PropStringSptr  val(new PropString(a_val));
    params->add(a_name, val);
}

void HttpAnswer::setAnswerParam(
    const string    &a_name,
    PropSptr        a_val)
{
    PropMapSptr params = getCreateAnswerParams();
    params->add(a_name, a_val);
}

void HttpAnswer::setAnswerApiReqId(
    const string &a_val)
{
    PropMapSptr api = getCreateAnswerApi();
    api->add("req_id", a_val);
}

void HttpAnswer::setAnswerApiSessid(
    const string &a_val)
{
    PropMapSptr api = getCreateAnswerApi();
    api->add("sessid", a_val);
}

// -------------------------- cookie ----------------------------------------80

void HttpAnswer::setAnswerCookie(
    const string    &a_name,    // Example: login
    const string    &a_val,     // Example: tigra
    const string    &a_expires, // Example: Fri, 31 Dec 2010 23:59:59 GMT
    const string    &a_path,    // Example: /
    const string    &a_domain,  // Example: .example.net
    const int32_t   &a_secure)
{
    string line;

    // add key and val
    line += a_name + "=" + a_val + ";";

    // add others info
    if (a_expires.size()){
        line += "expires=" + a_expires  + ";";
    }
    if (a_path.size()){
        line += "path=" + a_path + ";";
    }
    if (a_domain.size()){
        line += "domain=" + a_domain + ";";
    }
    if (a_secure){
        line += "secure";
    }

    setAnswerHeader("Set-Cookie", line);
}

// --------------------------------------------------------------------------80

string HttpAnswer::toHttp()
{
    char            buffer[65535] = { 0x00 };
    string          http;
    PropMapIt  it;

    snprintf(buffer, sizeof(buffer),
        "%s %d %s\r\n",
        m_answer_proto.c_str(),
        m_answer_status_code,
        m_answer_status_string.c_str()
    );
    http += buffer;

    for (it = m_answer_headers.begin();
        it != m_answer_headers.end();
        it++)
    {
        PropSptr    prop_val = it->second;
        string      val;
        if (prop_val){
            val = prop_val->toString();
        }
        http += it->first->toString();
        http += ": ";
        http += val;
        http += "\r\n";
    }

    http += "\r\n";
    if (m_answer_body.size()){
        http += m_answer_body;
    } else if ("POST" == getReqMethod()){
        // add params in answer
        if (m_answer_api){
            http += m_answer_api->serialize(": ");
        }
    }

    return http;
}

string HttpAnswer::dump()
{
    char    buffer[65535] = { 0x000 };
    string  dump;

    dump += HttpRequest::dump();
    dump += "answer:\n";
    dump += "  proto: " + m_answer_proto + "\n";
    {
        snprintf(buffer, sizeof(buffer),
            "%d",
            m_answer_status_code
        );
        dump += "  status_code: ";
        dump += buffer;
        dump += "\n";
    }
    dump += "  status_string: " + m_answer_status_string + "\n";
    dump += "  body: '" + m_answer_body + "'";

    return dump;
}

