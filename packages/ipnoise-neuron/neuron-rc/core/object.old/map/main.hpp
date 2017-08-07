#include "class.hpp"
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectMapIterator);
DEFINE_CLASS(ObjectMapConstIterator);

#ifndef OBJECT_MAP_HPP
#define OBJECT_MAP_HPP

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
DEFINE_CLASS(ObjectString);

DEFINE_CLASS(CmpObject);

using namespace std;

#define _OBJECT_MAP(a_out, a_flags, ...)                    \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectMap,                                          \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_MAP(a_out, a_flags, ...)                      \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectMap,                                          \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_MAP(a_out, ...)                              \
    CREATE_OBJECT(a_out, ObjectMap, ##__VA_ARGS__)

#define PROP_MAP(a_out, ...)                                \
    CREATE_PROP(a_out, ObjectMap, ##__VA_ARGS__)

typedef map<ObjectSptr, ObjectSptr, CmpObject>  ObjectMapVals;
typedef ObjectMapVals::iterator                 ObjectMapValsIt;
typedef ObjectMapVals::const_iterator           ObjectMapValsConstIt;

class ObjectMapIterator
{
    public:
        ObjectMapIterator();
        virtual ~ObjectMapIterator();

        ObjectMapValsIt   m_it;

        // !=
        bool operator != (
            const ObjectMapIterator &a_right
        ) const;

        bool operator != (
            const ObjectMapConstIterator &a_right
        ) const;

        // ==
        bool operator == (
            const ObjectMapIterator &a_right
        ) const;

        bool operator == (
            const ObjectMapConstIterator &a_right
        ) const;

        // =
        ObjectMapIterator & operator = (
            const ObjectMapIterator &a_it
        );

        // ++
        void operator ++ (int32_t);

        // ->
        ObjectMapValsIt operator -> () const;
};

class ObjectMapConstIterator
{
    public:
        ObjectMapConstIterator();
        virtual ~ObjectMapConstIterator();

        ObjectMapValsConstIt  m_it;

        // !=
        bool operator != (
            const ObjectMapConstIterator &a_right
        ) const;

        bool operator != (
            const ObjectMapIterator &a_right
        ) const;

        // =
        ObjectMapConstIterator & operator = (
            const ObjectMapConstIterator &
        );

        // ++
        void operator ++ (int32_t);

        // ->
        ObjectMapValsConstIt operator -> () const;
};

class ObjectMap
    :   public Object
{
    public:
        typedef ObjectMapIterator         iterator;
        typedef ObjectMapConstIterator    const_iterator;

        ObjectMap(const Object::CreateFlags     &);
        ObjectMap(
            const Object::CreateFlags           &,
            ObjectMapSptr
        );
        ObjectMap(
            const Object::CreateFlags           &,
            const map<ObjectSptr, ObjectSptr>   &
        );
        ObjectMap(
            const Object::CreateFlags           &,
            const ObjectSptr                    &a_key,
            const ObjectSptr                    &a_val
        );
        virtual ~ObjectMap();

        // interface
        virtual string              getType();
        virtual void                do_init_props();
        virtual void                do_init_api();
        virtual ObjectStringSptr    serialize(
            const string &a_delim = DEFAULT_DELIMITER
        );
/*
        virtual void    toBSON(
            mongo::BSONObjBuilder   &a_builder,
            const string            &a_field_name
        );
        virtual void    toBSON(
            mongo::BSONArrayBuilder &a_builder
        );
        virtual SV *    toPerl();
*/

        // generic
        void    add(ObjectSptr,         ObjectSptr);
        void    add(const string &,     const string &);
        void    add(const string &,     const ObjectSptr);
        void    add(ObjectSptr,         const string &);
        void    add(const string &,     const int32_t &);
        void    add(ObjectSptr,         const int32_t &);
        void    add(const int32_t &,    ObjectSptr);
        void    add(const string &,     const double &);
        void    add(ObjectSptr,         const double &);

        ObjectMap::iterator       begin()     noexcept;
        ObjectMap::const_iterator cbegin()    noexcept;
        ObjectMap::iterator       end()       noexcept;
        ObjectMap::const_iterator cend()      noexcept;

        ObjectMap::iterator       find(const ObjectSptr &a_key);
        ObjectMap::iterator       find(const string   &a_key);

        bool        has(const ObjectSptr &a_key);
        bool        has(const string &a_key);
        uint32_t    size();
        void        erase(const ObjectMap::iterator &);
        void        erase(ObjectSptr);
        void        erase(const string &);
        void        clear();
        ObjectSptr  get(ObjectSptr);
        ObjectSptr  get(const string &);
        bool        empty();

        ObjectSptr & operator []  (const ObjectSptr &a_key);
        ObjectSptr & operator []  (const string &a_key);

        ObjectSptr    m_npos;

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        ObjectMapVals    m_vals;

        void    do_init(const Object::CreateFlags &);
};

typedef ObjectMap::iterator       ObjectMapIt;
typedef ObjectMap::const_iterator ObjectMapConstIt;

#endif

