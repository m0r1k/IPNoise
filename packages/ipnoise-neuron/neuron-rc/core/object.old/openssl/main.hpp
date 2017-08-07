#include "class.hpp"
DEFINE_CLASS(ObjectOpenSSL);

#ifndef OBJECT_OPENSSL_HPP
#define OBJECT_OPENSSL_HPP

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

using namespace std;

#define _OBJECT_OPENSSL(a_out, a_flags, ...)                \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectOpenSSL,                                      \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_OPENSSL(a_out, a_flags, ...)                  \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectOpenSSL,                                      \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_OPENSSL(a_out, ...)                          \
    CREATE_OBJECT(a_out, ObjectOpenSSL, ##__VA_ARGS__)

#define PROP_OPENSSL(a_out, ...)                            \
    _CREATE_PROP(a_out, ObjectOpenSSL, ##__VA_ARGS__)

class ObjectOpenSSL
    :   public Object
{
    public:
        ObjectOpenSSL(const Object::CreateFlags &);
        virtual ~ObjectOpenSSL();

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
        ObjectMapSptr   getVal();

        // static
        static string sha1(
            const string    &a_data,
            const int32_t   &a_output_base64 = 1
        );

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        ObjectMapSptr   m_val;

        void    do_init(const Object::CreateFlags &);
};

#endif

