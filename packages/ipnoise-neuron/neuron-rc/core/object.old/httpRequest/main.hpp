#include "class.hpp"
DEFINE_CLASS(ObjectHttpRequest);

#ifndef OBJECT_HTTP_REQUEST_HPP
#define OBJECT_HTTP_REQUEST_HPP

#include <stdint.h>

#include <string>
#include <map>
#include <vector>
//#include <algorithm>

#include <mongo/client/dbclient.h>
#include <ipnoise-common/log_common.h>
#include "utils.hpp"

extern "C"
{
    #include <EXTERN.h>
    #include <perl.h>
    #include <XSUB.h>
};

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectAction);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectVector);
DEFINE_CLASS(ObjectMap);

#define WEBSOCK_STUPID_CONST "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

using namespace std;

typedef vector <ObjectHttpRequestSptr>  Boundaries;
typedef Boundaries::iterator            BoundariesIt;
typedef Boundaries::const_iterator      BoundariesConstIt;
typedef shared_ptr < Boundaries >       BoundariesSptr;

#define _OBJECT_HTTP_REQUEST(a_out, a_flags, ...)           \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectHttpRequest,                                  \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_HTTP_REQUEST(a_out, a_flags, ...)             \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectHttpRequest,                                  \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_HTTP_REQUEST(a_out, ...)                     \
    CREATE_OBJECT(a_out, ObjectHttpRequest, ##__VA_ARGS__)

#define PROP_HTTP_REQUEST(a_out, ...)                       \
    CREATE_PROP(a_out, ObjectHttpRequest, ##__VA_ARGS__)

class ObjectHttpRequest
    :   public Object
{
    public:
        ObjectHttpRequest(const Object::CreateFlags &);
        ObjectHttpRequest(
            const Object::CreateFlags &,
            ObjectHttpRequestSptr
        );
        virtual ~ObjectHttpRequest();

//        DEFINE_PROP(ReqDisposition, ObjectMapSptr);

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
//        virtual void    getAllProps(
//            ObjectMapSptr a_props
//        );
        virtual string  serialize(
            const string &a_delim = DEFAULT_DELIMITER
        );

        // generic
        string          getReqUri();
        string          getReqBody();
        string          getReqMethod();
        string          getReqProto();
        void            getReqApi(ObjectMapSptr);
        ObjectSptr      getReqCookie(const string &);
        ssize_t         getReqContentSize();
        BoundariesSptr  getReqBoundaries();
        ObjectSptr      getReqHeader(const string &);
        string          getReqDispositionName();
        string          getReqDispositionFileName();
        int32_t         isReqBoundary();
        string          dump();
        int32_t         parseRequest(const string &);
        bool            isReqWebsock();
        string          getReqWebsockAccept();

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

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

        void        processApi(
            ObjectStringSptr  a_param_name,
            ObjectStringSptr  a_param_value
        );
        int32_t     parseProtoLine(const string &);
        int32_t     parseHeaderLine(const string &);
        int32_t     parseBoundaryLine(const string &);
        int32_t     parseLine(const string &);
        string      debugLine(const string &);
        void        parseBSON();

        void        parseReqApi();
        void        parseCookie(const string &);
        void        contentDisposition(const string &);
        void        chompContent(ObjectHttpRequest *);
        void        chompLastBoundary();
        int32_t     processMReqBuff(
            const int32_t &a_force = 0
        );
        void        upgradeToWebSocket();
        void        upgrade();
        void        parseContentType(const string &);

    private:
        // don't forget update "copy" constructor
        // ObjectHttpRequest::ObjectHttpRequest
        string          m_req_buff;
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
        ObjectMapSptr   m_req_disposition;
        ObjectMapSptr   m_req_headers;
        ObjectMapSptr   m_req_cookies;
        ObjectMapSptr   m_req_api;
        ParseState      m_req_parse_state;
        string          m_req_websock_accept;

        void        do_init(const Object::CreateFlags &);
};

#endif

