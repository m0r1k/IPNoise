#include "class.hpp"
DEFINE_CLASS(ObjectTiming);

#ifndef OBJECT_TIMING_HPP
#define OBJECT_TIMING_HPP

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
DEFINE_CLASS(ObjectAction);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectTime);

extern int32_t g_debug_level;

using namespace std;

#define _OBJECT_TIMING(a_out, a_flags, ...)                 \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectTiming,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_TIMING(a_out, a_flags, ...)                   \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectTiming,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_TIMING(a_out, ...)                           \
    CREATE_OBJECT(a_out, ObjectTiming, ##__VA_ARGS__)

#define PROP_TIMING(a_out, ...)                             \
    CREATE_PROP(a_out, ObjectTiming, ##__VA_ARGS__)

#define PTIMING(a_time_max, a_fmt, ...)                     \
    ObjectTimingSptr    COMBINE(timing, __LINE__);          \
    do {                                                    \
        char buffer[512] = { 0x00 };                        \
        snprintf(buffer, sizeof(buffer),                    \
            a_fmt, ##__VA_ARGS__                            \
        );                                                  \
        PROP_TIMING(                                        \
            COMBINE(timing, __LINE__),                      \
            a_time_max,                                     \
            buffer,                                         \
            __FILE__,                                       \
            __LINE__,                                       \
            __FUNCTION__                                    \
        );                                                  \
    } while (0);

class ObjectTiming
    :   public Object
{
    public:
        ObjectTiming(
            const Object::CreateFlags &,
            const double              &a_time_max_ms = 1e6f,
            const string              &a_msg         = "",
            const string              &a_file        = "",
            const int32_t             &a_line        = 0,
            const string              &a_function    = ""
        );
        virtual ~ObjectTiming();

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        //virtual string  toString();
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
        //ObjectMapSptr   getVal();
        void            dumpToStderr();

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        double      m_time_init;
        double      m_time_end;
        double      m_time_delta;
        double      m_time_max;
        string      m_msg;
        string      m_file;
        int32_t     m_line;
        string      m_function;

        void    do_init(
            const Object::CreateFlags   &a_flags,
            const double                &a_time_max,
            const string                &a_msg,
            const string                &a_file,
            const int32_t               &a_line,
            const string                &a_function
        );
};

#endif

