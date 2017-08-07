#include "class.hpp"
DEFINE_CLASS(ObjectGroup);

#ifndef OBJECT_GROUP_HPP
#define OBJECT_GROUP_HPP

#include <stdint.h>
#include <stdio.h>

#include <string>
//#include <algorithm>

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

#define _OBJECT_GROUP(a_out, a_flags, ...)                  \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectGroup,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_GROUP(a_out, a_flags, ...)                    \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectGroup,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_GROUP(a_out, ...)                            \
    CREATE_OBJECT(a_out, ObjectGroup, ##__VA_ARGS__ )

#define PROP_GROUP(a_out, ...)                              \
    CREATE_PROP(a_out, ObjectGroup, ##__VA_ARGS__ )

class ObjectGroup
    :   public Object
{
    public:
        ObjectGroup(const Object::CreateFlags &);
        ObjectGroup(
            const Object::CreateFlags   &,
            ObjectGroup
        );
        ObjectGroup(
            const Object::CreateFlags   &,
            const string                &
        );
        virtual ~ObjectGroup();

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

        // static

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

