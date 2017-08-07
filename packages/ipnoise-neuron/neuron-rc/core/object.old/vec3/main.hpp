#include "class.hpp"
DEFINE_CLASS(ObjectVec3);

#ifndef OBJECT_VEC3_HPP
#define OBJECT_VEC3_HPP

#include <string>
#include <mutex>

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sys/time.h>

DEFINE_CLASS(Object);
DEFINE_CLASS(ObjectAction);
DEFINE_CLASS(ObjectMap);
DEFINE_CLASS(ObjectVector);
DEFINE_CLASS(ObjectDouble);

using namespace std;

#define _OBJECT_VEC3(a_out, a_flags, ...)                   \
    _CREATE_OBJECT(                                         \
        a_out,                                              \
        ObjectVec3,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define _PROP_VEC3(a_out, a_flags, ...)                     \
    _CREATE_PROP(                                           \
        a_out,                                              \
        ObjectVec3,                                         \
        a_flags,                                            \
        ##__VA_ARGS__                                       \
    )

#define OBJECT_VEC3(a_out, ...)                             \
    CREATE_OBJECT(a_out, ObjectVec3, ##__VA_ARGS__)

#define PROP_VEC3(a_out, ...)                               \
    CREATE_PROP(a_out, ObjectVec3, ##__VA_ARGS__)

class ObjectVec3
    :   public Object
{
    public:
        ObjectVec3(const Object::CreateFlags &);
        ObjectVec3(
            const Object::CreateFlags   &,
            ObjectVec3
        );
        ObjectVec3(
            const Object::CreateFlags   &,
            const double                &a_val0,
            const double                &a_val1,
            const double                &a_val2
        );
        virtual ~ObjectVec3();

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

        // ------- 0, X, Pitch -------
        // get
        double  get0();
        double  getX();
        double  getPitch();
        // set
        void    set0(const double &);
        void    setX(const double &);
        void    setPitch(const double &);
        // add
        void    add0(const double &);
        void    addX(const double &);
        void    addPitch(const double &);
        // sub
        void    sub0(const double &);
        void    subX(const double &);
        void    subPitch(const double &);

        // ------- 1, Y, Yaw -------
        // get
        double  get1();
        double  getY();
        double  getYaw();
        // set
        void    set1(const double &);
        void    setY(const double &);
        void    setYaw(const double &);
        // add
        void    add1(const double &);
        void    addY(const double &);
        void    addYaw(const double &);
        // sub
        void    sub1(const double &);
        void    subY(const double &);
        void    subYaw(const double &);

        // ------- 2, Z, Roll -------
        // get
        double  get2();
        double  getZ();
        double  getRoll();
        // set
        void    set2(const double &);
        void    setZ(const double &);
        void    setRoll(const double &);
        // add
        void    add2(const double &);
        void    addZ(const double &);
        void    addRoll(const double &);
        // sub
        void    sub2(const double &);
        void    subZ(const double &);
        void    subRoll(const double &);

        // static
        static double           magnitude(ObjectVec3Sptr);
        static ObjectVec3Sptr     cross(
            ObjectVec3Sptr a_pos_view,
            ObjectVec3Sptr a_pos,
            ObjectVec3Sptr a_pos_up
        );
        static ObjectVec3Sptr     normalize(ObjectVec3Sptr);
        static double           normalize_radians(const double &);

        // operators

/*
        ObjectVec3 & operator =  (
            ObjectVec3 &a_right
        );

        ObjectVec3 & operator *  (
            ObjectDouble &a_right
        );

        ObjectVec3 & operator -= (
            ObjectVec3 &a_right
        );

        ObjectVec3 & operator += (
            ObjectVec3 &a_right
        );

        const ObjectVec3 operator - (
            ObjectVec3 &a_right
        );

        const ObjectVec3 operator + (
            ObjectVec3 &a_right
        );
*/

        bool operator != (
            ObjectVec3 &a_right
        );

        // static

        // module
        static string       _getType();
        static string       _getTypePerl();
        static int32_t      _init();
        static void         _destroy();
        static ObjectSptr   _object_create();

    private:
        ObjectVectorSptr  m_val;

        void    do_init(const Object::CreateFlags &);
};

#endif

