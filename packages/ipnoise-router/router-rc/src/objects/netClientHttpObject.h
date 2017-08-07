/*
 *
 *  *** IPNoise ***
 *
 *  Roman E. Chechnev May 2011 (c)
 *  <ipnoise@chechnev.ru>
 *
 */

// stl
#include <string>
#include <vector>
#include <map>

using namespace std;

class NetClientHttpObject;

typedef map <string, string> URL_INFO;
typedef map <string, string> HEADERS;

#ifndef NET_CLIENT_HTTP_OBJECT_H
#define NET_CLIENT_HTTP_OBJECT_H

#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <time.h>

#include <libxml/tree.h>

#include <ipnoise-common/log.h>
#include <ipnoise-common/urls.h>

class DomDocument;

#include "objects/netClientObject.h"
#include "objects/object.h"

class NetClientHttpObject
    :   public NetClientObject
{
    public:
        NetClientHttpObject(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_net_client_http"
        );
        ~NetClientHttpObject();

        // internal
        virtual NetClientHttpObject * create_object(
            xmlNodePtr      a_node,
            DomDocument     *a_doc,
            const string    &a_tagname = "ipn_net_client_http"
        );

        // generic
        int     setRequestHeader(string name, string value);
        string  getRequestHeader(string name);
        int     unsetRequestHeader(string name);
        HEADERS getRequestHeaders();
        int     setRequestMethod(string _method);
        string  getRequestMethod();
        int     setRequestProtocol(string _protocol);
        string  getRequestProtocol();
        int     setRequestContent(string content);
        string  getRequestContent();
        int     setRequestContentType(string content_type);
        string  getRequestContentType();

        // answer
        HEADERS getAnswerHeaders();
        string  getAnswerHeader(string name);
        string  getAnswerProtocol();
        string  getAnswerRespCode();
        string  getAnswerRespDescr();
        string  getAnswerContent();
        string  getAnswerContentType();
        size_t  getAnswerContentLength();

        // http methods
        int get(string url);
        int post(string url);

        // setup partial read callback
        void setHttpPartialReadCb(
            void (*_partial_read_cb)(
                NetClientHttpObject     *client,
                size_t                  current_offset,
                size_t                  current_length,
                void                    *ctx
            ),
            void *_partial_read_cb_ctx
        );

        // setup connected callback
        void setHttpConnectedCb(
            void (*_connected_cb)(
                NetClientHttpObject     *client,
                size_t                  current_offset,
                size_t                  current_length,
                void                    *ctx
            ),
            void *_connected_cb_ctx
        );

        // setup connect closed callback
        void setHttpConnectClosedCb(
            void (*_connect_closed_cb)(
                NetClientHttpObject     *client,
                size_t                  current_offset,
                size_t                  current_length,
                void                    *ctx
            ),
            void *_connect_closed_cb_ctx
        );

        // setup http answer callback
        void setHttpAnswerCb(
            void (*_http_answer_cb)(
                NetClientHttpObject     *client,
                size_t                  current_offset,
                size_t                  current_length,
                void                    *ctx
            ),
            void *_http_answer_cb_ctx
        );

    protected:
        int send_request(string url);

        // answer
        int setAnswerHeader(string name, string value);
        int unsetAnswerHeader(string name);
        int setAnswerStatusLine(string line);
        int setAnswerProtocol(string _protocol);
        int setAnswerRespCode(string _resp_code);
        int setAnswerRespDescr(string _resp_descr);


        // called when new data for read available
        static void partial_read_cb(
            NetClientObject         *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );

        // called when connection established
        static void connected_cb(
            NetClientObject         *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );

        // called when connection closed
        static void connect_closed_cb(
            NetClientObject         *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );

    private:
        // connect url info
        URL_INFO url_info;

        // request
        string      request_method;
        string      request_protocol;
        HEADERS     request_headers;
        string      request_content;

        // answer
        int         answer_headers_was_read;
        string      answer_status_line;
        HEADERS     answer_headers;
        string      answer_protocol;
        string      answer_resp_code;
        string      answer_resp_descr;

        // partial read callback
        void (*http_partial_read_cb)(
            NetClientHttpObject     *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );
        void *http_partial_read_cb_ctx;

        // connection established callback
        void (*http_connected_cb)(
            NetClientHttpObject     *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );
        void *http_connected_cb_ctx;

        // connection closed callback
        void (*http_connect_closed_cb)(
            NetClientHttpObject     *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );
        void *http_connect_closed_cb_ctx;

        // http answer callback
        void (*http_answer_cb)(
            NetClientHttpObject     *client,
            size_t                  current_offset,
            size_t                  current_length,
            void                    *ctx
        );
        void *http_answer_cb_ctx;
};

#endif

