#include "class.hpp"
DEFINE_CLASS(ObjectLink);

#ifndef OBJECT_LINK_HPP
#define OBJECT_LINK_HPP

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

#define _OBJECT_LINK(a_out, a_flags, ...)                   \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectLink,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_LINK(a_out, a_flags, ...)                     \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectLink,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_LINK(a_out, ...)                             \
    CREATE_OBJECT(a_out, ObjectLink, ##__VA_ARGS__)

#define PROP_LINK(a_out, ...)                               \
    CREATE_PROP(a_out, ObjectLink, ##__VA_ARGS__)

class ObjectLink
    :   public Object
{
    public:
        ObjectLink(const Object::CreateFlags &);
        virtual ~ObjectLink();

        virtual string  serialize(
            const string &a_delim = DEFAULT_DELIMITER
        );

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual void    toBSON(
            mongo::BSONObjBuilder   &a_builder,
            const string            &a_field_name
        );
        virtual void    toBSON(
            mongo::BSONArrayBuilder &a_builder
        );
        virtual SV *    toPerl();
//        virtual void    remove(const int32_t &a_force = 0);
//        virtual void    setup(
//            Object  *a_src,
//            Object  *a_dst
//        );

        // generic
        ObjectMapSptr   getVal();
        void            setLinkCreator(const string &a_val);

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    protected:
        ObjectMapSptr   m_val;

    private:
        void    do_init(const Object::CreateFlags &);
};

#endif

