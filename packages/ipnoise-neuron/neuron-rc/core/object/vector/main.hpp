#include "class.hpp"
DEFINE_CLASS(ObjectVector);
DEFINE_CLASS(ObjectVectorIterator);
DEFINE_CLASS(ObjectVectorConstIterator);

#ifndef OBJECT_VECTOR_HPP
#define OBJECT_VECTOR_HPP

#include <string>
#include <vector>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);

using namespace std;

typedef vector<ObjectSptr>                ObjectVectorVals;
typedef ObjectVectorVals::iterator        ObjectVectorValsIt;
typedef ObjectVectorVals::const_iterator  ObjectVectorValsConstIt;

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

        ObjectVector(const CreateFlags &a_create_flags);
        virtual ~ObjectVector();

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


        // static

        // module
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static  void        s_getTests(Tests &);

    private:
        ObjectVectorVals    m_vals;
};

typedef ObjectVector::iterator       ObjectVectorIt;
typedef ObjectVector::const_iterator ObjectVectorConstIt;

#endif

