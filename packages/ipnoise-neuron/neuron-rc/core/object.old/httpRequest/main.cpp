#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/openssl/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/object/__END__.hpp"
#include "strings.hpp"

#include "core/object/httpRequest/main.hpp"

ObjectHttpRequest::ObjectHttpRequest(
    const Object::CreateFlags   &a_flags)
    : Object(a_flags)
{
    do_init(a_flags);
}

ObjectHttpRequest::ObjectHttpRequest(
    const Object::CreateFlags   &a_flags,
    ObjectHttpRequestSptr       a_req)
    :   Object(a_flags)
{
    do_init(a_flags);

    // TODO may be do copy here?
    // 20141019 morik@

    m_req_buff              = a_req->m_req_buff;
    m_req_boundary          = a_req->m_req_boundary;
    m_req_method            = a_req->m_req_method;
    m_req_uri               = a_req->m_req_uri;
    m_req_path              = a_req->m_req_path;
    m_req_proto             = a_req->m_req_proto;
    m_req_boundary_delim    = a_req->m_req_boundary_delim;
    m_req_boundary_delim_end = a_req->m_req_boundary_delim_end;
    m_req_body              = a_req->m_req_body;
    m_req_content_length    = a_req->m_req_content_length;

    m_req_cookies           = a_req->m_req_cookies;
    m_req_api               = a_req->m_req_api;
    m_req_disposition       = a_req->m_req_disposition;

    m_req_headers           = a_req->m_req_headers;
    m_req_parse_state       = a_req->m_req_parse_state;

    m_req_websock_accept    = a_req->m_req_websock_accept;
}

ObjectHttpRequest::~ObjectHttpRequest()
{
}

void ObjectHttpRequest::do_init(
    const Object::CreateFlags   &a_flags)
{
    m_req_content_length = 0;
    m_req_boundary       = 0;
    m_req_parse_state    = ObjectHttpRequest::PARSE_STATE_PROTO;

    PROP_MAP(m_req_cookies);
    PROP_MAP(m_req_api);
    PROP_MAP(m_req_disposition);
    PROP_MAP(m_req_headers);
}

string ObjectHttpRequest::getType()
{
    return ObjectHttpRequest::_getType();
}

void ObjectHttpRequest::do_init_props()
{
//    ObjectMapSptr   prop_req_disposition;
    Object::do_init_props();
//    PROP_MAP(prop_req_disposition);
//    INIT_PROP(this, ReqDisposition, prop_req_disposition);
}

void ObjectHttpRequest::do_init_api()
{
    Object::do_init_api();

    // register actions
}

/*
void ObjectHttpRequest::getAllProps(
    ObjectMapSptr a_props)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SAVE_PROP("req_disposition", getReqDisposition);

    Object::getAllProps(a_props);
}
*/

string ObjectHttpRequest::serialize(
    const string &a_delim)
{
    ObjectMapSptr   tmp;
    PROP_MAP(tmp);

    tmp->add("req_api",             m_req_api);
    tmp->add("req_path",            m_req_path);
    tmp->add("req_uri",             m_req_uri);
    tmp->add("req_proto",           m_req_proto);
    tmp->add("req_method",          m_req_method);
    tmp->add("req_headers",         m_req_headers);
    tmp->add("req_cookies",         m_req_cookies);
    tmp->add("req_body",            m_req_body);
    tmp->add("req_websock_accept",  m_req_websock_accept);
    tmp->add("req_disposition",     m_req_disposition);
    tmp->add("req_boundary_delim",  m_req_boundary_delim);
    tmp->add("req_boundary_delim_end", m_req_boundary_delim_end);

    return tmp->serialize(a_delim);
}

