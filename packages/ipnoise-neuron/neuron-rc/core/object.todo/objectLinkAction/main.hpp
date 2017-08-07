#include "class.hpp"
DEFINE_CLASS(ObjectLinkAction);

#ifndef OBJECT_LINK_ACTION_HPP
#define OBJECT_LINK_ACTION_HPP

#include <string>

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
DEFINE_CLASS(ObjectLink);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);

using namespace std;

#define _OBJECT_LINK_ACTION(a_out, a_flags, ...)            \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectLinkAction,                                   \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_LINK_ACTION(a_out, a_flags, ...)              \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectLinkAction,                                   \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_LINK_ACTION(a_out, ...)                      \
    CREATE_OBJECT(a_out, ObjectLinkAction, ##__VA_ARGS__ )

#define PROP_LINK_ACTION(a_out, ...)                        \
    CREATE_PROP(a_out, ObjectLinkAction, ##__VA_ARGS__ )

class ObjectLinkAction
    :   public ObjectLink
{
    public:
        ObjectLinkAction(const Object::CreateFlags &);
        ObjectLinkAction(
            const Object::CreateFlags   &,
            ObjectLinkActionSptr
        );
        virtual ~ObjectLinkAction();

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
        virtual void    setup(
            Object  *a_src,
            Object  *a_dst
        );

        // generic

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        void    do_init(const Object::CreateFlags &);
};

#endif

