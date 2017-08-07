#include <math.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/map/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/double/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/vec3/main.hpp"

ObjectVec3::ObjectVec3(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    ObjectDoubleSptr    val0;
    ObjectDoubleSptr    val1;
    ObjectDoubleSptr    val2;

    do_init(a_flags);

    PROP_VECTOR(m_val);

    PROP_DOUBLE(val0, 0.0f);
    PROP_DOUBLE(val1, 0.0f);
    PROP_DOUBLE(val2, 0.0f);

    m_val->add(val0);
    m_val->add(val1);
    m_val->add(val2);
}

ObjectVec3::ObjectVec3(
    const Object::CreateFlags   &a_flags,
    const double                &a_val0,
    const double                &a_val1,
    const double                &a_val2)
    :   Object(a_flags)
{
    ObjectDoubleSptr    val0;
    ObjectDoubleSptr    val1;
    ObjectDoubleSptr    val2;

    do_init(a_flags);

    PROP_VECTOR(m_val);

    PROP_DOUBLE(val0, a_val0);
    PROP_DOUBLE(val1, a_val1);
    PROP_DOUBLE(val2, a_val2);

    m_val->add(val0);
    m_val->add(val1);
    m_val->add(val2);

    setDirty(1);
}

ObjectVec3::ObjectVec3(
    const Object::CreateFlags   &a_flags,
    ObjectVec3                  a_val)
    :   Object(a_flags)
{
    do_init(a_flags);

    lock_guard<recursive_mutex> guard(a_val.m_mutex);

    ObjectDoubleSptr  val0;
    ObjectDoubleSptr  val1;
    ObjectDoubleSptr  val2;

    PROP_VECTOR(m_val);

    PROP_DOUBLE(val0, a_val.get0());
    PROP_DOUBLE(val1, a_val.get1());
    PROP_DOUBLE(val2, a_val.get2());

    m_val->add(val0);
    m_val->add(val1);
    m_val->add(val2);

    setDirty(1);
}

ObjectVec3::~ObjectVec3()
{
}

void ObjectVec3::do_init(
    const Object::CreateFlags   &a_flags)
{
}

string ObjectVec3::getType()
{
    return ObjectVec3::_getType();
}

void ObjectVec3::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.vec3");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectVec3::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectVec3::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDoubleSptr    val0;
    ObjectDoubleSptr    val1;
    ObjectDoubleSptr    val2;
    ObjectMapSptr       map;

    PROP_MAP(map);

    PROP_DOUBLE(val0, get0());
    PROP_DOUBLE(val1, get1());
    PROP_DOUBLE(val2, get2());

    // fucking mongo arrays..
    // using map instead :(
    // 20140926 morik@
    map->add("0", val0);
    map->add("1", val1);
    map->add("2", val2);
    map->toBSON(a_builder, a_field_name);
}

void ObjectVec3::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDoubleSptr    val0;
    ObjectDoubleSptr    val1;
    ObjectDoubleSptr    val2;
    ObjectMapSptr       map;

    PROP_MAP(map);

    PROP_DOUBLE(val0, get0());
    PROP_DOUBLE(val1, get1());
    PROP_DOUBLE(val2, get2());

    // fucking mongo arrays..
    // using miap instead :(
    // 20140926 morik@
    map->add("0", val0);
    map->add("1", val1);
    map->add("2", val2);
    map->toBSON(a_builder);
}

SV * ObjectVec3::toPerl()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SV *ret = NULL;
    ret = m_val->toPerl();
    return ret;
}

string ObjectVec3::serialize(
    const string &a_delim)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string ret;
    ret = m_val->serialize(a_delim);
    return ret;
}

// ---------------- 0, X, Pitch ----------------

// get
double ObjectVec3::get0()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDoubleSptr val;
    val = dynamic_pointer_cast<ObjectDouble>(
        m_val->at(0)
    );
    return val->getVal();
}

double ObjectVec3::getX()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return get0();
}

double ObjectVec3::getPitch()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return get0();
}

// set
void ObjectVec3::set0(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDoubleSptr val;
    PROP_DOUBLE(val, a_val);
    (*m_val.get())[0] = val;
    setDirty(1);
}

