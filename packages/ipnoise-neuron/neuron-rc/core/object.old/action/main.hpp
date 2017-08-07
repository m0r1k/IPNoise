#include "class.hpp"
DEFINE_CLASS(ObjectAction);

#ifndef OBJECT_ACTION_HPP
#define OBJECT_ACTION_HPP

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
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectTime);

using namespace std;

#define _OBJECT_ACTION(a_out, a_flags, ...)                 \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectAction,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    );

#define _PROP_ACTION(a_out, a_flags, ...)                   \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectAction,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_ACTION(a_out, ...)                           \
    CREATE_OBJECT(a_out, ObjectAction, ##__VA_ARGS__);

#define PROP_ACTION(a_out, ...)                             \
    CREATE_PROP(a_out, ObjectAction, ##__VA_ARGS__)

class ObjectAction
    :   public Object
{
    public:
        ObjectAction(
            const Object::CreateFlags   &a_flags,
            const string                &a_name
        );
        virtual ~ObjectAction();

        DEFINE_PROP(Location,       ObjectStringSptr);
        DEFINE_PROP(TimeProcess,    ObjectTimeSptr);
        DEFINE_PROP(Info,           ObjectMapSptr);

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual void    getAllProps(
            ObjectMapSptr a_props
        );
        virtual int32_t parseBSON(mongo::BSONObj);
//        virtual string  serialize(
//            const string &a_delim = DEFAULT_DELIMITER
//        );
//        virtual void    toBSON(
//            mongo::BSONObjBuilder   &a_builder,
//            const string            &a_field_name
//        );
//        virtual void    toBSON(
//            mongo::BSONArrayBuilder &a_builder
//        );
//        virtual SV *    toPerl();

        // generic
//        ObjectLinkSptr      addObject(Object*);
//        ObjectLinkSptr      addObject(ObjectSptr);

        void                schedule(
            ObjectTimeSptr a_time = ObjectTimeSptr()
        );
        void                scheduleCurThread(
            ObjectTimeSptr a_time = ObjectTimeSptr()
        );
        // params
        ObjectMapSptr       getParams();
        ObjectSptr          getParam(const string &a_name);

        // props
        void    addProp(
            const string    &a_key,
            const string    &a_val
        );
        void    addProp(
            const string    &a_key,
            ObjectSptr      a_val
        );
        void    addProp(
            ObjectSptr      a_key,
            const string    &a_val
        );
        void    addProp(
            ObjectSptr      a_key,
            ObjectSptr      a_val
        );
        // params
        void    addParam(
            const string    &a_key,
            const string    &a_val
        );
        void    addParam(
            const string    &a_key,
            ObjectSptr      a_val
        );
        void    addParam(
            ObjectSptr      a_key,
            const string    &a_val
        );
        void    addParam(
            ObjectSptr      a_key,
            ObjectSptr      a_val
        );

        // static
        static bool isAction(ObjectSptr);

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        string  m_name;

        void    do_init(
            const Object::CreateFlags   &a_flags,
            const string                &a_name
        );
};

#endif

