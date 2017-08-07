#include "class.hpp"
DEFINE_CLASS(PropVector);
DEFINE_CLASS(PropVectorIterator);
DEFINE_CLASS(PropVectorConstIterator);

#ifndef PROP_VECTOR_HPP
#define PROP_VECTOR_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <vector>

#include "prop.hpp"

using namespace std;

typedef vector<PropSptr>                PropVectorVals;
typedef PropVectorVals::iterator        PropVectorValsIt;
typedef PropVectorVals::const_iterator  PropVectorValsConstIt;

#define PROP_VECTOR(x)   \
    PropVectorSptr(new PropVector(x))

class PropVectorIterator
{
    public:
        PropVectorIterator();
        ~PropVectorIterator();

        PropVectorValsIt   m_it;

        // !=
        bool operator != (
            const PropVectorIterator &a_right
        ) const;

        bool operator != (
            const PropVectorConstIterator &a_right
        ) const;

        // ==
        bool operator == (
            const PropVectorIterator &a_right
        ) const;

        bool operator == (
            const PropVectorConstIterator &a_right
        ) const;

        void                operator    ++ (int32_t);
        PropSptr            operator    *  ();
        PropVectorValsIt    operator    -> ();
};

class PropVectorConstIterator
{
    public:
        PropVectorConstIterator();
        ~PropVectorConstIterator();

        PropVectorValsConstIt   m_it;

        bool operator != (
            const PropVectorConstIterator &a_right
        ) const;

        bool operator != (
            const PropVectorIterator &a_right
        ) const;

        PropVectorConstIterator & operator = (const PropVectorIterator &a_it){
            m_it = a_it.m_it;
            return *this;
        };

        void                    operator ++ (int32_t);
        PropSptr                operator *  ();
        PropVectorValsConstIt   operator -> ();
};

class PropVector
    :   public Prop
{
    public:
        typedef PropVectorIterator         iterator;
        typedef PropVectorConstIterator    const_iterator;

        PropVector(const vector<PropSptr> &);
        PropVector();
        ~PropVector();

        virtual string toString()   const;
        virtual string serialize(
            const string &a_delim = ": "
        )  const;

        PropVector::iterator   begin();
        PropVector::iterator   end();

        PropSptr at(const uint32_t &);
        PropSptr get(const uint32_t &);

        void        add(const PropSptr &);
        PropVector::iterator    erase(
            PropVector::iterator a_pos
        );
        void        push_back(const PropSptr &);
        void        pop_back();
        size_t      size() const;
        void        sort();

        PropSptr &  operator [] (const uint32_t &a_pos);
        operator    PropSptr() const;

    private:
        PropVectorVals    m_vals;
};

typedef PropVector::iterator       PropVectorIt;
typedef PropVector::const_iterator PropVectorConstIt;

#endif

