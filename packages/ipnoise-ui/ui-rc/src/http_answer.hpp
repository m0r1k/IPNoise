#include "class.hpp"
DEFINE_CLASS(HttpAnswer);

#ifndef HTTP_ANSWER_HPP
#define HTTP_ANSWER_HPP

#include <stdint.h>

#include <string>
#include <map>
#include <algorithm>

#include <QString>
#include <QStringList>
#include <QTcpSocket>

#include "prop/map.hpp"
#include "prop/string.hpp"
#include "http_request.hpp"

using namespace std;

class HttpAnswer
    :   public HttpRequest
{
    public:
        HttpAnswer();
        HttpAnswer(QTcpSocket *);
        HttpAnswer(const HttpRequest &);
        virtual ~HttpAnswer();

        int32_t needDisconnect();
        void    needDisconnect(int32_t);

        void setAnswerHeader(
            const string    &a_key,
            const string    &a_val
        );

        void        setAnswerStatusCode(const int32_t &);
        int32_t     getAnswerStatusCode();
        void        setAnswerProto(const string &);
        string      getAnswerProto();
        void        setAnswerStatusString(const string &);
        string      getAnswerStatusString();
        void        setAnswerBody(const string &);
        string      getAnswerBody();

        void        setAnswerCookie(
            const string    &a_name,
            const string    &a_val,
            const string    &a_expires  = "",
            const string    &a_path     = "/",
            const string    &a_domain   = "",
            const int32_t   &a_secure   = 0
        );


        void            setAnswerApiStatus(const string &);
        void            setAnswerApiDescr(const string &);
        void            setAnswerApiReqId(const string &);
        void            setAnswerApiSessid(const string &);

        PropMapSptr     getAnswerApi();
        PropMapSptr     getCreateAnswerApi();
        PropMapSptr     getAnswerParams();
        PropMapSptr     getCreateAnswerParams();
        void            setAnswerParam(
            const string &a_name,
            const string &a_val
        );
        void            setAnswerParam(
            const string    &a_name,
            PropSptr        a_val
        );


        string  toHttp();
        string  dump();

    private:
        void        do_init();

        string      m_answer_proto;
        int32_t     m_answer_status_code;
        string      m_answer_status_string;
        string      m_answer_body;
        PropMapSptr m_answer_api;
        PropMap     m_answer_headers;
        int32_t     m_need_disconnect;
};

#endif