void ObjectHttpRequest::processApi(
    ObjectStringSptr  a_param_name,
    ObjectStringSptr  a_param_value)
{
    if (!a_param_name){
        PFATAL("emty param: 'a_param_name'\n");
    }
    if (!a_param_value){
        PFATAL("emty param: 'a_param_value'\n");
    }

    PDEBUG(100, "ObjectHttpRequest::processApi:\n"
        " a_param_name:     '%s'\n"
        " a_param_value:    '%s'\n",
        a_param_name->toString()->c_str(),
        a_param_value->toString()->c_str()
    );

    m_req_api->add(a_param_name, a_param_value);

    a_param_name  = ObjectStringSptr();
    a_param_value = ObjectStringSptr();
}

void ObjectHttpRequest::parseReqApi()
{
    string::size_type   pos   = string::npos;
    UriParseState       state = URI_PARSE_STATE_PATH;

    string              path;
    ObjectStringSptr    param_name;
    ObjectStringSptr    param_value;

    for (pos = 0; pos < m_req_uri.size(); pos++){
        const char c = m_req_uri.at(pos);
        switch (state){
            case URI_PARSE_STATE_PARAM_VALUE:
                if ('&' == c){
                    processApi(param_name, param_value);
                    state       = URI_PARSE_STATE_PARAM_NAME;
                    param_name  = ObjectStringSptr();
                    param_value = ObjectStringSptr();
                    break;
                } else {
                    param_value->add(c);
                }
                break;

            case URI_PARSE_STATE_PARAM_NAME:
                if (!param_name){
                    PROP_STRING(param_name);
                }
                if (!param_value){
                    PROP_STRING(param_value);
                }
                if ('=' == c){
                    state = URI_PARSE_STATE_PARAM_VALUE;
                    break;
                } else if ('&' == c){
                    processApi(param_name, param_value);
                    state       = URI_PARSE_STATE_PARAM_NAME;
                    param_name  = ObjectStringSptr();
                    param_value = ObjectStringSptr();
                    break;
                } else {
                    param_name->add(c);
                }
                break;

            case URI_PARSE_STATE_PATH:
                if (    '?' == c
                    ||  '&' == c)
                {
                    state = URI_PARSE_STATE_PARAM_NAME;
                    break;
                }
                path += c;
                break;

            default:
                PERROR("invalid state: '%d' while parsing: '%s'\n",
                    state,
                    m_req_uri.c_str()
                );
                break;
        };
    }
    if (    param_name
        &&  param_value)
    {
        processApi(param_name, param_value);
        param_name  = ObjectStringSptr();
        param_value = ObjectStringSptr();
    }
}

void ObjectHttpRequest::contentDisposition(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectMapIt         it;
    string::size_type   pos1, pos2;
    Splitter2           parts(a_val, "; ");
//    ObjectMapSptr       req_disposition;

//    req_disposition = getReqDisposition();

    for (pos1 = 0; pos1 < parts.size(); pos1++){
        Splitter2   key_val(parts[pos1], "= ");
        string      k, v;
        if (!key_val.size()){
            continue;
        }
        k = key_val[0];
        if (2 <= key_val.size()){
            v = key_val[1];
            do {
                // remove " and ' from start and end
                char c = '\0';
                if (!v.size()){
                    break;
                }
                c = v.at(0);
                if (    '"'     == c
                    ||  '\''    == c)
                {
                    v = v.substr(1);
                }
                if (!v.size()){
                    break;
                }
                c = v.at(v.size() - 1);
                if (    '"'     == c
                    ||  '\''    == c)
                {
                    v = v.substr(0, v.size() - 1);
                }
            } while (0);
        }
        m_req_disposition->add(k, v);
    }

    for (it = m_req_disposition->begin();
        it != m_req_disposition->end();
        it++)
    {
        PDEBUG(20, "disposition: '%s' => '%s'\n",
            it->first->toString()->c_str(),
            it->second->toString()->c_str()
        );
    }
}

string ObjectHttpRequest::getReqDispositionName()
{
    string          ret;
//    ObjectMapSptr   req_disposition;

//    req_disposition = getReqDisposition();

    if (m_req_disposition->has("name")){
        ret = m_req_disposition->get("name")->toString();
    }
    return ret;
}

