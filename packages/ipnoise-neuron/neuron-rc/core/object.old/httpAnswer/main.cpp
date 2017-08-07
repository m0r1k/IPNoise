#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/httpRequest/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/httpAnswer/main.hpp"

ObjectHttpAnswer::ObjectHttpAnswer(
    const Object::CreateFlags   &a_flags)
    :   ObjectHttpRequest(a_flags)
{
    do_init(a_flags);
}

ObjectHttpAnswer::ObjectHttpAnswer(
    const Object::CreateFlags   &a_flags,
    ObjectHttpRequestSptr       a_req)
    :   ObjectHttpRequest(a_flags, a_req)
{
    do_init(a_flags);
}

ObjectHttpAnswer::~ObjectHttpAnswer()
{
}

void ObjectHttpAnswer::do_init(
    const Object::CreateFlags   &a_flags)
{
    PROP_MAP(m_answer_headers);

    setAnswerHeader("Content-type", "text/html");
}

string ObjectHttpAnswer::getType()
{
    return ObjectHttpAnswer::_getType();
}

void ObjectHttpAnswer::do_init_props()
{
    ObjectHttpRequest::do_init_props();
}

void ObjectHttpAnswer::do_init_api()
{
    ObjectHttpRequest::do_init_api();

    // register actions
}

void ObjectHttpAnswer::setWaitingReqId(
    const string &a_req_id)
{
    m_waiting_req_id = a_req_id;
}

string ObjectHttpAnswer::getWaitingReqId() const
{
    return m_waiting_req_id;
}

void ObjectHttpAnswer::eraseAnswerHeader(
    const string    &a_key)
{
//    m_answer_headers->erase(a_key);
}

void ObjectHttpAnswer::setAnswerHeader(
    const string    &a_key,
    const string    &a_val)
{
    ObjectStringSptr    prop_val;
    string              key = a_key;

    transform(
        key.begin(),
        key.end(),
        key.begin(),
        ::tolower
    );

    PROP_STRING(prop_val, a_val);
    m_answer_headers->add(key, prop_val);
}

// -------------------------- status code -----------------------------------80

void ObjectHttpAnswer::setAnswerStatusCode(
    const int32_t   &a_code)
{
    m_answer_status_code = a_code;
}

int32_t ObjectHttpAnswer::getAnswerStatusCode()
{
    return m_answer_status_code;
}

// -------------------------- proto string ----------------------------------80

void ObjectHttpAnswer::setAnswerProto(
    const string    &a_proto)
{
    m_answer_proto = a_proto;
}

string ObjectHttpAnswer::getAnswerProto()
{
    return m_answer_proto;
}

// -------------------------- status string ---------------------------------80

void ObjectHttpAnswer::setAnswerStatusString(
    const string    &a_string)
{
    m_answer_status_string = a_string;
}

string ObjectHttpAnswer::getAnswerStatusString()
{
    return m_answer_status_string;
}

// -------------------------- body ------------------------------------------80

void ObjectHttpAnswer::eraseAnswerBody()
{
    m_answer_body = "";
    eraseAnswerHeader("Content-length");
}

