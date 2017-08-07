#include "class.hpp"
DEFINE_CLASS(ObjectUnique);
DEFINE_CLASS(ObjectUniqueIterator);
DEFINE_CLASS(ObjectUniqueConstIterator);

#ifndef OBJECT_UNIQUE_HPP
#define OBJECT_UNIQUE_HPP

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
DEFINE_CLASS(ObjectVector);

using namespace std;

#define _OBJECT_UNIQUE(a_out, a_flags, ...)                 \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectUnique,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_UNIQUE(a_out, a_flags, ...)                   \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectUnique,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_UNIQUE(a_out, ...)                           \
    CREATE_OBJECT(a_out, ObjectUnique, ##__VA_ARGS__)

#define PROP_UNIQUE(a_out, ...)                             \
    CREATE_PROP(a_out, ObjectUnique, ##__VA_ARGS__)

class ObjectUniqueIterator
{
    public:
        ObjectUniqueIterator();
        virtual ~ObjectUniqueIterator();

        ObjectMapIt   m_it;

        // !=
        bool operator != (
            const ObjectUniqueIterator &a_right
        ) const;

        bool operator != (
            const ObjectUniqueConstIterator &a_right
        ) const;

        // ==
        bool operator == (
            const ObjectUniqueIterator &a_right
        ) const;

        bool operator == (
            const ObjectUniqueConstIterator &a_right
        ) const;

        // =
        ObjectUniqueIterator & operator = (
            const ObjectUniqueIterator &a_it
        );

        // ++
        void operator ++ (int32_t);

        // *
        ObjectSptr operator * () const;

        // ->
        ObjectMapIt operator -> () const;
};

class ObjectUniqueConstIterator
{
    public:
        ObjectUniqueConstIterator();
        virtual ~ObjectUniqueConstIterator();

        ObjectMapConstIt  m_it;

        // !=
        bool operator != (
            const ObjectUniqueConstIterator &a_right
        ) const;

        bool operator != (
            const ObjectUniqueIterator &a_right
        ) const;

        // =
        ObjectUniqueConstIterator & operator = (
            const ObjectUniqueConstIterator &
        );

        // ++
        void operator ++ (int32_t);

        // *
        ObjectSptr operator * () const;

        // ->
        ObjectMapConstIt operator -> () const;
};

class ObjectUnique
    :   public Object
{
    public:
        typedef ObjectUniqueIterator         iterator;
        typedef ObjectUniqueConstIterator    const_iterator;

        ObjectUnique(const Object::CreateFlags &);
        ObjectUnique(
            const Object::CreateFlags   &a_flags,
            ObjectVectorSptr            a_vals
        );
        virtual ~ObjectUnique();

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
        void    add(ObjectSptr);
        void    add(const string    &);
        void    add(const int32_t   &);

        ObjectUnique::iterator       begin()     noexcept;
        ObjectUnique::const_iterator cbegin()    noexcept;
        ObjectUnique::iterator       end()       noexcept;
        ObjectUnique::const_iterator cend()      noexcept;

        ObjectUnique::iterator       find(const ObjectSptr &a_key);
        ObjectUnique::iterator       find(const string   &a_key);

        bool        has(const ObjectSptr &a_key);
        bool        has(const string &a_key);
        uint32_t    size();
        void        erase(const ObjectUnique::iterator &);
        void        erase(ObjectSptr);
        void        erase(const string &);
        void        erase(const int32_t &);
        void        clear();
        void        getVals(ObjectVectorSptr);

        ObjectSptr  get(ObjectSptr);
        ObjectSptr  get(const string &);
        bool        empty();

        ObjectSptr  m_npos;

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        ObjectMapSptr   m_val;

        void    do_init(
            const Object::CreateFlags   &,
            ObjectVectorSptr            a_vals = ObjectVectorSptr()
        );
};

typedef ObjectUnique::iterator       ObjectUniqueIt;
typedef ObjectUnique::const_iterator ObjectUniqueConstIt;

#endif

