#include "class.hpp"
DEFINE_CLASS(ObjectCore);

#ifndef OBJECT_CORE_HPP
#define OBJECT_CORE_HPP

#include <stdio.h>

#include <map>
#include <string>
#include <memory>

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

#define _OBJECT_CORE(a_out, a_flags, ...)                   \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectCore,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_CORE(a_out, a_flags, ...)                     \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectCore,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_CORE(a_out, ...)                             \
    CREATE_OBJECT(a_out, ObjectCore, ##__VA_ARGS__ )

#define PROP_CORE(a_out, ...)                               \
    CREATE_PROP(a_out, ObjectCore, ##__VA_ARGS__ )

class ObjectCore
    :   public  Object
{
    public:
        ObjectCore(const Object::CreateFlags &);
        virtual ~ObjectCore();

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual int32_t do_autorun();
        virtual void    getAllProps(
            ObjectMapSptr a_props
        );
        virtual int32_t parseBSON(mongo::BSONObj);

        // generic

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _module_init();
        static void         _module_destroy();
        static ObjectSptr   _module_object_create();

    private:
        void    do_init(const Object::CreateFlags &);
};

#endif

