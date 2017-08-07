#include "class.hpp"
DEFINE_CLASS(ObjectSession);

#ifndef OBJECT_SESSION_HPP
#define OBJECT_SESSION_HPP

#include <stdint.h>
#include <string>
#include <mutex>

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectAction);
DEFINE_CLASS(ObjectCore);
DEFINE_CLASS(ObjectCamera);
DEFINE_CLASS(ObjectGod);
DEFINE_CLASS(ObjectInt32);
DEFINE_CLASS(ObjectHttpAnswer);
DEFINE_CLASS(ObjectHttpRequest);
DEFINE_CLASS(ObjectSession);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectUser);
DEFINE_CLASS(ObjectVec3);

using namespace std;

#define _OBJECT_SESSION(a_out, a_flags, ...)                \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectSession,                                      \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_SESSION(a_out, a_flags, ...)                  \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectSession,                                      \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_SESSION(a_out, ...)                          \
    CREATE_OBJECT(a_out, ObjectSession, ##__VA_ARGS__)

#define PROP_SESSION(a_out, ...)                            \
    CREATE_PROP(a_out, ObjectSession, ##__VA_ARGS__)

class ObjectSession
    :   public Object
{
    public:
        ObjectSession(const Object::CreateFlags &);
        virtual ~ObjectSession();

        DEFINE_PROP(Fd,                 ObjectInt32Sptr);
//        DEFINE_PROP(WebsockMask,        ObjectInt32Sptr);
        DEFINE_PROP(WebsockAccept,      ObjectStringSptr);
//        DEFINE_PROP(WebsockPendingData, ObjectStringSptr);

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual void    getAllProps(
            ObjectMapSptr a_props
        );
        virtual int32_t parseBSON(mongo::BSONObj);
//        virtual void    remove(const int32_t &a_force);

        // generic
        void                    setFd(const int32_t &);
//        ObjectHttpAnswerSptr    getAnswer();
//        int32_t                 sendAnswer(
//            const int32_t &a_stop_thread = 1
//        );

        static int32_t parseData(
            ObjectSessionSptr,
            const string &
        );
        bool    isWebsock();

        // error codes
        void setError(
            ObjectHttpAnswerSptr,
            ObjectMapSptr   a_req_props,
            ObjectMapSptr   a_req_params,
            const int32_t   &a_status_code,
            const string    &a_status_string
        );
        void set302(
            ObjectHttpAnswerSptr,
            ObjectMapSptr   a_req_props,
            ObjectMapSptr   a_req_params,
            const string    &a_location,
            const string    &a_status_string = "Found"
        );
        void set404(
            ObjectHttpAnswerSptr,
            ObjectMapSptr   a_req_props,
            ObjectMapSptr   a_req_params,
            const string    &a_status_string = "Not found"
        );
        void set500(
            ObjectHttpAnswerSptr,
            ObjectMapSptr   a_req_props,
            ObjectMapSptr   a_req_params,
            const string    &a_status_string = "Internal error"
        );

        // static
        static ObjectSessionSptr createSession(
            ObjectHttpAnswerSptr,
            ObjectUserSptr
        );

        static ObjectMapSptr getParams(
            ObjectMapSptr a_req_props
        );
        static ObjectMapSptr getCreateParams(
            ObjectMapSptr a_req_props
        );

        // api
        static void     actionLogout(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void     actionRegister(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void     actionLogin(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void     actionIoRead(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void     actionIoWrite(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );
        static void     actionIoEvent(
            ObjectSptr          a_object,
            ObjectActionSptr    a_action
        );

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    protected:
        ObjectSptr      getCurObject(
            ObjectMapSptr a_req_props,
            ObjectMapSptr a_req_params
        );
        int32_t         willRedirect(
            ObjectHttpAnswerSptr    a_answer,
            ObjectMapSptr           a_req_props,
            ObjectMapSptr           a_req_params
        );
        static void     processPost(
            ObjectSessionSptr       a_session,
            ObjectHttpAnswerSptr    a_answer,
            ObjectMapSptr           a_req_props,
            ObjectMapSptr           a_req_params
        );
        static void     processGet(
            ObjectSessionSptr       a_session,
            ObjectHttpAnswerSptr    a_answer,
            ObjectMapSptr           a_req_props,
            ObjectMapSptr           a_req_params
        );

        static ObjectUserSptr  getUser(
            ObjectSessionSptr
        );

        static int32_t parseDataWebsock(
            ObjectSessionSptr,
            const string &
        );
        static int32_t parseDataHttp(
            ObjectSessionSptr,
            const string &
        );

/*
        void            setupContext(
            ObjectHttpAnswerSptr    a_answer,
            ObjectMapSptr           a_req_props,
            ObjectMapSptr           a_req_params
        );
*/
        static void     processHttp(
            ObjectSessionSptr,
            ObjectHttpRequestSptr
        );
        static void     processRequest(
            ObjectHttpAnswerSptr    a_answer,
            const ObjectMapSptr     a_req_props,
            const ObjectMapSptr     a_req_params
        );
        void            isWebsock(const int32_t &);

    private:
//        ObjectHttpRequestSptr       m_req;
//        ObjectHttpAnswerSptr        m_answer;

        void    do_init(const Object::CreateFlags &);
};

#endif

