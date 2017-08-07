#include "class.hpp"
DEFINE_CLASS(ObjectHttpRequest);

#ifndef OBJECT_HTTP_REQUEST_HPP
#define OBJECT_HTTP_REQUEST_HPP

#include <string>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectInt32);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectVector);

using namespace std;

class ObjectHttpRequest
    :   public Object
{
    public:
        ObjectHttpRequest(const CreateFlags &a_create_flags);
        virtual ~ObjectHttpRequest();

        virtual string      getType();
        virtual int32_t     do_init_as_prop(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual int32_t     do_init_as_object(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual ObjectSptr  copy();
        virtual void        getAllProps(ObjectMapSptr);
        virtual ObjectStringSptr serializeAsProp(
            const char  *a_delim = DEFAULT_DELIMITER
        );

        // generic
        int32_t             parseRequest(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        ObjectStringSptr    getReqUri();
        ObjectStringSptr    getReqBody();
        ObjectStringSptr    getReqMethod();
        ObjectStringSptr    getReqProto();
        void                getReqApi(ObjectMapSptr);
        ObjectStringSptr    getReqCookie(const char *);
        ObjectInt32Sptr     getReqContentSize();
        ObjectVectorSptr    getReqBoundaries();
        ObjectStringSptr    getReqHeader(const char *);
        ObjectStringSptr    getReqDispositionName();
        ObjectStringSptr    getReqDispositionFileName();
        ObjectInt32Sptr     isReqBoundary();
        ObjectStringSptr    dump();
        ObjectInt32Sptr     isReqWebsock();
        ObjectStringSptr    getReqWebsockAccept();

        // static

        // module
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static  void        s_getTests(Tests &);

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
        int32_t     parseProtoLine(const char *);
        ObjectSptr  parseQ(const char *);
        int32_t     parseHeaderLine(const char *);
        int32_t     parseBoundaryLine(const char *);
        int32_t     parseLine(const char *);
        string      debugLine(const char *);

        void        parseReqApi();
        void        parseCookie(const char *);
        void        contentDisposition(const char *);
        void        chompContent(ObjectHttpRequest *);
        void        chompLastBoundary();
        int32_t     process_buffer(
            const int32_t &a_force = 0
        );
        void        upgradeToWebSocket();
        void        upgrade();
        void        parseContentType(const char *);

    private:
        ObjectStringSptr    m_req_buffer;

        ObjectInt32Sptr     m_req_is_boundary;
        ObjectStringSptr    m_req_method;
        ObjectStringSptr    m_req_uri;
        ObjectStringSptr    m_req_path;
        ObjectStringSptr    m_req_proto;
        ObjectStringSptr    m_req_boundary_delim;
        ObjectStringSptr    m_req_boundary_delim_end;
        ObjectStringSptr    m_req_body;
        ObjectInt32Sptr     m_req_content_length;

        ObjectVectorSptr    m_req_boundaries;
        ObjectMapSptr       m_req_disposition;
        ObjectMapSptr       m_req_headers;
        ObjectMapSptr       m_req_cookies;
        ObjectMapSptr       m_req_api;
        ObjectInt32Sptr     m_req_parse_state;
        ObjectStringSptr    m_req_websock_accept;
};

#endif

