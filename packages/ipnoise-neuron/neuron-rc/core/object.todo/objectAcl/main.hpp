#include "class.hpp"
DEFINE_CLASS(ObjectAcl);

#ifndef OBJECT_ACL_HPP
#define OBJECT_ACL_HPP

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
DEFINE_CLASS(ObjectMap);

using namespace std;

#define _OBJECT_ACL(a_out, a_flags, ...)                    \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectAcl,                                          \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_ACL(a_out, a_flags, ...)                      \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectAcl,                                          \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_ACL(a_out, ...)                              \
    CREATE_OBJECT(a_out, ObjectAcl, ##__VA_ARGS__)

#define PROP_ACL(a_out, ...)                                \
    CREATE_PROP(a_out, ObjectAcl, ##__VA_ARGS__)

class ObjectAclIterator
{
    public:
        ObjectAclIterator();
        virtual ~ObjectAclIterator();

        ObjectMapIt   m_it;

        bool operator != (
            const ObjectAclIterator &a_right
        ) const;
        void operator ++ (int32_t);
        ObjectMapIt operator -> ();
};

class ObjectAcl
    :   public Object
{
    public:
        typedef ObjectAclIterator iterator;

        ObjectAcl(const Object::CreateFlags &);
        ObjectAcl(
            const Object::CreateFlags           &,
            ObjectAclSptr
        );
        ObjectAcl(
            const Object::CreateFlags           &,
            const map<ObjectSptr, ObjectSptr>   &
        );
        ObjectAcl(
            const Object::CreateFlags           &,
            const ObjectSptr                    &a_key,
            const ObjectSptr                    &a_val
        );
        ~ObjectAcl();

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
        void        add(
            const ObjectSptr &,
            const ObjectSptr &
        );

        ObjectAcl::iterator   begin();
        ObjectAcl::iterator   end();
        ObjectAcl::iterator   find(const ObjectSptr &a_key);

        bool        has(const ObjectSptr &a_key);
        void        erase(const ObjectAcl::iterator &);
        void        erase(const ObjectSptr &a_key);

        ObjectSptr & operator []  (const ObjectSptr &a_key);

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        ObjectMapSptr m_val;

        void    do_init(const Object::CreateFlags &);
};

typedef ObjectAcl::iterator ObjectAclIt;

#endif

