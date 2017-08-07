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

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);
DEFINE_CLASS(ObjectVector);

using namespace std;

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

        ObjectUnique(const CreateFlags &a_create_flags);
        virtual ~ObjectUnique();

        // interface
        virtual string      getType();
        virtual int32_t     do_init_as_prop(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual int32_t     do_init_as_object(
            const char      *a_data,
            const uint64_t  &a_data_size
        );
        virtual ObjectSptr  copy();
        virtual void        getAllProps(ObjectMapSptr);
        virtual ObjectStringSptr toString();
        virtual ObjectStringSptr serializeAsProp(
            const char  *a_delim = DEFAULT_DELIMITER
        );
        virtual int32_t     loadAsProp(const char *a_dir);
        virtual int32_t     saveAsProp(const char *a_dir);

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
        static int32_t s_test_save(const char *a_tmp_path);
        static int32_t s_test_load(const char *a_tmp_path);

        // module
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static  void        s_getTests(Tests &);

    private:
        ObjectMapSptr   m_vals;
};

typedef ObjectUnique::iterator       ObjectUniqueIt;
typedef ObjectUnique::const_iterator ObjectUniqueConstIt;

#endif