string ObjectHttpRequest::getReqDispositionFileName()
{
    string          ret;
//    ObjectMapSptr   req_disposition;

//    req_disposition = getReqDisposition();

    if (m_req_disposition->has("filename")){
        ret = m_req_disposition->get("filename")->toString();
    }

    return ret;
}

void ObjectHttpRequest::parseContentType(
    const string &a_val)
{
    string::size_type   i, pos;
    Splitter2           parts(a_val, "; ");
    int32_t             multipart_form_data = 0;
    string              boundary;

    for (i = 0; i < parts.size(); i++){
        Splitter2   key_val(parts[i], " =");
        string      key, val;
        if (!key_val.size()){
            continue;
        }
        key = key_val[0];
        if (1 < key_val.size()){
            val = key_val[1];
        }

        if ("multipart/form-data" == key){
            multipart_form_data = 1;
        }

        if ("boundary" == key){
            boundary = val;
        }
    }
    if (    multipart_form_data
        &&  boundary.size())
    {
        m_req_boundary_delim        = boundary;
        m_req_boundary_delim_end    = m_req_boundary_delim + "--";
    }
}

int32_t ObjectHttpRequest::parseHeaderLine(
    const string &a_line)
{
    string              key;
    string              val;
    string::size_type   pos;
    string              line;
    int32_t             err = -1;

    if (!a_line.size()){
        err = 0;
        goto out;
    }

    if (    "\r\n"  == a_line
        ||  "\n"    == a_line)
    {
        if ("POST" != getReqMethod()){
            // end of request
            err = 0;
            goto out;
        }
        if (m_req_boundary_delim.size()){
            m_req_parse_state = PARSE_STATE_BOUNDARY;
        } else {
            m_req_parse_state = PARSE_STATE_CONTENT;
        }
        goto out;
    }

    line = a_line;
    Utils::chomp(line);
    pos = line.find(":");
    if (string::npos != pos){
        ObjectStringSptr    prop_val;

        key = line.substr(0, pos);
        // remove spaces from key
        Utils::removeSpaces(key);
        transform(
            key.begin(),
            key.end(),
            key.begin(),
            ::tolower
        );
        val = line.substr(pos + 1);
        // remove first spaces from val
        Utils::removeSpacesFromStart(val);
        Utils::chomp(val);

        PROP_STRING(prop_val, val);
        m_req_headers->add(key, prop_val);

        if ("content-length" == key){
            m_req_content_length = atoi(val.c_str());
        } else if ("content-type" == key){
            parseContentType(val);
        } else if ("content-disposition" == key){
            contentDisposition(val);
        } else if ("cookie" == key){
            parseCookie(val);
        }
    }

out:
    return err;
}

int32_t ObjectHttpRequest::parseProtoLine(
    const string &a_line)
{
    int32_t             err = -1;
    string::size_type   pos;
    string              line;
    Splitter2           parts;

    if (!a_line.size()){
        err = 0;
        goto out;
    }

    line = a_line;
    Utils::chomp(line);
    parts = Splitter2(line, " ");

    if (1 <= parts.size()){
        m_req_method = parts[0];
    }
    if (2 <= parts.size()){
        m_req_uri = parts[1];
        parseReqApi();
    }
    if (3 <= parts.size()){
        m_req_proto = parts[2];
    }

    m_req_parse_state = PARSE_STATE_HEADERS;

out:
    return err;
}

void ObjectHttpRequest::chompContent(
    ObjectHttpRequest *a_req)
{
    Utils::chomp2(a_req->m_req_body);
}

void ObjectHttpRequest::chompLastBoundary()
{
    ObjectHttpRequestSptr req;

    if (    !m_req_boundaries
        ||  !m_req_boundaries->size())
    {
        goto out;
    }

    req = m_req_boundaries->at(
        m_req_boundaries->size() - 1
    );

    // remove last '\r' or '\r\n' from content
    chompContent(req.get());

out:
    return;
}

