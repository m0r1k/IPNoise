#include "class.hpp"
DEFINE_CLASS(ObjectParam);

#ifndef OBJECT_PARAM_HPP
#define OBJECT_PARAM_HPP

#include <stdio.h>

#include <map>
#include <string>
#include <memory>

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectAction);
DEFINE_CLASS(ObjectHttpAnswer);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);

using namespace std;

#define _OBJECT_PARAM(a_out, a_flags, ...)                  \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectParam,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_PARAM(a_out, a_flags, ...)                    \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectParam,                                        \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_PARAM(a_out, ...)                            \
    CREATE_OBJECT(a_out, ObjectParam, ##__VA_ARGS__ )

#define PROP_PARAM(a_out, ...)                              \
    CREATE_PROP(a_out, ObjectParam, ##__VA_ARGS__ )

class ObjectParam
    :   public  Object
{
    public:
        ObjectParam(const Object::CreateFlags &);
        virtual ~ObjectParam();

        DEFINE_PROP(Value, ObjectStringSptr);

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual int32_t do_autorun();
        virtual void    getAllProps(
            ObjectMapSptr a_props
        );
//        virtual int32_t save();
        virtual int32_t parseBSON(mongo::BSONObj);

        // generic
        void setValue(const string &);

        // static

        // api
        static void actionUpdate(
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

