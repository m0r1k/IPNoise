#include "class.hpp"
DEFINE_CLASS(ObjectGod);

#ifndef OBJECT_GOD_HPP
#define OBJECT_GOD_HPP

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
DEFINE_CLASS(ObjectUser);

#define GOD_LOGIN       "god"
#define GOD_PASSWORD    "god"

using namespace std;

#define _OBJECT_GOD(a_out, a_flags, ...)                    \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectGod,                                          \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_GOD(a_out, a_flags, ...)                      \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectGod,                                          \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_GOD(a_out, ...)                              \
    CREATE_OBJECT(a_out, ObjectGod, ##__VA_ARGS__)

#define PROP_GOD(a_out, ...)                                \
    CREATE_PROP(a_out, ObjectGod, ##__VA_ARGS__)

class ObjectGod
    :   public  ObjectUser
{
    public:
        ObjectGod(const Object::CreateFlags &);
        virtual ~ObjectGod();

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
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        void    do_init(const Object::CreateFlags &);
};

#endif

