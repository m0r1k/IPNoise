#include "class.hpp"
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectMapIterator);
DEFINE_CLASS(ObjectMapConstIterator);

#ifndef OBJECT_MAP_HPP
#define OBJECT_MAP_HPP

#include <string>
#include <map>

#include "module.hpp"

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectString);

DEFINE_CLASS(CmpObject);

using namespace std;

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

        ObjectMap(const CreateFlags &a_create_flags);
        virtual ~ObjectMap();

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
        void    add(ObjectSptr,         ObjectSptr);
        void    add(const string &,     const string &);
        void    add(const string &,     ObjectSptr);
        void    add(const char *,       ObjectSptr);
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
        ObjectMap::iterator       find(const int32_t  &a_key);

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
        static  string      s_getType();
        static  int32_t     s_init(EngineInfo *);
        static  int32_t     s_shutdown();
        static  ObjectSptr  s_objectCreate(
            const CreateFlags   &a_create_flags
        );
        static  void        s_getTests(Tests &);

    private:
        ObjectMapVals    m_vals;
};

typedef ObjectMap::iterator       ObjectMapIt;
typedef ObjectMap::const_iterator ObjectMapConstIt;

#endif

