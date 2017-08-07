#include <stdlib.h>
#include <algorithm>

#include <ipnoise-common/log_common.h>

#include <core/object/object/main.hpp>
#include <core/object/dir/main.hpp>
#include <core/object/int32/main.hpp>
#include <core/object/map/main.hpp>
#include <core/object/string/main.hpp>
#include <core/object/unique/main.hpp>
#include <core/object/vector/main.hpp>

#include "main.hpp"

ObjectHttpRequest::ObjectHttpRequest(
    const CreateFlags &a_create_flags)
    :   Object(a_create_flags)
{
}

ObjectHttpRequest::~ObjectHttpRequest()
{
}

string ObjectHttpRequest::getType()
{
    return ObjectHttpRequest::s_getType();
}

int32_t ObjectHttpRequest::do_init_as_prop(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = 0;

    PFATAL("object cannot be prop\n");

    res = Object::do_init_as_prop(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t ObjectHttpRequest::do_init_as_object(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t res, err = 0;

    res = Object::do_init_as_object(a_data, a_data_size);
    if (res){
        err = res;
        goto fail;
    }

    m_req_buffer         = OBJECT<ObjectString>(CREATE_PROP);
    m_req_is_boundary    = OBJECT<ObjectInt32>(0, CREATE_PROP);
    m_req_method         = OBJECT<ObjectString>(CREATE_PROP);
    m_req_uri            = OBJECT<ObjectString>(CREATE_PROP);
    m_req_path           = OBJECT<ObjectString>(CREATE_PROP);
    m_req_proto          = OBJECT<ObjectString>(CREATE_PROP);
    m_req_boundary_delim = OBJECT<ObjectString>(CREATE_PROP);
    m_req_boundary_delim_end = OBJECT<ObjectString>(CREATE_PROP);
    m_req_body           = OBJECT<ObjectString>(CREATE_PROP);
    m_req_content_length = OBJECT<ObjectInt32>(0, CREATE_PROP);

    m_req_boundaries     = OBJECT<ObjectVector>(CREATE_PROP);
    m_req_disposition    = OBJECT<ObjectMap>(CREATE_PROP);
    m_req_headers        = OBJECT<ObjectMap>(CREATE_PROP);
    m_req_cookies        = OBJECT<ObjectMap>(CREATE_PROP);
    m_req_api            = OBJECT<ObjectMap>(CREATE_PROP);
    m_req_parse_state    = OBJECT<ObjectInt32>(
        ObjectHttpRequest::PARSE_STATE_PROTO,
        CREATE_PROP
    );
    m_req_websock_accept = OBJECT<ObjectString>(CREATE_PROP);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

ObjectSptr ObjectHttpRequest::copy()
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectHttpRequestSptr   ret;

    ret = OBJECT<ObjectHttpRequest>();

    ret->parseRequest(
        m_req_buffer->c_str(),
        m_req_buffer->size()
    );

    return ret;
}

void ObjectHttpRequest::getAllProps(
    ObjectMapSptr   a_out)
{
    lock(a_out->m_rmutex, m_rmutex);
    lock_guard<recursive_mutex> guard1(a_out->m_rmutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(m_rmutex, adopt_lock);

    CreateFlags flags = getObjectFlags();

    if (!(flags & CREATE_PROP)){
        a_out->add("request_buffer",         m_req_buffer);
        a_out->add("request_is_boundary",    m_req_is_boundary);
        a_out->add("request_method",         m_req_method);
        a_out->add("request_uri",            m_req_uri);
        a_out->add("request_path",           m_req_path);
        a_out->add("request_proto",          m_req_proto);
        a_out->add("request_boundary_delim", m_req_boundary_delim);
        a_out->add("request_boundary_delim_end", m_req_boundary_delim_end);
        a_out->add("request_body",           m_req_body);
        a_out->add("request_content_length", m_req_content_length);
        a_out->add("request_boundaries",     m_req_boundaries);
        a_out->add("request_disposition",    m_req_disposition);
        a_out->add("request_headers",        m_req_headers);
        a_out->add("request_cookies",        m_req_cookies);
        a_out->add("request_api",            m_req_api);
        a_out->add("request_parse_state",    m_req_parse_state);
        a_out->add("request_websock_accept", m_req_websock_accept);

        Object::getAllProps(a_out);
    }
}

ObjectStringSptr ObjectHttpRequest::serializeAsProp(
    const char *a_delim)
{
    return m_req_buffer->serializeAsProp(a_delim);
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

    for (pos = 0; pos < m_req_uri->size(); pos++){
        const char c = m_req_uri->at(pos);
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
                    param_name = OBJECT<ObjectString>();
                }
                if (!param_value){
                    param_value = OBJECT<ObjectString>();
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
                    m_req_uri->c_str()
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
    const char *a_val)
{
    lock_guard<recursive_mutex> guard(m_rmutex);

    ObjectMapIt                 it;
    string::size_type           pos1, pos2;
    vector<string>              parts;
    vector<string>::iterator    parts_it;

    if (!a_val){
        PFATAL("missing argument 'a_val'\n");
    }

    ObjectString::s_split(
        a_val,
        "; ",
        parts,
        1               // strict
    );

    for (parts_it = parts.begin();
        parts_it != parts.end();
        parts_it++)
    {
        vector<string>              key_val;
        vector<string>::iterator    key_val_it;
        string                      k, v;

        ObjectString::s_split(
            parts_it->c_str(),
            "= ",
            key_val,
            0           // strict
        );

        if (key_val.empty()){
            continue;
        }
        k = key_val.at(0);
        if (2 <= key_val.size()){
            v = key_val.at(1);
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

ObjectStringSptr ObjectHttpRequest::getReqDispositionName()
{
    ObjectStringSptr    ret;

    ret = dynamic_pointer_cast<ObjectString>(
        m_req_disposition->get("name")
    );

    return ret;
}

ObjectStringSptr ObjectHttpRequest::getReqDispositionFileName()
{
    ObjectStringSptr    ret;

    ret = dynamic_pointer_cast<ObjectString>(
        m_req_disposition->get("filename")
    );

    return ret;
}

void ObjectHttpRequest::parseContentType(
    const char *a_val)
{
    string::size_type           i, pos;
    vector<string>              parts;
    vector<string>::iterator    parts_it;
    int32_t                     multipart_form_data = 0;
    ObjectStringSptr            boundary;

    if (!a_val){
        PFATAL("missing argument 'a_val'\n");
    }

    ObjectString::s_split(
        a_val,
        "; ",
        parts,
        0       // strict
    );

    for (parts_it = parts.begin();
        parts_it != parts.end();
        parts_it++)
    {
        vector<string>              key_val;
        vector<string>::iterator    key_val_it;
        string                      key, val;

        ObjectString::s_split(
            parts_it->c_str(),
            " =",
            key_val,
            0           // strict
        );
        if (key_val.empty()){
            continue;
        }
        key = key_val.at(0);
        if (1 < key_val.size()){
            val = key_val.at(1);
        }

        if ("multipart/form-data" == key){
            multipart_form_data = 1;
        }

        if ("boundary" == key){
            boundary = OBJECT<ObjectString>(val);
        }
    }
    if (    multipart_form_data
        &&  boundary)
    {
        m_req_boundary_delim        = boundary;
        m_req_boundary_delim_end    = OBJECT<ObjectString>();
        m_req_boundary_delim_end->add(m_req_boundary_delim);
        m_req_boundary_delim_end->add("--");
    }
}

ObjectSptr ObjectHttpRequest::parseQ(
    const char *a_val)
{
    ObjectVectorSptr  ret;

    // parse string like:
    // en-US,en;q=0.8,ru;q=0.6,es;q=0.4,de;
    vector<string>              parts;
    vector<string>::iterator    parts_it;

    ret = OBJECT<ObjectVector>();
    ObjectString::s_split(
        a_val,
        ";",
        parts,
        0       // strict
    );
    for (parts_it = parts.begin();
        parts_it != parts.end();
        parts_it++)
    {
        vector<string>              params;
        vector<string>::iterator    params_it;
        ObjectUniqueSptr            part;

        part = OBJECT<ObjectUnique>();

        ObjectString::s_split(
            parts_it->c_str(),
            ",",
            params,
            0       // strict
        );
        for (params_it = params.begin();
            params_it != params.end();
            params_it++)
        {
            part->add(*params_it);
        }

        ret->add(part);
    }

    return ret;
}

int32_t ObjectHttpRequest::parseHeaderLine(
    const char *a_line)
{
    string              key;
    string              val;
    string::size_type   pos;
    string              line;
    int32_t             err = -1;

    if (!a_line){
        PFATAL("missing argument 'a_line'\n");
    }

    if (!strlen(a_line)){
        err = 0;
        goto out;
    }

    if (    !strcmp("\r\n", a_line)
        ||  !strcmp("\n", a_line))
    {
        if (strcmp("POST", getReqMethod()->c_str())){
            // end of request
            err = 0;
            goto out;
        }
        if (!m_req_boundary_delim->empty()){
            m_req_parse_state = OBJECT<ObjectInt32>(
                PARSE_STATE_BOUNDARY
            );
        } else {
            m_req_parse_state = OBJECT<ObjectInt32>(
                PARSE_STATE_CONTENT
            );
        }
        goto out;
    }

    line = a_line;
    ObjectString::s_chomp(line);
    pos = line.find(":");
    if (string::npos != pos){
        ObjectSptr    prop_val;

        key = line.substr(0, pos);
        // remove spaces from key
        ObjectString::s_removeSpaces(key);
        transform(
            key.begin(),
            key.end(),
            key.begin(),
            ::tolower
        );
        val = line.substr(pos + 1);
        // remove first spaces from val
        ObjectString::s_removeSpacesFromStart(val);
        ObjectString::s_chomp(val);

        // parse 'val' if need
        if ("accept-language" == key){
            prop_val = parseQ(val.c_str());
        } else if ("accept" == key){
            prop_val = parseQ(val.c_str());
        } else {
            prop_val = OBJECT<ObjectString>(val);
        }
        m_req_headers->add(key, prop_val);

        if ("content-length" == key){
            m_req_content_length = OBJECT<ObjectInt32>(
                atoi(val.c_str())
            );
        } else if ("content-type" == key){
            parseContentType(val.c_str());
        } else if ("content-disposition" == key){
            contentDisposition(val.c_str());
        } else if ("cookie" == key){
            parseCookie(val.c_str());
        }
    }

out:
    return err;
}

int32_t ObjectHttpRequest::parseProtoLine(
    const char *a_line)
{
    int32_t                     err = -1;
    string::size_type           pos;
    string                      line;
    vector<string>              parts;
    vector<string>::iterator    parts_it;

    if (!a_line){
        PFATAL("missing argument 'a_line'\n");
    }

    if (!strlen(a_line)){
        err = 0;
        goto out;
    }

    line = a_line;
    ObjectString::s_chomp(line);

    ObjectString::s_split(
        line.c_str(),
        " ",
        parts,
        0       // strict
    );

    if (1 <= parts.size()){
        m_req_method = OBJECT<ObjectString>(
            parts.at(0)
        );
    }
    if (2 <= parts.size()){
        m_req_uri = OBJECT<ObjectString>(
            parts.at(1)
        );
        parseReqApi();
    }
    if (3 <= parts.size()){
        m_req_proto = OBJECT<ObjectString>(
            parts.at(2)
        );
    }

    m_req_parse_state = OBJECT<ObjectInt32>(
        PARSE_STATE_HEADERS
    );

out:
    return err;
}

void ObjectHttpRequest::chompContent(
    ObjectHttpRequest *a_req)
{
    string tmp = a_req->m_req_body->toStringStd();
    ObjectString::s_chomp2(tmp);
    a_req->m_req_body = OBJECT<ObjectString>(tmp);
}

void ObjectHttpRequest::chompLastBoundary()
{
    ObjectHttpRequestSptr req;

    if (    !m_req_boundaries
        ||  !m_req_boundaries->size())
    {
        goto out;
    }

    req = dynamic_pointer_cast<ObjectHttpRequest>(
        m_req_boundaries->at(
            m_req_boundaries->size() - 1
        )
    );

    // remove last '\r' or '\r\n' from content
    chompContent(req.get());

out:
    return;
}

int32_t ObjectHttpRequest::parseBoundaryLine(
    const char *a_line)
{
    int32_t err = 0;

    if (!a_line){
        PFATAL("missing argument 'a_line'\n");
    }

    if (!strlen(a_line)){
        err = 0;
        goto out;
    }

    // maybe last of boundary
    if (    !m_req_boundary_delim_end->empty()
        &&  strstr(a_line, m_req_boundary_delim_end->c_str()))
    {
        // remove '\r\n' or '\n' from last boundary
        chompLastBoundary();
        // end of boundary
        err = -1;
        goto out;
    }

    // start of delim, prepare boundary
    if (    !m_req_boundary_delim->empty()
        &&  strstr(a_line, m_req_boundary_delim->c_str()))
    {
        if (!m_req_boundaries){
            m_req_boundaries = OBJECT<ObjectVector>();
        }

        // remove '\r\n' or '\n' from last boundary
        chompLastBoundary();

        // create new boundary
        ObjectHttpRequestSptr request;

        request = OBJECT<ObjectHttpRequest>();

        request->m_req_parse_state    = OBJECT<ObjectInt32>(
            PARSE_STATE_HEADERS
        );
        request->m_req_method         = m_req_method;
        request->m_req_proto          = m_req_proto;
        request->m_req_uri            = m_req_uri;
        request->m_req_is_boundary    = OBJECT<ObjectInt32>(1);
        m_req_boundaries->push_back(request);
        goto out;
    }

    // store to boundary
    if (    m_req_boundaries
        &&  m_req_boundaries->size())
    {
        ObjectHttpRequestSptr req;
        req = dynamic_pointer_cast<ObjectHttpRequest>(
            m_req_boundaries->at(
                m_req_boundaries->size() - 1
            )
        );
        req->parseLine(a_line);
    } else {
        PERROR("Cannot parse HTTP line: '%s'\n",
            a_line
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

ObjectInt32Sptr ObjectHttpRequest::isReqBoundary()
{
    return m_req_is_boundary;
}

string ObjectHttpRequest::debugLine(
    const char *a_line)
{
    char                buffer[1024] = { 0x00 };
    string              line;
    string::size_type   pos = 0;
    string              prefix, name;

    if (!a_line){
        PFATAL("missing argument 'a_line'\n");
    }

    if (isReqBoundary()){
        prefix  = "    ";
        name    = "boundary";
    } else {
        prefix  = "";
        name    = "request";
    }

    do {
        char c = a_line[pos++];
        if ('\0' == c){
            break;
        }
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
    } while (1);

    ObjectString::s_chomp(line);
#ifdef __x86_64__
    snprintf(buffer, sizeof(buffer),
        "%s[ %s 0x%lx ]"
        " state: '%d',"
        " line: '%s',"
        " line size: '%d'"
        " m_req_body size: '%ld'",
        prefix.c_str(),
        name.c_str(),
        (uint64_t)this,
        m_req_parse_state->getVal(),
        line.c_str(),
        (uint32_t)strlen(a_line),
        (int64_t)m_req_body->size()
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
        m_req_parse_state->getVal(),
        line.c_str(),
        (uint32_t)strlen(a_line),
        (int32_t)m_req_body->size()
    );
#endif
    return buffer;
}

//
// return 0 if EOF
//
int32_t ObjectHttpRequest::parseLine(
    const char *a_line)
{
    int32_t err = -1;
    ssize_t req_content_size = 0;

    if (!a_line){
        PFATAL("missing argument 'a_line'\n");
    }

    switch (m_req_parse_state->getVal()){
        case PARSE_STATE_PROTO:
            err = parseProtoLine(a_line);
            break;

        case PARSE_STATE_HEADERS:
            err = parseHeaderLine(a_line);
            break;

        case PARSE_STATE_CONTENT:
            m_req_body->add(a_line);
            req_content_size = getReqContentSize()->getVal();
            if (req_content_size){
                if (ssize_t(m_req_body->size()) >= req_content_size){
                    m_req_parse_state = OBJECT<ObjectInt32>(
                        PARSE_STATE_END
                    );
                    err = 0;
                }
            } else if (!isReqBoundary()){
                // stop if We are not boundary and new line occur
                if (    !strcmp("\r\n", a_line)
                    ||  !strcmp("\n", a_line))
                {
                    m_req_parse_state = OBJECT<ObjectInt32>(
                        PARSE_STATE_END
                    );
                    // remove '\r\n' or '\n' from last boundary
                    chompContent(this);
                    err = 0;
                }
            }
            break;

        case PARSE_STATE_BOUNDARY:
            m_req_body->add(a_line);
            err = parseBoundaryLine(a_line);
            if (ssize_t(m_req_body->size()) >= getReqContentSize()->getVal()){
                m_req_body->clear();
                err = 0;
            }
            break;

        case PARSE_STATE_END:
            err = 0;
            break;

        default:
            PERROR("Unknown state: '%d'\n",
                m_req_parse_state->getVal()
            );
            break;
    };

    //PDEBUG(100, "pase line: '%s', err: '%d'\n",
    //    debugLine(a_line).c_str(),
    //    err
    //);

    return err;
};

ObjectVectorSptr ObjectHttpRequest::getReqBoundaries()
{
    return m_req_boundaries;
}

ObjectStringSptr  ObjectHttpRequest::getReqHeader(
    const char *a_name)
{
    ObjectStringSptr ret;

    if (!a_name){
        PFATAL("missing argument 'a_name'\n");
    }

    ret = dynamic_pointer_cast<ObjectString>(
        m_req_headers->get(a_name)
    );

    return ret;
}

ObjectInt32Sptr ObjectHttpRequest::getReqContentSize()
{
    return m_req_content_length;
}

int32_t ObjectHttpRequest::process_buffer(
    const int32_t &a_force)
{
    string      line;
    uint64_t    i;
    int32_t     res, ret    = -1;
    int32_t     line_found  = 0;

    // process line by line
    for (i = 0; i < m_req_buffer->size(); i++){
        char c = m_req_buffer->at(i);
        line += c;
        if ('\n' == c){
            line_found   = 1;
            m_req_buffer = m_req_buffer->substr(i + 1);
            res          = parseLine(line.c_str());
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
        ||  (!line_found
            && PARSE_STATE_CONTENT == m_req_parse_state->getVal()))
    {
        // process full buffer
        string tmp;
        tmp.assign(
            m_req_buffer->c_str(),
            m_req_buffer->size()
        );
        m_req_buffer->clear();
        if (tmp.size()){
            ret = parseLine(tmp.c_str());
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
        key = prop_key->toStringStd();
    }

/*
    TODO XXX FIXME
    m_req_websock_accept = ObjectOpenSSL::sha1(
        key + WEBSOCK_STUPID_CONST
    );
*/

    PWARN("upgrade to web socket:\n"
        "  key:                 '%s'\n"
        "  accept:              '%s'\n",
        key.c_str(),
        m_req_websock_accept->c_str()
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
    upgrade = prop_upgrade->toStringStd();

    if ("websocket" == upgrade){
        upgradeToWebSocket();
    } else {
        goto out;
    }

out:
    return;
}

int32_t ObjectHttpRequest::parseRequest(
    const char      *a_data,
    const uint64_t  &a_data_size)
{
    int32_t         res, ret = -1;
    ObjectVectorIt  it;

    if (!a_data){
        PFATAL("missing argument 'a_data'\n");
    }

    if (!a_data_size){
        PWARN("empty data in parseRequest\n");
        goto out;
    }

    m_req_buffer->add(a_data, a_data_size);

    res = process_buffer(1);
    if (res){
        ret = res;
        goto out;
    }

    if (PARSE_STATE_BOUNDARY == m_req_parse_state->getVal()){
        m_req_body->clear();
    }

    PDEBUG(60, "was read:\n%s\n", dump()->c_str());
    if (!m_req_boundaries->empty()){
        PDEBUG(60, "was read: '%d' boundaries\n",
            int32_t(m_req_boundaries->size())
        );
        for (it = m_req_boundaries->begin();
            it != m_req_boundaries->end();
            it++)
        {
            ObjectHttpRequestSptr req;
            req = dynamic_pointer_cast<ObjectHttpRequest>(*it);

            PDEBUG(60, "boundary:\n%s\n",
                req->dump()->c_str()
            );
        }
    }

    upgrade();

    // all ok
    ret = 0;

out:
    return ret;
}

ObjectInt32Sptr ObjectHttpRequest::isReqWebsock()
{
    ObjectInt32Sptr ret;

    ret = OBJECT<ObjectInt32>(
        m_req_websock_accept->empty() ? 0 : 1
    );

    return ret;
}

ObjectStringSptr ObjectHttpRequest::getReqWebsockAccept()
{
    return m_req_websock_accept;
}

ObjectStringSptr ObjectHttpRequest::getReqUri()
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

ObjectStringSptr ObjectHttpRequest::getReqCookie(
    const char *a_name)
{
    ObjectStringSptr    ret;
    ObjectMapIt         it;

    if (!a_name){
        PFATAL("missing argument 'a_name'\n");
    }

    it = m_req_cookies->find(a_name);
    if (m_req_cookies->end() != it){
        ret = dynamic_pointer_cast<ObjectString>(
            it->second
        );
    }

    return ret;
}

void ObjectHttpRequest::parseCookie(
    const char *a_line)
{
    string::size_type           pos1, pos2 = string::npos;
    string                      name;
    string                      val;
    ObjectStringSptr            prop_name;
    ObjectStringSptr            prop_val;
    vector<string>              parts;
    vector<string>::iterator    parts_it;

    if (!a_line){
        PFATAL("missing argument 'a_line'\n");
    }

    ObjectString::s_split(
        a_line,
        "; ",
        parts,
        0       // strict
    );

    for (parts_it = parts.begin();
        parts_it != parts.end();
        parts_it++)
    {
        vector<string>              key_val;
        vector<string>::iterator    key_val_it;
        string                      k, v;

        ObjectString::s_split(
            parts_it->c_str(),
            "= ",
            key_val,
            0       // strict
        );

        if (key_val.empty()){
            continue;
        }

        k = key_val.at(0);
        if (2 <= key_val.size()){
            v = key_val.at(1);
        }

        // skip symbols that started from '_'
        if ('_' != k.at(0)){
            name = k;
            val  = v;
            break;
        }
    }

    // remove spaces from start and end
    ObjectString::s_removeSpaces(name);

    // remove spaces from start
    ObjectString::s_removeSpacesFromStart(val);

    if (!name.size()){
        goto out;
    }

    prop_name = OBJECT<ObjectString>(name);
    prop_val  = OBJECT<ObjectString>(val);

    m_req_cookies->add(prop_name, prop_val);

out:
    return;
}

ObjectStringSptr ObjectHttpRequest::getReqBody()
{
    return m_req_body;
}

ObjectStringSptr ObjectHttpRequest::getReqProto()
{
    return m_req_proto;
}

ObjectStringSptr ObjectHttpRequest::getReqMethod()
{
    return m_req_method;
}

ObjectStringSptr ObjectHttpRequest::dump()
{
    ObjectStringSptr    dump;
    ObjectMapIt         headers_it;

    dump = OBJECT<ObjectString>();

    dump->add("request:\n");
    dump->add("  generic:\n");
    dump->add("    method: '" + m_req_method->toStringStd() + "'\n");
    dump->add("    uri: '"    + m_req_uri->toStringStd()    + "'\n");
    dump->add("    proto: '"  + m_req_proto->toStringStd()  + "'\n");

    dump->add("  headers:\n");
    for (headers_it = m_req_headers->begin();
        headers_it != m_req_headers->end();
        headers_it++)
    {
        dump->add("    ");
        dump->add(headers_it->first->toString());
        dump->add(" => ");
        dump->add(headers_it->second->toString());
        dump->add("\n");
    }
    dump->add("  body: '" + m_req_body->toStringStd() + "'\n");

    return dump;
}


// ---------------- static ----------------

// ---------------- module ----------------

string ObjectHttpRequest::s_getType()
{
    return "core.object.http.request";
}

int32_t ObjectHttpRequest::s_init(
    EngineInfo  *a_engine_info)
{
    g_engine_info = a_engine_info;
    return 0;
}

int32_t ObjectHttpRequest::s_shutdown()
{
    return 0;
}

ObjectSptr ObjectHttpRequest::s_objectCreate(
    const CreateFlags   &a_create_flags)
{
    ObjectSptr  object;

    object = ObjectSptr(new ObjectHttpRequest(a_create_flags));

    return object;
}

void ObjectHttpRequest::s_getTests(
    Tests &a_out)
{
}

ModuleInfo core_object_http_request = {
    .getType        = ObjectHttpRequest::s_getType,
    .init           = ObjectHttpRequest::s_init,
    .shutdown       = ObjectHttpRequest::s_shutdown,
    .objectCreate   = ObjectHttpRequest::s_objectCreate,
    .getTests       = ObjectHttpRequest::s_getTests
};

