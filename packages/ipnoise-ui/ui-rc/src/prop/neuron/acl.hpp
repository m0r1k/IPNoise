#include "class.hpp"
DEFINE_CLASS(PropNeuronAcl);

#ifndef PROP_NEURON_ACL_HPP
#define PROP_NEURON_ACL_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <map>

#include "prop.hpp"

using namespace std;

typedef map<PropSptr, PropSptr, PropCmp>    PropNeuronAclVals;
typedef PropNeuronAclVals::iterator         PropNeuronAclValsIt;
typedef PropNeuronAclVals::const_iterator   PropNeuronAclValsConstIt;

class PropNeuronAclIterator
{
    public:
        PropNeuronAclIterator();
        ~PropNeuronAclIterator();

        PropNeuronAclValsIt   m_it;

        bool operator != (
            const PropNeuronAclIterator &a_right
        ) const;
        void operator ++ (int32_t);
        PropNeuronAclValsIt operator -> ();
};

class PropNeuronAcl
    :   public Prop
{
    public:
        typedef PropNeuronAclIterator iterator;

        PropNeuronAcl();
        PropNeuronAcl(const map<PropSptr, PropSptr> &);
        PropNeuronAcl(
            const PropSptr  &a_key,
            const PropSptr  &a_val
        );
        ~PropNeuronAcl();

        virtual string toString()   const;
        virtual string serialize(
            const string &a_delim = ": "
        )  const;

        void        add(
            const PropSptr &,
            const PropSptr &
        );

        PropNeuronAcl::iterator   begin();
        PropNeuronAcl::iterator   end();
        PropNeuronAcl::iterator   find(const PropSptr &a_key);

        bool        has(const PropSptr &a_key);
        void        erase(const PropNeuronAcl::iterator &);
        void        erase(const PropSptr &a_key);
        operator    PropSptr() const;

        PropSptr & operator []  (const PropSptr &a_key);

    private:
        PropNeuronAclVals    m_vals;
};

typedef PropNeuronAcl::iterator PropNeuronAclIt;

#endif

