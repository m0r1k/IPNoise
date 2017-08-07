#include "strings.hpp"
#include "utils.hpp"

#include "http_request.hpp"

HttpRequest::HttpRequest()
{
    do_init();
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::do_init()
{
    m_req_cookies           = PROP_MAP();
    m_req_api               = PROP_MAP();
    m_req_content_length    = 0;
    m_req_parse_state       = HttpRequest::PARSE_STATE_PROTO;
    m_req_boundary          = 0;
}

void HttpRequest::processApi(
    PropStringSptr  a_param_name,
    PropStringSptr  a_param_value)
{
    if (!a_param_name){
        PFATAL("emty param: 'a_param_name'\n");
    }
    if (!a_param_value){
        PFATAL("emty param: 'a_param_value'\n");
    }

    PWARN("MORIK HttpRequest::processApi:\n"
        " a_param_name:     '%s'\n"
        " a_param_value:    '%s'\n",
        a_param_name->toString().c_str(),
        a_param_value->toString().c_str()
    );

    m_req_api->add(a_param_name, a_param_value);

    a_param_name  = PropStringSptr();
    a_param_value = PropStringSptr();
}

void HttpRequest::parseReqApi()
{
    string::size_type   pos   = string::npos;
    UriParseState       state = URI_PARSE_STATE_PATH;

    string          path;
    PropStringSptr  param_name;
    PropStringSptr  param_value;

    for (pos = 0; pos < m_req_uri.size(); pos++){
        const char c = m_req_uri.at(pos);
        switch (state){
            case URI_PARSE_STATE_PARAM_VALUE:
                if ('&' == c){
                    processApi(param_name, param_value);
                    state       = URI_PARSE_STATE_PARAM_NAME;
                    param_name  = PropStringSptr();
                    param_value = PropStringSptr();
                    break;
                } else {
                    param_value->add(c);
                }
                break;

            case URI_PARSE_STATE_PARAM_NAME:
                if (!param_name){
                    param_name = PROP_STRING();
                }
                if (!param_value){
                    param_value = PROP_STRING();
                }
                if ('=' == c){
                    state = URI_PARSE_STATE_PARAM_VALUE;
                    break;
                } else if ('&' == c){
                    processApi(param_name, param_value);
                    state       = URI_PARSE_STATE_PARAM_NAME;
                    param_name  = PropStringSptr();
                    param_value = PropStringSptr();
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
        param_name  = PropStringSptr();
        param_value = PropStringSptr();
    }
}

void HttpRequest::contentDisposition(
    const string &a_val)
{
    PropMapIt           it;
    string::size_type   pos1, pos2;
    Splitter2           parts(a_val, "; ");

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
        m_req_disposition.add(k, v);
    }

    for (it = m_req_disposition.begin();
        it != m_req_disposition.end();
        it++)
    {
        PDEBUG(20, "disposition: '%s' => '%s'\n",
            it->first->toString().c_str(),
            it->second->toString().c_str()
        );
    }
}

string HttpRequest::getReqDispositionName()
{
    string ret;
    if (m_req_disposition.has("name")){
        ret = m_req_disposition["name"]->toString();
    }
    return ret;
}

string HttpRequest::getReqDispositionFileName()
{
    string ret;
    if (m_req_disposition.has("filename")){
        ret = m_req_disposition["filename"]->toString();
    }
    return ret;
}

void HttpRequest::parseContentType(
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

int32_t HttpRequest::parseHeaderLine(
    const string &a_line)
{
    string              key;
    string              val;
    string::size_type   pos;
    string              line;
    int32_t             err = 0;

    if (!a_line.size()){
        goto out;
    }

    if (    "\r\n"  == a_line
        ||  "\n"    == a_line)
    {
        if ("POST" != getReqMethod()){
            // end of request
            err = -1;
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
        m_req_headers[key] = PROP_STRING(val);

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

    // all ok
    err = 0;

out:
    return err;
}

int32_t HttpRequest::parseProtoLine(
    const string &a_line)
{
    int32_t             err = 0;
    string::size_type   pos;
    string              line;
    Splitter2           parts;

    if (!a_line.size()){
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

void HttpRequest::chompContent(
    HttpRequest *a_req)
{
    Utils::chomp2(a_req->m_req_body);
}

void HttpRequest::chompLastBoundary()
{
    HttpRequestSptr req;

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

int32_t HttpRequest::parseBoundaryLine(
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
        HttpRequestSptr request(new HttpRequest);
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

int32_t HttpRequest::isReqBoundary() const
{
    return m_req_boundary;
}

string HttpRequest::debugLine(
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

int32_t HttpRequest::parseLine(
    const string &a_line)
{
    int32_t err = 0;

    PDEBUG(25, "%s\n", debugLine(a_line).c_str());

    switch (m_req_parse_state){
        case PARSE_STATE_PROTO:
            err = parseProtoLine(a_line);
            break;

        case PARSE_STATE_HEADERS:
            err = parseHeaderLine(a_line);
            break;

        case PARSE_STATE_CONTENT:
            m_req_body.append(a_line);
            if (getReqContentSize()){
                if (ssize_t(m_req_body.size()) >= getReqContentSize()){
                    m_req_parse_state = PARSE_STATE_END;
                    err = -1;
                }
            } else if (!isReqBoundary()){
                // stop if We are not boundary and new line occur
                if ("\r\n" == a_line || "\n" == a_line){
                    m_req_parse_state = PARSE_STATE_END;
                    // remove '\r\n' or '\n' from last boundary
                    chompContent(this);
                    err = -1;
                }
            }
            break;

        case PARSE_STATE_BOUNDARY:
            m_req_body.append(a_line);
            err = parseBoundaryLine(a_line);
            if (ssize_t(m_req_body.size()) >= getReqContentSize()){
                m_req_body.clear();
                err = -1;
            }
            break;

        case PARSE_STATE_END:
            err = -1;
            break;

        default:
            PERROR("Unknown state: '%d'\n", m_req_parse_state);
            break;
    };

    return err;
};

BoundariesSptr HttpRequest::getReqBoundaries() const
{
    return m_req_boundaries;
}

PropSptr  HttpRequest::getReqHeader(
    const string &a_name)
{
    PropSptr ret;
    ret = m_req_headers[a_name];
    return ret;
}

ssize_t HttpRequest::getReqContentSize() const
{
    return m_req_content_length;
}

int32_t HttpRequest::processMReqBuff(
    int32_t a_force)
{
    QByteArray  line;
    int32_t     i, err = 0;
    int32_t     line_found = 0;

    // process line by line
    for (i = 0; i < m_req_buff.size(); i++){
        char c = m_req_buff.at(i);
        line += c;
        if ('\n' == c){
            line_found = 1;
            m_req_buff.remove(0, i + 1);
            string tmp;
            tmp.assign(line.data(), line.size());
            line.clear();
            err = parseLine(tmp);
            if (err){
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
        tmp.assign(m_req_buff.data(), m_req_buff.size());
        m_req_buff.clear();
        if (tmp.size()){
            err = parseLine(tmp);
        }
    }

    return err;
}

int32_t HttpRequest::parseRequest(
    QByteArray  &a_data)
{
    int32_t         res, ret = -1;
    BoundariesIt    it;

    if (!a_data.size()){
        PWARN("Empty data in parseRequest\n");
        goto out;
    }

    m_req_buff += a_data;
    res = processMReqBuff(1);
    if (!res){
        ret = 0;
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

out:
    return ret;
}

string HttpRequest::getReqUri() const
{
    return m_req_uri;
}

void HttpRequest::getReqApi(
    PropMapSptr a_req_api) const
{
    PropMapConstIt  it;

    for (it = m_req_api->begin();
        it != m_req_api->end();
        it++)
    {
        if (!a_req_api->has(it->first)){
            a_req_api->add(it->first, it->second);
            PDEBUG(20, "imported param from uri,"
                " param name: '%s' value: '%s'\n",
                it->first->toString().c_str(),
                it->second->toString().c_str()
            );
        }
    }
}

PropSptr HttpRequest::getReqCookie(
    const string &a_name)   const
{
    PropSptr        ret;
    PropMapConstIt  it;

    it = m_req_cookies->find(
        PROP_STRING(a_name)
    );
    if (m_req_cookies->end() != it){
        ret = it->second;
    }

    return ret;
}

void HttpRequest::parseCookie(
    const string &a_line)
{
    string::size_type   pos1, pos2 = string::npos;
    string              name;
    string              val;

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

    (*(m_req_cookies.get()))[PROP_STRING(name)]
        = PROP_STRING(val);

out:
    return;
}

string HttpRequest::getReqBody() const
{
    return m_req_body;
}

string HttpRequest::getReqProto() const
{
    return m_req_proto;
}

string HttpRequest::getReqMethod() const
{
    return m_req_method;
}

string HttpRequest::dump()
{
    string          dump;
    PropMapConstIt  headers_it;

    dump += "request:\n";
    dump += "  generic:\n";
    dump += "    method: '" + m_req_method + "'\n";
    dump += "    uri: '" + m_req_uri + "'\n";
    dump += "    proto: '" + m_req_proto + "'\n";

    dump += "  headers:\n";
    for (headers_it = m_req_headers.begin();
        headers_it != m_req_headers.end();
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

