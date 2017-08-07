#include "class.hpp"
DEFINE_CLASS(ObjectUint32);

#ifndef OBJECT_UINT32_HPP
#define OBJECT_UINT32_HPP

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
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);

using namespace std;

#define _OBJECT_UINT32(a_out, a_flags, ...)                 \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectUint32,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_UINT32(a_out, a_flags, ...)                   \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectUint32,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_UINT32(a_out, ...)                           \
    CREATE_OBJECT(a_out, ObjectUint32, ##__VA_ARGS__ )

#define PROP_UINT32(a_out, ...)                             \
    CREATE_PROP(a_out, ObjectUint32, ##__VA_ARGS__ )

class ObjectUint32
    :   public Object
{
    public:
        ObjectUint32(const Object::CreateFlags &);
        ObjectUint32(
            const Object::CreateFlags   &,
            const uint32_t              &
        );
        virtual ~ObjectUint32();

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual string  serialize(
            const string    &a_delim = DEFAULT_DELIMITER
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
        uint32_t    getVal();
        operator    uint32_t();

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        uint32_t    m_val;

        void        do_init(const Object::CreateFlags &);
};

#endif

