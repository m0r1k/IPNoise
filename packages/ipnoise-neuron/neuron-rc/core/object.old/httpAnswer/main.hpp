#include "class.hpp"
DEFINE_CLASS(ObjectHttpAnswer);

#ifndef OBJECT_HTTP_ANSWER_HPP
#define OBJECT_HTTP_ANSWER_HPP

#include <stdint.h>

#include <string>
#include <map>
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
DEFINE_CLASS(ObjectHttpRequest);
DEFINE_CLASS(ObjectMap);

using namespace std;

#define _OBJECT_HTTP_ANSWER(a_out, a_flags, ...)            \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectHttpAnswer,                                   \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_HTTP_ANSWER(a_out, a_flags, ...)              \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectHttpAnswer,                                   \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_HTTP_ANSWER(a_out, ...)                      \
    CREATE_OBJECT(a_out, ObjectHttpAnswer, ##__VA_ARGS__)

#define PROP_HTTP_ANSWER(a_out, ...)                        \
    CREATE_PROP(a_out, ObjectHttpAnswer, ##__VA_ARGS__)

class ObjectHttpAnswer
    :   public ObjectHttpRequest
{
    public:
        ObjectHttpAnswer(const Object::CreateFlags &);
        ObjectHttpAnswer(
            const Object::CreateFlags &,
            ObjectHttpRequestSptr
        );
        virtual ~ObjectHttpAnswer();

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();

        // generic
        string  getWaitingReqId() const;
        void    setWaitingReqId(const string &);

        void eraseAnswerHeader(
            const string    &a_key
        );
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
        void        eraseAnswerBody();
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


        void    setAnswerApiStatus(const string &);
        void    setAnswerApiDescr(const string &);
        void    setAnswerApiReqId(const string &);
        void    setAnswerApiSessid(const string &);

        void                setAnswerApi(ObjectMapSptr);
        ObjectMapSptr       getAnswerApi();
        ObjectMapSptr       getCreateAnswerApi();
        ObjectMapSptr       getAnswerHeaders();
        ObjectMapSptr       getCreateAnswerHeaders();
        ObjectMapSptr       getAnswerParams();
        ObjectMapSptr       getCreateAnswerParams();
        void                setAnswerParam(
            const string &a_name,
            const string &a_val
        );
        void            setAnswerParam(
            const string    &a_name,
            ObjectSptr        a_val
        );


        string  toHttp();
        string  dump();

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        string          m_answer_proto;
        int32_t         m_answer_status_code;
        string          m_answer_status_string;
        string          m_answer_body;
        ObjectMapSptr   m_answer_api;
        ObjectMapSptr   m_answer_headers;
        string          m_waiting_req_id;

        void    do_init(const Object::CreateFlags &);
};

#endif

