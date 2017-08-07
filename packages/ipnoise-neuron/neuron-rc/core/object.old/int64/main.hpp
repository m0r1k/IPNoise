#include "class.hpp"
DEFINE_CLASS(ObjectInt64);

#ifndef OBJECT_INT64_HPP
#define OBJECT_INT64_HPP

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

using namespace std;

#define _OBJECT_INT64(a_out, a_flags, ...)                  \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectInt64,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_INT64(a_out, a_flags, ...)                    \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectInt64,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_INT64(a_out, ...)                            \
    CREATE_OBJECT(a_out, ObjectInt64, ##__VA_ARGS__ )

#define PROP_INT64(a_out, ...)                              \
    CREATE_PROP(a_out, ObjectInt64, ##__VA_ARGS__ )

class ObjectInt64
    :   public Object
{
    public:
        ObjectInt64(const Object::CreateFlags &);
        ObjectInt64(
            const Object::CreateFlags   &,
            const int64_t               &
        );
        virtual ~ObjectInt64();

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
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
        int64_t         getVal();
        operator        int64_t();

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        int64_t m_val;

        void    do_init(const Object::CreateFlags &);
};

#endif

