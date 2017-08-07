#include "class.hpp"
DEFINE_CLASS(ObjectSession);

#ifndef OBJECT_SESSION_HPP
#define OBJECT_SESSION_HPP

#include <stdio.h>

#include <map>
#include <string>
#include <memory>

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
DEFINE_CLASS(ObjectId);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectUser);
DEFINE_CLASS(ObjectVector);

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
    CREATE_OBJECT(a_out, ObjectSession, ##__VA_ARGS__ )

#define PROP_SESSION(a_out, ...)                            \
    CREATE_PROP(a_out, ObjectSession, ##__VA_ARGS__ )

class ObjectSession
    :   public  Object
{
    public:
        ObjectSession(const Object::CreateFlags &);
        virtual ~ObjectSession();

        DEFINE_PROP(CurObjectId,    ObjectIdSptr);

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual int32_t do_autorun();
        virtual void    getAllProps(
            ObjectMapSptr a_props
        );
        virtual int32_t parseBSON(mongo::BSONObj);
        virtual void    processEvents();

        // generic
        ObjectUserSptr  getUser();
        void            setCurObject(ObjectSptr);
        ObjectSptr      getCurObject();
        ObjectUserSptr  unlinkFromRegisteredUsers();

        // isOnline
        int32_t         isOnline();

//        void          setWaitingReqId(const string &);
//        string        getWaitingReqId() const;

//        void          setWaitingReqFd(int32_t);
//        int32_t       getWaitingReqFd() const;
//        void          closeEventSocket();

//        void setEventSocket(TcpSocketSptr);

        // static
        static ObjectSessionSptr    getSession(
            const string &a_sessid
        );

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    //protected slots:
    //    void    waitingReqSlot(int);

    private:
        //int32_t             m_waiting_req_fd;
        //string              m_waiting_req_id;
        //QSocketNotifierSptr m_waiting_req_notifier;

        void    do_init(const Object::CreateFlags &);
};

#endif

