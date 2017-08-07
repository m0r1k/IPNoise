#include "class.hpp"
DEFINE_CLASS(Prop);

#ifndef PROP_HPP
#define PROP_HPP

#include <stdint.h>

#include <memory>
#include <string>

using namespace std;

class Prop
{
    public:
        // donnot forget update Template::parseNeuronProps
        enum PropType {
            PROP_TYPE_ERROR             = 0,
            PROP_TYPE_INT,
            PROP_TYPE_STRING,
            PROP_TYPE_VECTOR,
            PROP_TYPE_MAP,
            PROP_TYPE_TIME,
            PROP_TYPE_DOUBLE,

            PROP_TYPE_NEURON            = 100,
            PROP_TYPE_NEURON_ID,
            PROP_TYPE_NEURON_TYPE,
            PROP_TYPE_NEURON_GROUP,

            PROP_TYPE_NEURON_LINK       = 150,
            PROP_TYPE_NEURON_LINK_EVENT
        };

        Prop(const PropType &);
        virtual ~Prop();

        virtual string toString()   const = 0;
        virtual string serialize(
            const string &a_delim = ": "
        ) const = 0;

        PropType    getType()   const;
        void        setDirty(
            const int32_t &
        );
        int32_t     isDirty()   const;
        int32_t     toInt()     const;

    protected:
        int32_t     m_is_dirty;
        string      m_group;      // who can edit|view this prop? (group name);
        PropType    m_prop_type;
};

class PropCmp
{
    public:
        bool operator () (
            PropSptr const a,
            PropSptr const b)
        {
            return a->toString() < b->toString();
        }
};

#endif

