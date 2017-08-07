#include "class.hpp"
DEFINE_CLASS(ObjectType);

#ifndef OBJECT_TYPE_HPP
#define OBJECT_TYPE_HPP

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <map>

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

#define _OBJECT_TYPE(a_out, a_flags, ...)                   \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectType,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_TYPE(a_out, a_flags, ...)                     \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectType,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_TYPE(a_out, ...)                             \
    CREATE_OBJECT(a_out, ObjectType, ##__VA_ARGS__)

#define PROP_TYPE(a_out, ...)                               \
    CREATE_PROP(a_out, ObjectType, ##__VA_ARGS__)

class ObjectType
    :   public Object
{
    public:
        ObjectType(const Object::CreateFlags &);
        ObjectType(
            const Object::CreateFlags &,
            const string &
        );
        ObjectType(
            const Object::CreateFlags &,
            const char *
        );
        virtual ~ObjectType();

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
        operator    string();

        // static
//        static string   serializeInfo(const string &a_delim);

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        ObjectStringSptr  m_val;

        void    do_init(const Object::CreateFlags &);
};

#endif

