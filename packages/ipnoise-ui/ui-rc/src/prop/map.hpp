#include "class.hpp"
DEFINE_CLASS(PropMap);
DEFINE_CLASS(PropMapIterator);
DEFINE_CLASS(PropMapConstIterator);

#ifndef PROP_MAP_HPP
#define PROP_MAP_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <map>

#include "prop.hpp"

using namespace std;

typedef map<PropSptr, PropSptr, PropCmp>    PropMapVals;
typedef PropMapVals::iterator               PropMapValsIt;
typedef PropMapVals::const_iterator         PropMapValsConstIt;

#define PROP_MAP(args...) \
    PropMapSptr(new PropMap(args))

class PropMapIterator
{
    public:
        PropMapIterator();
        ~PropMapIterator();

        PropMapValsIt   m_it;

        // !=
        bool operator != (
            const PropMapIterator &a_right
        ) const;

        bool operator != (
            const PropMapConstIterator &a_right
        ) const;

        // ==
        bool operator == (
            const PropMapIterator &a_right
        ) const;

        bool operator == (
            const PropMapConstIterator &a_right
        ) const;

        // ++
        void operator ++ (int32_t);

        // ->
        PropMapValsIt operator -> ();
};

class PropMapConstIterator
{
    public:
        PropMapConstIterator();
        ~PropMapConstIterator();

        PropMapValsConstIt   m_it;

        bool operator != (
            const PropMapConstIterator &a_right
        ) const;

        bool operator != (
            const PropMapIterator &a_right
        ) const;

        PropMapConstIterator & operator = (const PropMapIterator &a_it){
            m_it = a_it.m_it;
            return *this;
        };

        void operator ++ (int32_t);
        PropMapValsConstIt operator -> ();
};

class PropMap
    :   public Prop
{
    public:
        typedef PropMapIterator         iterator;
        typedef PropMapConstIterator    const_iterator;

        PropMap();
        PropMap(const map<PropSptr, PropSptr> &);
        PropMap(
            const PropSptr  &a_key,
            const PropSptr  &a_val
        );
        ~PropMap();

        virtual string toString()   const;
        virtual string serialize(
            const string &a_delim = ": "
        )   const;

        void        add(
            const PropSptr &a_key,
            const PropSptr &a_val
        );

        void        add(
            const string &a_key,
            const string &a_val
        );

        void        add(
            const string    &a_key,
            const PropSptr  &a_val
        );

        PropMap::iterator   begin();
        PropMap::iterator   end();
        PropMap::iterator   find(const PropSptr &a_key);
        PropMap::iterator   find(const string   &a_key);

//        PropMap::const_iterator   begin()                       const;
//        PropMap::const_iterator   end()                         const;
//        PropMap::const_iterator   find(const PropSptr &a_key)   const;

        bool        has(const PropSptr &a_key);
        bool        has(const string &a_key);
        void        erase(const PropMap::iterator &);
        void        erase(const PropSptr &a_key);
        PropSptr    get(const PropSptr &);
        PropSptr    get(const string &);
        bool        empty() const;

        operator    PropSptr() const;

        PropSptr & operator []  (const PropSptr &a_key);
        PropSptr & operator []  (const string &a_key);

        PropSptr    m_npos;

    private:
        PropMapVals    m_vals;
};

typedef PropMap::iterator       PropMapIt;
typedef PropMap::const_iterator PropMapConstIt;

#endif

