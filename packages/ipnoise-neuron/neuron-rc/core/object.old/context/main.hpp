#include "class.hpp"
DEFINE_CLASS(ObjectContext);

#ifndef OBJECT_CONTEXT_HPP
#define OBJECT_CONTEXT_HPP

#include <stdint.h>

#include <string>
#include <map>
#include <mutex>

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
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectSession);
DEFINE_CLASS(ObjectTransaction);
DEFINE_CLASS(ObjectUser);

// XXX TODO FIXME remove
#define DEFAULT_DELIMITER   ": "

using namespace std;

// as default, don't add object in transaction here
#define _CREATE_OBJECT(a_out, a_class, a_flags, ...)        \
    do {                                                    \
        ObjectContextSptr context;                          \
        Object::CreateFlags flags;                          \
        flags = Object::CreateFlags(                        \
            a_flags | Object::CREATE_OBJECT                 \
        );                                                  \
        context = ObjectContext::getCurContext();           \
        a_out   = a_class ## Sptr(new a_class(              \
            flags,                                          \
            ##__VA_ARGS__                                   \
        ));                                                 \
        a_out->do_init_props();                             \
        a_out->do_init_api();                               \
    } while (0);

#define _CREATE_PROP(a_out, a_class, a_flags, ...)          \
    do {                                                    \
        Object::CreateFlags flags;                          \
        flags &= ~Object::CREATE_OBJECT;                    \
        a_out = a_class ## Sptr(new a_class(                \
            flags,                                          \
            ##__VA_ARGS__                                   \
        ));                                                 \
    } while (0);


#define CREATE_OBJECT(a_out, a_class, ...)                  \
    do {                                                    \
        ObjectContextSptr context;                          \
        context = ObjectContext::getCurContext();           \
        a_out   = a_class ## Sptr(new a_class(              \
            Object::CreateFlags(Object::CREATE_OBJECT),     \
            ##__VA_ARGS__                                   \
        ));                                                 \
        a_out->do_init_props();                             \
        a_out->do_init_api();                               \
        context->transactionAdd(a_out);                     \
    } while (0);

#define CREATE_PROP(a_out, a_class, ...)                    \
    do {                                                    \
        a_out = a_class ## Sptr(new a_class(                \
            Object::CreateFlags(Object::CREATE_PROP),       \
            ##__VA_ARGS__                                   \
        ));                                                 \
    } while (0);


#define _OBJECT_CONTEXT(a_out, a_flags, ...)                \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectContext,                                      \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_CONTEXT(a_out, a_flags, ...)                  \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectContext,                                      \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_CONTEXT(a_out, ...)                          \
    CREATE_OBJECT(a_out, ObjectContext, ##__VA_ARGS__ )

#define PROP_CONTEXT(a_out, ...)                            \
    CREATE_PROP(a_out, ObjectContext, ##__VA_ARGS__ )

class ObjectContext
    :   public Object
{
    public:
        ObjectContext(const Object::CreateFlags &);
        virtual ~ObjectContext();

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual string  toString();
        virtual string  serialize(
            const string &a_delim = DEFAULT_DELIMITER
        );
        virtual void    toBSON(
            mongo::BSONObjBuilder   &a_builder,
            const string            &a_field_name
        );
        virtual void    toBSON(
            mongo::BSONArrayBuilder &a_builder
        );
        virtual SV *    toPerl();

        // generic
        ObjectSessionSptr   getSession();
        string              getSessionId();
        void                setSession(ObjectSessionSptr);
//        void                updateEventSession(
//            ObjectSessionSptr,
//            int32_t a_event_fd = -1
//        );

        ObjectUserSptr  getUser();
        void            setUser(ObjectUserSptr);

        void            setFd(const int32_t &);
        int32_t         getFd();

        // transaction
        ObjectTransactionSptr   getTransaction();
        void                    transactionCommit();
        void                    transactionAdd(ObjectSptr);
        void                    transactionAddForSave(
            ObjectSptr  a_object
        );

        void    commit();

        ObjectSptr _getObjectById(
            const string    &a_id,
            const string    &a_type         = "",
            const int32_t   &a_get_removed  = 0,
            const int32_t   &a_db_lock      = 0,
            const int32_t   &a_do_autorun   = 1
        );

        template <class T = Object>
        shared_ptr<T> getObjectById(
            const string    &a_id,
            const string    &a_type         = "",
            const int32_t   &a_get_removed  = 0,
            const int32_t   &a_db_lock      = 0,
            const int32_t   &a_do_autorun   = 1)
        {
            lock_guard<recursive_mutex> guard(m_mutex);

            shared_ptr<T> ret;
            ret = dynamic_pointer_cast<T>(
                _getObjectById(
                    a_id,
                    a_type,
                    a_get_removed,
                    a_db_lock,
                    a_do_autorun
                )
            );
            return ret;
        }

        ObjectSptr  _getCreateObjectById(
            const string    &a_id,
            const string    &a_type,
            const int32_t   &a_get_removed  = 0,
            const int32_t   &a_db_lock      = 0,
            const int32_t   &a_do_autorun   = 1
        );

        template <class T>
        shared_ptr<T>  getCreateObjectById(
            const string    &a_id,
            const string    &a_type,
            const int32_t   &a_get_removed  = 0,
            const int32_t   &a_db_lock      = 0,
            const int32_t   &a_do_autorun   = 1)
        {
            shared_ptr<T> ret;

            ret = dynamic_pointer_cast<T>(
                _getCreateObjectById(
                    a_id,
                    a_type,
                    a_get_removed,
                    a_do_autorun
                )
            );

            return ret;
        };

        void    remove(
            ObjectSptr      a_object,
            const int32_t   a_force = 0
        );

        ObjectSptr  _getCurObject();
//        void        _setCurObject(ObjectSptr a_object);

        recursive_mutex     m_mutex;

        // static
        static ObjectUserSptr       getCurUser();
        static ObjectSessionSptr    getCurSession();
        static ObjectContextSptr    getCurContext();
        static ObjectSptr           getCurObject();
//        static void                 setCurObject(ObjectSptr);

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    protected:

    private:
        ObjectMapSptr   m_val;

//        ObjectUserSptr          m_user;
//        ObjectSessionSptr       m_session;
//        int32_t                 m_fd;

        // transaction
//        ObjectTransactionSptr   m_trans;

        void do_init(const Object::CreateFlags &);
};

#endif