void ObjectVec3::setX(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    set0(a_val);
}

void ObjectVec3::setPitch(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double val = normalize_radians(a_val);
    set0(val);
}

// add
void ObjectVec3::add0(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double new_val = get0() + a_val;
    set0(new_val);
}

void ObjectVec3::addX(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    add0(a_val);
}

void ObjectVec3::addPitch(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double val = normalize_radians(get0() + a_val);
    set0(val);
}

// sub
void ObjectVec3::sub0(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double new_val = get0() - a_val;
    set0(new_val);
}

void ObjectVec3::subX(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    sub0(a_val);
}

void ObjectVec3::subPitch(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double val = normalize_radians(get0() - a_val);
    set0(val);
}

// ---------------- 1, Y, Yaw ----------------

// get
double ObjectVec3::get1()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDoubleSptr val;
    val = dynamic_pointer_cast<ObjectDouble>(
        m_val->at(1)
    );
    return val->getVal();
}

double ObjectVec3::getY()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return get1();
}

double ObjectVec3::getYaw()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return get1();
}

// set
void ObjectVec3::set1(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDoubleSptr val;
    PROP_DOUBLE(val, a_val);

    (*m_val.get())[1] = val;
    setDirty(1);
}

void ObjectVec3::setY(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    set1(a_val);
}

void ObjectVec3::setYaw(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double val = normalize_radians(a_val);
    set1(val);
}

// add
void ObjectVec3::add1(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double new_val = get1() + a_val;
    set1(new_val);
}

void ObjectVec3::addY(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    add1(a_val);
}

void ObjectVec3::addYaw(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double val = normalize_radians(get1() + a_val);
    set1(val);
}

// sub
void ObjectVec3::sub1(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double new_val = get1() - a_val;
    set1(new_val);
}

void ObjectVec3::subY(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    sub1(a_val);
}

void ObjectVec3::subYaw(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double val = normalize_radians(get1() - a_val);
    set1(val);
}

// ---------------- 2, Z, Roll ----------------

// get
double ObjectVec3::get2()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDoubleSptr val;
    val = dynamic_pointer_cast<ObjectDouble>(
        m_val->at(2)
    );
    return val->getVal();
}

double ObjectVec3::getZ()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return get2();
}

double ObjectVec3::getRoll()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return get2();
}

// set
void ObjectVec3::set2(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    ObjectDoubleSptr val;
    PROP_DOUBLE(val, a_val);

    (*m_val.get())[2] = val;
    setDirty(1);
}

void ObjectVec3::setZ(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    set2(a_val);
}

void ObjectVec3::setRoll(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double val = normalize_radians(a_val);
    set2(val);
}

// add
void ObjectVec3::add2(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double new_val = get2() + a_val;
    set2(new_val);
}

void ObjectVec3::addZ(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    add2(a_val);
}

void ObjectVec3::addRoll(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double val = normalize_radians(get2() + a_val);
    set2(val);
}

// sub
void ObjectVec3::sub2(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double new_val = get2() - a_val;
    set2(new_val);
}

void ObjectVec3::subZ(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    sub2(a_val);
}

void ObjectVec3::subRoll(
    const double &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    double val = normalize_radians(get2() - a_val);
    set2(val);
}

// ---------------- static ----------------

double ObjectVec3::normalize_radians(
    const double &a_val)
{
    double val = a_val;

    if (    2*M_PI  <= val
        ||  -2*M_PI >= val)
    {
        double integer = 0.0f;
        modf(val/(2*M_PI), &integer);
        val -= integer * 2*M_PI;
    }

    return val;
}

double ObjectVec3::magnitude(
    ObjectVec3Sptr a_vec)
{
    lock_guard<recursive_mutex> guard(a_vec->m_mutex);

    double ret = 0.0f;

    // calculate vector's normal (vector's length)
    // it's need for vector normalization
    ret = sqrt(
            (a_vec->get0() * a_vec->get0())
        +   (a_vec->get1() * a_vec->get1())
        +   (a_vec->get2() * a_vec->get2())
    );

    return ret;
}

