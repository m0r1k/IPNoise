#include "class.hpp"
DEFINE_CLASS(Object);

#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <stdint.h>

#include <mongo/client/dbclient.h>

#include <mutex>
#include <memory>
#include <string>

using namespace std;

#define DEFAULT_DELIMITER ": "

class Object
{
    public:
        // don't forget update Template::parseObjects
        enum ObjectType {
            OBJECT_TYPE_ERROR             = 0,
            OBJECT_TYPE_INT32,
            OBJECT_TYPE_UINT32,
            OBJECT_TYPE_INT64,
            OBJECT_TYPE_UINT64,
            OBJECT_TYPE_STRING,
            OBJECT_TYPE_VECTOR,
            OBJECT_TYPE_MAP,
            OBJECT_TYPE_ACL,
            OBJECT_TYPE_TIME,
            OBJECT_TYPE_DOUBLE,
            OBJECT_TYPE_VEC3,
            OBJECT_TYPE_THREAD,

            OBJECT_TYPE_OBJECT            = 100,
            OBJECT_TYPE_OBJECT_ID,
            OBJECT_TYPE_OBJECT_TYPE,
            OBJECT_TYPE_OBJECT_GROUP,

            OBJECT_TYPE_OBJECT_LINK       = 150,
            OBJECT_TYPE_OBJECT_LINK_INFO,
            OBJECT_TYPE_OBJECT_LINK_EVENT
        };

        Object(const ObjectType &);
        virtual ~Object();

        virtual string  toString();
        virtual string  serialize(
            const string &a_delim = DEFAULT_DELIMITER
        ) = 0;

        virtual void    toBSON(
            mongo::BSONObjBuilder   &a_builder,
            const string            &a_field_name
        ) = 0;
        virtual void    toBSON(
            mongo::BSONArrayBuilder &a_builder
        ) = 0;

        ObjectType    getType()   const;
        void        setDirty(
            const int32_t &
        );
        int32_t     isDirty()   const;

        int32_t     toInt();
        double      toDouble();

        recursive_mutex     m_mutex;

    protected:
        int32_t             m_is_dirty;
        // who can edit|view this object? (group name);
        string              m_group;
        ObjectType            m_object_type;
};

class CmpObject
{
    public:
        bool operator () (
            ObjectSptr const a,
            ObjectSptr const b) const
        {
            return a->toString() < b->toString();
        }
};

#endif

