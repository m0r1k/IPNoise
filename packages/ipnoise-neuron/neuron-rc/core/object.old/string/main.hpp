#include "class.hpp"
DEFINE_CLASS(ObjectString);

#ifndef OBJECT_STRING_HPP
#define OBJECT_STRING_HPP

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
DEFINE_CLASS(ObjectVector);

using namespace std;

#define _OBJECT_STRING(a_out, a_flags, ...)                 \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectString,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_STRING(a_out, a_flags, ...)                   \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectString,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_STRING(a_out, ...)                           \
    CREATE_OBJECT(a_out, ObjectString, ##__VA_ARGS__ )

#define PROP_STRING(a_out, ...)                             \
    CREATE_PROP(a_out, ObjectString, ##__VA_ARGS__ )

class ObjectString
    :   public Object
{
    public:
        ObjectString(const Object::CreateFlags &);
        ObjectString(
            const Object::CreateFlags   &,
            const string                &
        );
        ObjectString(
            const Object::CreateFlags   &,
            const char
        );
        ObjectString(
            const Object::CreateFlags   &,
            const char *
        );
        ObjectString(
            const Object::CreateFlags   &,
            ObjectStringSptr
        );

        virtual ~ObjectString();

        // interface
        virtual string  getType();
        virtual void    do_init_props();
        virtual void    do_init_api();
        virtual string  toString();
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
        bool                empty();
        uint64_t            size();
        void                assign(const string &);
        const char *        c_str() noexcept;
        char                at(const int32_t   &);
        void                toLower();
        void                add(const string   &);
        void                add(const char     &);
        void                add(
            const char      *a_buff,
            const uint64_t  &a_len
        );
        void                add(ObjectStringSptr);
        string              getVal();
        string              hexdump();
/*
        ObjectVectorSptr    split(
            const char  &a_delim,
            const bool  &a_delim_can_repeat = 1
        );
*/
        operator            string();

        // static
        static ObjectVectorSptr split(
            const char          *a_val,
            const char          &a_delim,
            const bool          &a_delim_can_repeat = 1
        );

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        string  m_val;

        void    do_init(const Object::CreateFlags &);
};

#endif

