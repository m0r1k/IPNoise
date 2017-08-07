#include "class.hpp"
DEFINE_CLASS(ObjectTime);

#ifndef OBJECT_TIME_HPP
#define OBJECT_TIME_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sys/time.h>

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
DEFINE_CLASS(ObjectDouble);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);

using namespace std;

#define _OBJECT_TIME(a_out, a_flags, ...)                   \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectTime,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_TIME(a_out, a_flags, ...)                     \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectTime,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_TIME(a_out, ...)                             \
    CREATE_OBJECT(a_out, ObjectTime, ##__VA_ARGS__)

#define PROP_TIME(a_out, ...)                               \
    CREATE_PROP(a_out, ObjectTime, ##__VA_ARGS__)


#define OBJECT_TIME_NOW(a_out, a_flags, ...)                \
    do {                                                    \
        OBJECT_TIME(a_out, ##__VA_ARGS__);                  \
        a_out->reset();                                     \
    } while (0)

#define PROP_TIME_NOW(a_out, ...)                           \
    do {                                                    \
        PROP_TIME(a_out, ##__VA_ARGS__);                    \
        a_out->reset();                                     \
    } while (0)

class ObjectTime
    :   public Object
{
    public:
        ObjectTime(const Object::CreateFlags &);
        ObjectTime(
            const Object::CreateFlags   &,
            const double                &
        );
        virtual ~ObjectTime();

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
        // static ObjectTimeSptr   now(const int32_t &a_is_prop);
        void            reset();
        double          getVal();

        operator    double();
        operator    int64_t();
        operator    int32_t();

        bool operator != (
            ObjectTime a_right
        );

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        ObjectDoubleSptr    m_val;

        void    do_init(const Object::CreateFlags &);
};

#endif

