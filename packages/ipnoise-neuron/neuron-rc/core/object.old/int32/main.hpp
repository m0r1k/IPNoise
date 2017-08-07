#include "class.hpp"
DEFINE_CLASS(ObjectInt32);

#ifndef OBJECT_INT32_HPP
#define OBJECT_INT32_HPP

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

#define _OBJECT_INT32(a_out, a_flags, ...)                  \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectInt32,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_INT32(a_out, a_flags, ...)                    \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectInt32,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_INT32(a_out, ...)                            \
    CREATE_OBJECT(a_out, ObjectInt32, ##__VA_ARGS__)

#define PROP_INT32(a_out, ...)                              \
    CREATE_PROP(a_out, ObjectInt32, ##__VA_ARGS__)

class ObjectInt32
    :   public Object
{
    public:
        ObjectInt32(const Object::CreateFlags &);
        ObjectInt32(
            const Object::CreateFlags   &,
            const int32_t               &a_val
        );
        virtual ~ObjectInt32();

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
        int32_t         getVal();
        operator        int();

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        int32_t m_val;

        void    do_init(const Object::CreateFlags &);
};

#endif

