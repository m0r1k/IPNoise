#include "class.hpp"
DEFINE_CLASS(ObjectVector);
DEFINE_CLASS(ObjectVectorIterator);
DEFINE_CLASS(ObjectVectorConstIterator);

#ifndef OBJECT_VECTOR_HPP
#define OBJECT_VECTOR_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <vector>

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

using namespace std;

typedef vector<ObjectSptr>                ObjectVectorVals;
typedef ObjectVectorVals::iterator        ObjectVectorValsIt;
typedef ObjectVectorVals::const_iterator  ObjectVectorValsConstIt;

#define _OBJECT_VECTOR(a_out, a_flags, ...)                 \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectVector,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_VECTOR(a_out, a_flags, ...)                   \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectVector,                                       \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_VECTOR(a_out, ...)                           \
    CREATE_OBJECT(a_out, ObjectVector, ##__VA_ARGS__)

#define PROP_VECTOR(a_out, ...)                             \
    CREATE_PROP(a_out, ObjectVector, ##__VA_ARGS__)

class ObjectVectorIterator
{
    public:
        ObjectVectorIterator();
        virtual ~ObjectVectorIterator();

        ObjectVectorValsIt   m_it;

        // !=
        bool operator != (
            const ObjectVectorIterator &a_right
        ) const;

        bool operator != (
            const ObjectVectorConstIterator &a_right
        ) const;

        // ==
        bool operator == (
            const ObjectVectorIterator &a_right
        ) const;

        bool operator == (
            const ObjectVectorConstIterator &a_right
        ) const;

        // =
        ObjectVectorIterator & operator = (
            const ObjectVectorIterator &a_it
        );

        // ++
        void operator ++ (int32_t);

        // *
        ObjectSptr operator * () const;

        // ->
        ObjectVectorValsIt operator -> () const;
};

class ObjectVectorConstIterator
{
    public:
        ObjectVectorConstIterator();
        ~ObjectVectorConstIterator();

        ObjectVectorValsConstIt   m_it;

        // !=
        bool operator != (
            const ObjectVectorConstIterator &a_right
        ) const;

        bool operator != (
            const ObjectVectorIterator &a_right
        ) const;

        // =
        ObjectVectorConstIterator & operator = (
            const ObjectVectorConstIterator &
        );

        // ++
        void operator ++ (int32_t);

        // *
        ObjectSptr operator * () const;

        // ->
        ObjectVectorValsConstIt operator -> () const;
};

class ObjectVector
    :   public Object
{
    public:
        typedef ObjectVectorIterator         iterator;
        typedef ObjectVectorConstIterator    const_iterator;

        ObjectVector(const Object::CreateFlags &);
        ObjectVector(
            const Object::CreateFlags   &,
            ObjectVectorSptr
        );
        ObjectVector(
            const Object::CreateFlags   &,
            const vector<ObjectSptr>    &
        );
        ObjectVector(
            const Object::CreateFlags   &a_flags,
            const char                  *a_val,
            const char                  &a_delim,
            const bool                  &a_delim_can_repeat = 1
        );
        ~ObjectVector();

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
        ObjectVector::iterator        begin()     noexcept;
        ObjectVector::iterator        cbegin()    noexcept;
        ObjectVector::iterator        end()       noexcept;
        ObjectVector::iterator        cend()      noexcept;

        ObjectSptr at(const uint32_t &);
        ObjectSptr get(const uint32_t &);

        ObjectVector::iterator    erase(
            ObjectVector::iterator a_pos
        );

        void        add(ObjectSptr);
        void        add(const int32_t &);
        void        add(const string &);
        void        push_back(ObjectSptr);
        ObjectSptr  back();
        ObjectSptr  pop_back();
        size_t      size();
        bool        empty();
        void        sort();
        void        clear();

        ObjectSptr &  operator [] (const uint32_t &a_pos);

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        ObjectVectorVals    m_vals;

        void    do_init(const Object::CreateFlags &);
};

typedef ObjectVector::iterator       ObjectVectorIt;
typedef ObjectVector::const_iterator ObjectVectorConstIt;

#endif

