#include "class.hpp"
DEFINE_CLASS(ObjectBin);

#ifndef OBJECT_BIN_HPP
#define OBJECT_BIN_HPP

#include <stdint.h>
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
DEFINE_CLASS(ObjectString);

using namespace std;

#define _OBJECT_BIN(a_out, a_flags, ...)                    \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectBin,                                          \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_BIN(a_out, a_flags, ...)                      \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectBin,                                          \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_BIN(a_out, ...)                              \
    CREATE_OBJECT(a_out, ObjectBin, ##__VA_ARGS__ )

#define PROP_BIN(a_out, ...)                                \
    CREATE_PROP(a_out, ObjectBin, ##__VA_ARGS__ )

class ObjectBin
    :   public Object
{
    public:
        ObjectBin(const Object::CreateFlags &);
        ObjectBin(
            const Object::CreateFlags   &,
            const string                &
        );
        virtual ~ObjectBin();

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
        ObjectStringSptr    getVal();
        bool                empty();
        uint64_t            size();
        const char *        c_str() noexcept;
        char                at(const int32_t &);
        void                add(const char &a_val);
        void                add(
            const char      *a_buff,
            const uint64_t  &a_len
        );
        void                add(ObjectStringSptr a_val);
        string              hexdump();

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        ObjectStringSptr    m_val;

        void    do_init(
            const Object::CreateFlags   &,
            const string                &a_val = ""
        );
};

#endif