ObjectVec3Sptr ObjectVec3::cross(
    ObjectVec3Sptr a_pos_view,
    ObjectVec3Sptr a_pos,
    ObjectVec3Sptr a_pos_up)
{
    lock(
        a_pos_view->m_mutex,
        a_pos->m_mutex,
        a_pos_up->m_mutex
    );

    lock_guard<recursive_mutex> guard_1(
        a_pos_view->m_mutex,
        adopt_lock
    );
    lock_guard<recursive_mutex> guard_2(
        a_pos->m_mutex,
        adopt_lock
    );
    lock_guard<recursive_mutex> guard_3(
        a_pos_up->m_mutex,
        adopt_lock
    );

    ObjectVec3Sptr vec_normal;
    ObjectVec3Sptr vec_view;

    PROP_VEC3(vec_normal, 0.0f, 0.0f, 0.0f);
    PROP_VEC3(vec_view,   0.0f, 0.0f, 0.0f);

    // get vector of view
    vec_view->set0(a_pos_view->get0() - a_pos->get0());
    vec_view->set1(a_pos_view->get1() - a_pos->get1());
    vec_view->set2(a_pos_view->get2() - a_pos->get2());

    // If We have two vectors (view vector and up vector),
    // then We have plane. We can calculate 90 degree angle
    // from it. Cross calculation is simple,
    // but it hard to keep in mind at first time

    // X = (V1[Y] * V2[Z]) - (V1[Z] * V2[Y])
    vec_normal->setX(
        (vec_view->getY() * a_pos_up->getZ())
            - (vec_view->getZ() * a_pos_up->getY())
    );

    // Y = (V1[Z] * V2[X]) - (V1[X] * V2[Z])
    vec_normal->setY(
        (vec_view->getZ() * a_pos_up->getX())
            - (vec_view->getX() * a_pos_up->getZ())
    );

    // Z = (V1[X] * V2[Y]) - (V1[Y] * V2[X])
    vec_normal->setZ(
        (vec_view->getX() * a_pos_up->getY())
            - (vec_view->getY() * a_pos_up->getX())
    );

    // Important. You can not change order,
    // otherwise it will not be work.

    // Must be order as here, Just keep in mind,
    // if You seek X, You should not use X value
    // from others vectors. The same for Y and Z.
    // Note, You will calculate values from another two axises
    // and never from same axis.

    // So, why all this? We should found axis for rotate
    // around which. Rotate to left, right is simple,
    // because vertical axis always (0, 1, 0).
    // Rotating up and down is little different,
    // because it is outside of global axises.
    // Get the book "linear algebra"
    // it book still is useful to you

    return vec_normal;
}

ObjectVec3Sptr ObjectVec3::normalize(
    ObjectVec3Sptr a_vec)
{
    lock_guard<recursive_mutex> guard(a_vec->m_mutex);

    ObjectVec3Sptr    ret         = a_vec;
    double          magnitude   = 0.0f;
    double          vec_x       = ret->getX();
    double          vec_y       = ret->getY();
    double          vec_z       = ret->getZ();

    // What is this function?
    // We must ensure that our vector has normalized.
    // Normalized vector - which means that its length
    // is equal to 1. For example, the vector (2,0,0)
    // after normalization is (1,0,0).

    if (    1 > vec_x
        &&  1 > vec_y
        &&  1 > vec_z)
    {
        goto out;
    }

    // calculate the value of the normal
    magnitude = ObjectVec3::magnitude(ret);
    if (!magnitude){
        PFATAL("magnitude is NULL for vec: '%s'"
            " (vector size == 0)\n",
            ret->toString()->c_str()
        );
    }

    // Now we have the size and we can divide our vector
    // by its magnitude. This will make the length of vector
    // equal to one, so it will be easier to work
    ret->setX(vec_x / magnitude);
    ret->setY(vec_y / magnitude);
    ret->setZ(vec_z / magnitude);

out:
    return ret;
}

