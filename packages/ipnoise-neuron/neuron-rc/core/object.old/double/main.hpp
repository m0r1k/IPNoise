#include "class.hpp"
DEFINE_CLASS(ObjectDouble);

#ifndef OBJECT_DOUBLE_HPP
#define OBJECT_DOUBLE_HPP

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

#define _OBJECT_DOUBLE(a_out, a_flags, ...)                 \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectDouble,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_DOUBLE(a_out, a_flags, ...)                   \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectDouble,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_DOUBLE(a_out, ...)                           \
    CREATE_OBJECT(a_out, ObjectDouble, ##__VA_ARGS__ )

#define PROP_DOUBLE(a_out, ...)                             \
    CREATE_PROP(a_out, ObjectDouble, ##__VA_ARGS__ )

class ObjectDouble
    :   public Object
{
    public:
        ObjectDouble(const Object::CreateFlags &);
        ObjectDouble(
            const Object::CreateFlags   &,
            const double                &
        );
        virtual ~ObjectDouble();

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
        double      getVal();
        operator    double();

/*
        ObjectDouble & operator -= (
            const ObjectDouble &a_right
        );

        const ObjectDouble operator - (
            const ObjectDouble &a_right
        ) const;

        bool operator != (
            const ObjectDouble &a_right
        ) const;
*/

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        double   m_val;

        void    do_init(
            const Object::CreateFlags   &,
            const double                &a_val = 0.0f
        );
};

#endif

