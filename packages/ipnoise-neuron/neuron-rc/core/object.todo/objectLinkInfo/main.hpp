#include "class.hpp"
DEFINE_CLASS(ObjectLinkInfo);

#ifndef OBJECT_LINK_INFO_HPP
#define OBJECT_LINK_INFO_HPP

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
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectLink);

using namespace std;

#define _OBJECT_LINK_INFO(a_out, a_flags, ...)              \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectLinkInfo,                                     \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_LINK_INFO(a_out, a_flags, ...)                \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectLinkInfo,                                     \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_LINK_INFO(a_out, ...)                        \
    CREATE_OBJECT(a_out, ObjectLinkInfo, ##__VA_ARGS__ )

#define PROP_LINK_INFO(a_out, ...)                          \
    CREATE_PROP(a_out, ObjectLinkInfo, ##__VA_ARGS__ )

class ObjectLinkInfo
    :   public ObjectLink
{
    public:
        ObjectLinkInfo(const Object::CreateFlags &);
        ObjectLinkInfo(
            const Object::CreateFlags &,
            ObjectLinkInfoSptr
        );
        virtual ~ObjectLinkInfo();

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
        void    addNeigh(ObjectSptr);
        void    addNeigh(Object *);
        void    setWalkers(ObjectMapSptr a_walkers);

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

