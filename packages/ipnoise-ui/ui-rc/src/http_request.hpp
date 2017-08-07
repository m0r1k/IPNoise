#include "class.hpp"
DEFINE_CLASS(HttpRequest);

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <stdint.h>

#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include <QString>
#include <QStringList>
#include <QTcpSocket>

#include "log.hpp"
#include "prop.hpp"
#include "prop/map.hpp"
#include "prop/vector.hpp"
#include "prop/string.hpp"

using namespace std;

typedef vector <HttpRequestSptr>        Boundaries;
typedef Boundaries::iterator            BoundariesIt;
typedef Boundaries::const_iterator      BoundariesConstIt;
typedef shared_ptr < Boundaries >       BoundariesSptr;

class HttpRequest
{
    public:
        HttpRequest();
        HttpRequest(QTcpSocket *);
        virtual ~HttpRequest();

        string          getReqUri()                     const;
        string          getReqBody()                    const;
        string          getReqMethod()                  const;
        string          getReqProto()                   const;
        void            getReqApi(PropMapSptr)          const;
        PropSptr        getReqCookie(const string &)    const;
        ssize_t         getReqContentSize()             const;
        BoundariesSptr  getReqBoundaries()              const;
        PropSptr        getReqHeader(const string &);
        string          getReqDispositionName();
        string          getReqDispositionFileName();
        int32_t         isReqBoundary() const;
        string          dump();
        int32_t         parseRequest(QByteArray &);

    protected:
        enum UriParseState {
            URI_PARSE_STATE_ERROR = 0,
            URI_PARSE_STATE_PATH,
            URI_PARSE_STATE_PARAMS,
            URI_PARSE_STATE_PARAM_NAME,
            URI_PARSE_STATE_PARAM_VALUE
        };

        enum ParseState {
            PARSE_STATE_ERROR = 0,
            PARSE_STATE_PROTO,
            PARSE_STATE_HEADERS,
            PARSE_STATE_CONTENT,
            PARSE_STATE_BOUNDARY,
            PARSE_STATE_END
        };

        void        do_init();
        void        processApi(
            PropStringSptr  a_param_name,
            PropStringSptr  a_param_value
        );
        int32_t     parseProtoLine(const string &);
        int32_t     parseHeaderLine(const string &);
        int32_t     parseBoundaryLine(const string &);
        int32_t     parseLine(const string &);
        string      debugLine(const string &);

        void        parseReqApi();
        void        parseCookie(const string &);
        void        contentDisposition(const string &);
        void        chompContent(HttpRequest *);
        void        chompLastBoundary();
        int32_t     processMReqBuff(int32_t a_force = 0);

        void        parseContentType(const string &);

    private:
        QByteArray      m_req_buff;
        int32_t         m_req_boundary;

        string          m_req_method;
        string          m_req_uri;
        string          m_req_path;
        string          m_req_proto;
        string          m_req_boundary_delim;
        string          m_req_boundary_delim_end;
        string          m_req_body;
        ssize_t         m_req_content_length;
        BoundariesSptr  m_req_boundaries;
        PropMap         m_req_disposition;
        PropMap         m_req_headers;
        PropMapSptr     m_req_cookies;
        PropMapSptr     m_req_api;
        ParseState      m_req_parse_state;
};

#endif

