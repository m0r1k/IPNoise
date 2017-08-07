#include "class.hpp"
DEFINE_CLASS(ObjectTransaction);

#ifndef OBJECT_TRANSACTION_HPP
#define OBJECT_TRANSACTION_HPP

#include <stdio.h>

#include <string>

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
DEFINE_CLASS(ObjectString);

using namespace std;

#define _OBJECT_TRANSACTION(a_out, a_flags, ...)            \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectTransaction,                                  \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_TRANSACTION(a_out, a_flags, ...)              \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectTransaction,                                  \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_TRANSACTION(a_out, ...)                      \
    CREATE_OBJECT(a_out, ObjectTransaction, ##__VA_ARGS__)

#define PROP_TRANSACTION(a_out, ...)                        \
    CREATE_PROP(a_out, ObjectTransaction, ##__VA_ARGS__)

class ObjectTransaction
    :   public Object
{
    public:
        ObjectTransaction(const Object::CreateFlags &);
        virtual ~ObjectTransaction();

        // interface
        virtual string              getType();
        virtual void                do_init_props();
        virtual void                do_init_api();
        virtual ObjectStringSptr    toString();
        virtual ObjectStringSptr    serialize(
            const string &a_delim = DEFAULT_DELIMITER
        );
/*
        virtual void    toBSON(
            mongo::BSONObjBuilder   &a_builder,
            const string            &a_field_name
        );
        virtual void    toBSON(
            mongo::BSONArrayBuilder &a_builder
        );
        virtual SV *    toPerl();
*/

        // generic
        ObjectMapSptr   getObjectInfo(
            ObjectSptr a_object
        );
        ObjectMapSptr   getCreateObjectInfo(
            ObjectSptr a_object
        );
        void            add(ObjectSptr);
        void            addForSave(ObjectSptr);
        ObjectMapSptr   getVal();
        void            commit();
        void            remove(const string &a_object_id);

        ObjectSptr      _getObjectById(
            const string    &a_id,
            const string    &a_type         = "",
            const int32_t   &a_get_removed  = 0,
            const int32_t   &a_do_autorun   = 1
        );

        template <class T = Object>
        shared_ptr<T>   getObjectById(
            const string    &a_id,
            const string    &a_type         = "",
            const int32_t   &a_get_removed  = 0,
            const int32_t   &a_do_autorun   = 1)
        {
            lock_guard<recursive_mutex> guard(m_mutex);

            shared_ptr<T> ret;
            ret = dynamic_pointer_cast<T>(
                _getObjectById(
                    a_id,
                    a_type,
                    a_get_removed,
                    a_do_autorun
                )
            );
            return ret;
        }

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    protected:

    private:
        ObjectMapSptr   m_queue_all;
//        ObjectMapSptr   m_queue_save;

        void    do_init(const Object::CreateFlags &);
};

#endif

