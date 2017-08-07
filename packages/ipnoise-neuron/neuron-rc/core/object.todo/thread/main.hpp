#include "class.hpp"
DEFINE_CLASS(ObjectThread);

#ifndef OBJECT_THREAD_HPP
#define OBJECT_THREAD_HPP

#include <stdint.h>
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

DEFINE_CLASS(Thread);

using namespace std;

#define _OBJECT_THREAD(a_out, a_flags, ...)                 \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectThread,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_THREAD(a_out, a_flags, ...)                   \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectThread,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_THREAD(a_out, ...)                           \
    CREATE_OBJECT(a_out, ObjectThread, ##__VA_ARGS__)

#define PROP_THREAD(a_out, ...)                             \
    CREATE_PROP(a_out, ObjectThread, ##__VA_ARGS__)

class ObjectThread
    :   public Object
{
    public:
        ObjectThread(const Object::CreateFlags &);
        ObjectThread(
            const Object::CreateFlags &,
            ThreadSptr
        );
        virtual ~ObjectThread();

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

        // generic
        ThreadSptr  getVal();

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        ThreadSptr  m_val;

        void    do_init(const Object::CreateFlags &);
};

#endif

