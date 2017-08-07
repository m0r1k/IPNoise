#include "class.hpp"
DEFINE_CLASS(Object);

#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <string>

#include <stdint.h>
#include <stdio.h>

#include <string>

#include "object.hpp"

DEFINE_CLASS(Object);

using namespace std;

#define OBJECT(args...)   \
    ObjectSptr(new Object(args))

class Object
    :   public Object
{
    public:
        Object();
        Object(const Object &);
        Object(ObjectSptr &);
        virtual ~Object();

        virtual string  toString();
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

        ObjectSptr  getVal()    const;

        operator    ObjectSptr()  const;

    private:
        ObjectSptr  m_val;
};

#endif