int32_t ObjectHttpRequest::parseBoundaryLine(
    const string &a_line)
{
    int32_t err = 0;

    if (!a_line.size()){
        goto out;
    }

    // maybe last of boundary
    if (    m_req_boundary_delim_end.size()
        &&  a_line.find(m_req_boundary_delim_end) != string::npos)
    {
        // remove '\r\n' or '\n' from last boundary
        chompLastBoundary();
        // end of boundary
        err = -1;
        goto out;
    }

    // start of delim, prepare boundary
    if (    m_req_boundary_delim.size()
        &&  a_line.find(m_req_boundary_delim) != string::npos)
    {
        if (!m_req_boundaries){
            m_req_boundaries = BoundariesSptr(new Boundaries);
        }

        // remove '\r\n' or '\n' from last boundary
        chompLastBoundary();

        // create new boundary
        ObjectHttpRequestSptr request;
        PROP_HTTP_REQUEST(request);
        request->m_req_parse_state    = PARSE_STATE_HEADERS;
        request->m_req_method         = m_req_method;
        request->m_req_proto          = m_req_proto;
        request->m_req_uri            = m_req_uri;
        request->m_req_boundary       = 1;
        m_req_boundaries->push_back(request);
        goto out;
    }

    // store to boundary
    if (    m_req_boundaries
        &&  m_req_boundaries->size())
    {
        m_req_boundaries->at(
            m_req_boundaries->size() - 1
        )->parseLine(
            a_line
        );
    } else {
        PERROR("Cannot parse HTTP line: '%s'\n",
            a_line.c_str()
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

int32_t ObjectHttpRequest::isReqBoundary()
{
    return m_req_boundary;
}

string ObjectHttpRequest::debugLine(
    const string &a_line)
{
    char                buffer[1024] = { 0x00 };
    string              line;
    string::size_type   pos;
    string              prefix, name;

    if (isReqBoundary()){
        prefix  = "    ";
        name    = "boundary";
    } else {
        prefix  = "";
        name    = "request";
    }

    for (pos = 0; pos < a_line.size(); pos++){
        char c = a_line.at(pos);
        if (    128 > c
            &&  31  < c)
        {
            line += c;
        } else if ('\r' == c){
            line += "\\r";
        } else if ('\n' == c){
            line += "\\n";
        } else {
            line += "<binary data>";
            break;
        }
    }
    Utils::chomp(line);
#ifdef __x86_64
    snprintf(buffer, sizeof(buffer),
        "%s[ %s 0x%lx ]"
        " state: '%d',"
        " line: '%s',"
        " line size: '%d'"
        " m_req_body size: '%d'",
        prefix.c_str(),
        name.c_str(),
        (uint64_t)this,
        m_req_parse_state,
        line.c_str(),
        (uint32_t)a_line.size(),
        (int32_t)m_req_body.size()
    );
#else
    snprintf(buffer, sizeof(buffer),
        "%s[ %s 0x%x ]"
        " state: '%d',"
        " line: '%s',"
        " line size: '%d'"
        " m_req_body size: '%d'",
        prefix.c_str(),
        name.c_str(),
        (uint32_t)this,
        m_req_parse_state,
        line.c_str(),
        (uint32_t)a_line.size(),
        (int32_t)m_req_body.size()
    );
#endif
    return buffer;
}

int32_t ObjectHttpRequest::parseLine(
    const string &a_line)
{
    int32_t err = -1;
    ssize_t req_content_size = 0;

    switch (m_req_parse_state){
        case PARSE_STATE_PROTO:
            err = parseProtoLine(a_line);
            break;

        case PARSE_STATE_HEADERS:
            err = parseHeaderLine(a_line);
            break;

        case PARSE_STATE_CONTENT:
            m_req_body.append(a_line);
            req_content_size = getReqContentSize();
            if (req_content_size){
                if (ssize_t(m_req_body.size()) >= req_content_size){
                    m_req_parse_state = PARSE_STATE_END;
                    err = 0;
                }
            } else if (!isReqBoundary()){
                // stop if We are not boundary and new line occur
                if (    "\r\n"  == a_line
                    ||  "\n"    == a_line)
                {
                    m_req_parse_state = PARSE_STATE_END;
                    // remove '\r\n' or '\n' from last boundary
                    chompContent(this);
                    err = 0;
                }
            }
            break;

        case PARSE_STATE_BOUNDARY:
            m_req_body.append(a_line);
            err = parseBoundaryLine(a_line);
            if (ssize_t(m_req_body.size()) >= getReqContentSize()){
                m_req_body.clear();
                err = 0;
            }
            break;

        case PARSE_STATE_END:
            err = 0;
            break;

        default:
            PERROR("Unknown state: '%d'\n", m_req_parse_state);
            break;
    };

    PDEBUG(100, "pase line: '%s', err: '%d'\n",
        debugLine(a_line).c_str(),
        err
    );

    return err;
};

BoundariesSptr ObjectHttpRequest::getReqBoundaries()
{
    return m_req_boundaries;
}

ObjectSptr  ObjectHttpRequest::getReqHeader(
    const string &a_name)
{
    ObjectSptr ret;
    ret = m_req_headers->get(a_name);
    return ret;
}

ssize_t ObjectHttpRequest::getReqContentSize()
{
    return m_req_content_length;
}

int32_t ObjectHttpRequest::processMReqBuff(
    const int32_t &a_force)
{
    string      line;
    uint32_t    i;
    int32_t     res, ret    = -1;
    int32_t     line_found  = 0;

    // process line by line
    for (i = 0; i < m_req_buff.size(); i++){
        char c = m_req_buff.at(i);
        line += c;
        if ('\n' == c){
            line_found = 1;
            m_req_buff = m_req_buff.substr(i + 1);
            res = parseLine(line);
            line.clear();
            if (!res){
                ret = res;
                break;
            }
            // start again
            i = -1;
        }
    }

    if (    a_force
        ||  (!line_found && PARSE_STATE_CONTENT == m_req_parse_state))
    {
        // process full buffer
        string tmp;
        tmp.assign(m_req_buff.c_str(), m_req_buff.size());
        m_req_buff = "";
        if (tmp.size()){
            ret = parseLine(tmp);
        }
    }

    return ret;
}

void ObjectHttpRequest::upgradeToWebSocket()
{
    ObjectSptr  prop_key;
    string      key;

    // get key
    prop_key = getReqHeader("sec-websocket-key");
    if (prop_key){
        key = prop_key->toString();
    }

    m_req_websock_accept = ObjectOpenSSL::sha1(
        key + WEBSOCK_STUPID_CONST
    );

    PWARN("upgrade to web socket:\n"
        "  key:                 '%s'\n"
        "  accept:              '%s'\n",
        key.c_str(),
        m_req_websock_accept.c_str()
    );
}

void ObjectHttpRequest::upgrade()
{
    ObjectSptr  prop_upgrade;
    string      upgrade;

    // get upgrade header
    prop_upgrade = getReqHeader("upgrade");
    if (!prop_upgrade){
        goto out;
    }
    upgrade = prop_upgrade->toString();

    if ("websocket" == upgrade){
        upgradeToWebSocket();
    } else {
        goto out;
    }

out:
    return;
}

int32_t ObjectHttpRequest::parseRequest(
    const string &a_data)
{
    int32_t         res, ret = -1;
    BoundariesIt    it;

    if (!a_data.size()){
        PWARN("Empty data in parseRequest\n");
        goto out;
    }

    m_req_buff += a_data;
    res = processMReqBuff(1);
    if (res){
        ret = res;
        goto out;
    }

    if (PARSE_STATE_BOUNDARY == m_req_parse_state){
        m_req_body = "";
    }

    PDEBUG(60, "was read:\n%s\n", dump().c_str());
    if (m_req_boundaries){
        PDEBUG(60, "was read: '%d' boundaries\n",
            int32_t(m_req_boundaries->size())
        );
        for (it = m_req_boundaries->begin();
            it != m_req_boundaries->end();
            it++)
        {
            PDEBUG(60, "boundary:\n%s\n",
                (*it)->dump().c_str()
            );
        }
    }

    upgrade();

    // all ok
    ret = 0;

out:
    return ret;
}

bool ObjectHttpRequest::isReqWebsock()
{
    return !m_req_websock_accept.empty();
}

string ObjectHttpRequest::getReqWebsockAccept()
{
    return m_req_websock_accept;
}

string ObjectHttpRequest::getReqUri()
{
    return m_req_uri;
}

void ObjectHttpRequest::getReqApi(
    ObjectMapSptr a_req_api)
{
    ObjectMapIt   it;

    for (it = m_req_api->begin();
        it != m_req_api->end();
        it++)
    {
        if (!a_req_api->has(it->first)){
            a_req_api->add(it->first, it->second);
            PDEBUG(20, "imported param from uri,"
                " param name: '%s' value: '%s'\n",
                it->first->toString()->c_str(),
                it->second->toString()->c_str()
            );
        }
    }
}

ObjectSptr ObjectHttpRequest::getReqCookie(
    const string &a_name)
{
    ObjectSptr          ret;
    ObjectMapIt         it;
    ObjectStringSptr    prop_name;

    PROP_STRING(prop_name, a_name);

    it = m_req_cookies->find(prop_name);
    if (m_req_cookies->end() != it){
        ret = it->second;
    }

    return ret;
}

void ObjectHttpRequest::parseCookie(
    const string &a_line)
{
    string::size_type   pos1, pos2 = string::npos;
    string              name;
    string              val;
    ObjectStringSptr    prop_name;
    ObjectStringSptr    prop_val;

    Splitter2   parts(a_line, "; ");
    for (pos1 = 0; pos1 < parts.size(); pos1++){
        Splitter2   key_val(parts[pos1], "= ");
        string      k, v;
        if (!key_val.size()){
            continue;
        }
        k = key_val[0];
        if (2 <= key_val.size()){
            v = key_val[1];
        }

        // skip symbols that started from '_'
        if ('_' != k.at(0)){
            name = k;
            val  = v;
            break;
        }
    }

    // remove spaces from start and end
    Utils::removeSpaces(name);

    // remove spaces from start
    Utils::removeSpacesFromStart(val);

    if (!name.size()){
        goto out;
    }

    PROP_STRING(prop_name, name);
    PROP_STRING(prop_val,  val);

    m_req_cookies->add(prop_name, prop_val);

out:
    return;
}

string ObjectHttpRequest::getReqBody()
{
    return m_req_body;
}

string ObjectHttpRequest::getReqProto()
{
    return m_req_proto;
}

string ObjectHttpRequest::getReqMethod()
{
    return m_req_method;
}

string ObjectHttpRequest::dump()
{
    string      dump;
    ObjectMapIt   headers_it;

    dump += "request:\n";
    dump += "  generic:\n";
    dump += "    method: '" + m_req_method + "'\n";
    dump += "    uri: '" + m_req_uri + "'\n";
    dump += "    proto: '" + m_req_proto + "'\n";

    dump += "  headers:\n";
    for (headers_it = m_req_headers->begin();
        headers_it != m_req_headers->end();
        headers_it++)
    {
        dump += "    ";
        dump += headers_it->first->toString();
        dump += " => ";
        dump += headers_it->second->toString();
        dump += "\n";
    }
    dump += "  body: '" + m_req_body + "'\n";

    return dump;
}

// ---------------- module ----------------

string ObjectHttpRequest::_getType()
{
    return "core.object.http.request";
}

string ObjectHttpRequest::_getTypePerl()
{
    return "core::object::httpRequest::main";
}

int32_t ObjectHttpRequest::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectHttpRequest::_destroy()
{
}

ObjectSptr ObjectHttpRequest::_object_create()
{
    ObjectSptr object;
    _OBJECT_HTTP_REQUEST(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectHttpRequest::_getType,
    .type_perl      = ObjectHttpRequest::_getTypePerl,
    .init           = ObjectHttpRequest::_init,
    .destroy        = ObjectHttpRequest::_destroy,
    .object_create  = ObjectHttpRequest::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