/*
ObjectVec3 & ObjectVec3::operator = (
    ObjectVec3 &a_right)
{
    lock(m_mutex, a_right.m_mutex);
    lock_guard<recursive_mutex> guard_1(m_mutex,         adopt_lock);
    lock_guard<recursive_mutex> guard_2(a_right.m_mutex, adopt_lock);

    setX(a_right.getX());
    setY(a_right.getY());
    setZ(a_right.getZ());

    return *this;
}

ObjectVec3 & ObjectVec3::operator * (
    ObjectDouble &a_right)
{
    lock(m_mutex, a_right.m_mutex);
    lock_guard<recursive_mutex> guard_1(m_mutex,         adopt_lock);
    lock_guard<recursive_mutex> guard_2(a_right.m_mutex, adopt_lock);

    double val   = a_right.getVal();
    double new_x = getX() * val;
    double new_y = getY() * val;
    double new_z = getZ() * val;

    setX(new_x);
    setY(new_y);
    setZ(new_z);

    return *this;
}

ObjectVec3 & ObjectVec3::operator -= (
    ObjectVec3 &a_right)
{
    lock(m_mutex, a_right.m_mutex);
    lock_guard<recursive_mutex> guard_1(m_mutex,         adopt_lock);
    lock_guard<recursive_mutex> guard_2(a_right.m_mutex, adopt_lock);

    subX(a_right.getX());
    subY(a_right.getY());
    subZ(a_right.getZ());

    return *this;
}

ObjectVec3 & ObjectVec3::operator += (
    ObjectVec3 &a_right)
{
    lock(m_mutex, a_right.m_mutex);
    lock_guard<recursive_mutex> guard_1(m_mutex,         adopt_lock);
    lock_guard<recursive_mutex> guard_2(a_right.m_mutex, adopt_lock);

    addX(a_right.getX());
    addY(a_right.getY());
    addZ(a_right.getZ());

    return *this;
}

const ObjectVec3 ObjectVec3::operator - (
    ObjectVec3 &a_right)
{
    lock(m_mutex, a_right.m_mutex);
    lock_guard<recursive_mutex> guard_1(m_mutex,         adopt_lock);
    lock_guard<recursive_mutex> guard_2(a_right.m_mutex, adopt_lock);

    double cur_val0 = get0();
    double cur_val1 = get1();
    double cur_val2 = get2();

    ObjectVec3 ret(
        Object::CreateFlags(Object::CREATE_PROP),
        cur_val0,
        cur_val1,
        cur_val2
    );
    ret.sub0(a_right.get0());
    ret.sub1(a_right.get1());
    ret.sub2(a_right.get2());

    return ret;
}

const ObjectVec3 ObjectVec3::operator + (
    ObjectVec3 &a_right)
{
    lock(m_mutex, a_right.m_mutex);
    lock_guard<recursive_mutex> guard_1(m_mutex,         adopt_lock);
    lock_guard<recursive_mutex> guard_2(a_right.m_mutex, adopt_lock);

    double cur_val0 = get0();
    double cur_val1 = get1();
    double cur_val2 = get2();

    ObjectVec3 ret(
        Object::CreateFlags(Object::CREATE_PROP),
        cur_val0,
        cur_val1,
        cur_val2
    );
    ret.add0(a_right.get0());
    ret.add1(a_right.get1());
    ret.add2(a_right.get2());

    return ret;
}

*/

bool ObjectVec3::operator != (
    ObjectVec3 &a_right)
{
    bool ret = false;

    lock(m_mutex, a_right.m_mutex);
    lock_guard<recursive_mutex> guard_1(m_mutex,         adopt_lock);
    lock_guard<recursive_mutex> guard_2(a_right.m_mutex, adopt_lock);

    if (    getX() != a_right.getX()
        ||  getY() != a_right.getY()
        ||  getZ() != a_right.getZ())
    {
        ret = true;
    }

    return ret;
}

// ---------------- module ----------------

string ObjectVec3::_getType()
{
    return "core.object.vec3";
}

string ObjectVec3::_getTypePerl()
{
    return "core::object::vec3::main";
}

int32_t ObjectVec3::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectVec3::_destroy()
{
}

ObjectSptr ObjectVec3::_object_create()
{
    ObjectSptr object;
    _OBJECT_VEC3(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectVec3::_getType,
    .type_perl      = ObjectVec3::_getTypePerl,
    .init           = ObjectVec3::_init,
    .destroy        = ObjectVec3::_destroy,
    .object_create  = ObjectVec3::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

