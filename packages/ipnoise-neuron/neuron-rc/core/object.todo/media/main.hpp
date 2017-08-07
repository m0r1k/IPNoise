#include "class.hpp"
DEFINE_CLASS(ObjectMedia);

#ifndef OBJECT_MEDIA_HPP
#define OBJECT_MEDIA_HPP

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
DEFINE_CLASS(ObjectHttpAnswer);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectMap);

using namespace std;

#define _OBJECT_MEDIA(a_out, a_flags, ...)                  \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectMedia,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_MEDIA(a_out, a_flags, ...)                    \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectMedia,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_MEDIA(a_out, ...)                            \
    CREATE_OBJECT(a_out, ObjectMedia, ##__VA_ARGS__ )

#define PROP_MEDIA(a_out, ...)                              \
    CREATE_PROP(a_out, ObjectMedia, ##__VA_ARGS__ )

class ObjectMedia
    :   public  Object
{
    public:
        ObjectMedia(const Object::CreateFlags &);
        virtual ~ObjectMedia();

        DEFINE_PROP(Title,  ObjectStringSptr);
        DEFINE_PROP(Status, ObjectStringSptr);

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

        // api
        static void actionUpdate(
            Object          *a_object,
            ObjectAction    *a_action
        );
        static void actionSetStatus(
            Object          *a_object,
            ObjectAction    *a_action
        );

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