void ObjectHttpAnswer::setAnswerBody(
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

string ObjectHttpAnswer::getAnswerBody()
{
    return m_answer_body;
}

// -------------------------- answer api ------------------------------------80

void ObjectHttpAnswer::setAnswerApiStatus(
    const string &a_status)
{
    ObjectMapSptr api = getCreateAnswerApi();
//    api->add("status", a_status);
}

void ObjectHttpAnswer::setAnswerApiDescr(
    const string &a_descr)
{
    ObjectMapSptr api = getCreateAnswerApi();
//    api->add("descr", a_descr);
}

void ObjectHttpAnswer::setAnswerApi(
    ObjectMapSptr a_answer_api)
{
    m_answer_api = a_answer_api;
}

// ---------------- answer api ----------------

ObjectMapSptr ObjectHttpAnswer::getAnswerApi()
{
    return m_answer_api;
}

ObjectMapSptr ObjectHttpAnswer::getCreateAnswerApi()
{
    ObjectMapSptr api = getAnswerApi();
    if (!api){
        PROP_MAP(api);
        m_answer_api = api;
    }

    return api;
}

// ---------------- answer headers ----------------

ObjectMapSptr ObjectHttpAnswer::getAnswerHeaders()
{
    return m_answer_headers;
}

ObjectMapSptr ObjectHttpAnswer::getCreateAnswerHeaders()
{
    ObjectMapSptr answer_headers = getAnswerHeaders();
    if (!answer_headers){
        PROP_MAP(answer_headers);
        m_answer_headers = answer_headers;
    }

    return answer_headers;
}

// ---------------- answer params ----------------

ObjectMapSptr ObjectHttpAnswer::getAnswerParams()
{
    ObjectMapSptr params;
//    if (m_answer_api){
//        params = dynamic_pointer_cast<ObjectMap>(
//            m_answer_api->get("params")
//        );
//    }
    return params;
}

ObjectMapSptr ObjectHttpAnswer::getCreateAnswerParams()
{
    ObjectMapSptr api;
    ObjectMapSptr params;

    api     = getCreateAnswerApi();
    params  = dynamic_pointer_cast<ObjectMap>(
        api->get("params")
    );

    if (!params){
        PROP_MAP(params);
        api->add("params", params);
    }

    return params;
}

void ObjectHttpAnswer::setAnswerParam(
    const string &a_name,
    const string &a_val)
{
//    ObjectMapSptr     params = getCreateAnswerParams();
//    ObjectStringSptr  val(new ObjectString(a_val));
//    params->add(a_name, val);
}

void ObjectHttpAnswer::setAnswerParam(
    const string    &a_name,
    ObjectSptr        a_val)
{
//    ObjectMapSptr params = getCreateAnswerParams();
//    params->add(a_name, a_val);
}

void ObjectHttpAnswer::setAnswerApiReqId(
    const string &a_val)
{
//    ObjectMapSptr api = getCreateAnswerApi();
//    api->add("req_id", a_val);
}

void ObjectHttpAnswer::setAnswerApiSessid(
    const string &a_val)
{
//    ObjectMapSptr api = getCreateAnswerApi();
//    api->add("sessid", a_val);
}

// -------------------------- cookie ----------------------------------------80

void ObjectHttpAnswer::setAnswerCookie(
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

string ObjectHttpAnswer::toHttp()
{
    string          http;
    char            buffer[65535] = { 0x00 };
    ObjectMapIt     it;
    string          websock_accept;

    websock_accept = getReqWebsockAccept();

    if (!websock_accept.empty()){
        setAnswerHeader("Upgrade",    "WebSocket");
        setAnswerHeader("Connection", "Upgrade");
        setAnswerHeader(
            "Sec-WebSocket-Accept", websock_accept
        );
        setAnswerStatusCode(101);
        setAnswerStatusString(
            "Web Socket Protocol Handshake"
        );
    }

    snprintf(buffer, sizeof(buffer),
        "%s %d %s\r\n",
        m_answer_proto.c_str(),
        m_answer_status_code,
        m_answer_status_string.c_str()
    );
    http += buffer;

    for (it = m_answer_headers->begin();
        it != m_answer_headers->end();
        it++)
    {
        ObjectSptr    object_val = it->second;
        string      val;
        if (object_val){
            val = object_val->toString();
        }
        http += it->first->toString();
        http += DEFAULT_DELIMITER;
        http += val;
        http += "\r\n";
    }

    http += "\r\n";
    if (m_answer_body.size()){
        http += m_answer_body;
    } else if ("POST" == getReqMethod()){
        // add params in answer
        if (m_answer_api){
            http += m_answer_api->serialize();
        }
    }

    return http;
}

string ObjectHttpAnswer::dump()
{
    char    buffer[65535] = { 0x000 };
    string  dump;

    dump += ObjectHttpRequest::dump();
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

// ---------------- module ----------------

string ObjectHttpAnswer::_getType()
{
    return "core.object.http.answer";
}

string ObjectHttpAnswer::_getTypePerl()
{
    return "core::object::httpAnswer::main";
}

int32_t ObjectHttpAnswer::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectHttpAnswer::_destroy()
{
}

ObjectSptr ObjectHttpAnswer::_object_create()
{
    ObjectSptr object;
    _OBJECT_HTTP_ANSWER(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectHttpAnswer::_getType,
    .type_perl      = ObjectHttpAnswer::_getTypePerl,
    .init           = ObjectHttpAnswer::_init,
    .destroy        = ObjectHttpAnswer::_destroy,
    .object_create  = ObjectHttpAnswer::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

