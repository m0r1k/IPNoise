#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"

#include "core/object/string/main.hpp"

ObjectString::ObjectString(
    const Object::CreateFlags   &a_flags)
    :   Object(a_flags)
{
    do_init(a_flags);
}

ObjectString::ObjectString(
    const Object::CreateFlags   &a_flags,
    const char                  *a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    m_val = a_val;
    setDirty(1);
}

ObjectString::ObjectString(
    const Object::CreateFlags   &a_flags,
    const char                  a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    m_val = a_val;
    setDirty(1);
}

ObjectString::ObjectString(
    const Object::CreateFlags   &a_flags,
    const string                &a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    m_val = a_val;
    setDirty(1);
}

ObjectString::ObjectString(
    const Object::CreateFlags   &a_flags,
    ObjectStringSptr            a_val)
    :   Object(a_flags)
{
    do_init(a_flags);
    m_val = a_val->getVal();
    setDirty(1);
}

ObjectString::~ObjectString()
{
}

void ObjectString::do_init(
    const Object::CreateFlags &a_flags)
{
}

string ObjectString::getType()
{
    return ObjectString::_getType();
}

void ObjectString::do_init_props()
{
    Object::do_init_props();

//    ObjectTypeSptr  prop_type;
//    PROP_TYPE(prop_type, "core.object.string");
//    INIT_PROP(this, Type, prop_type);
}

void ObjectString::do_init_api()
{
    Object::do_init_api();

    // register actions
}

void ObjectString::toBSON(
    mongo::BSONObjBuilder   &a_builder,
    const string            &a_field_name)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    a_builder.append(a_field_name, m_val);
}

void ObjectString::toBSON(
    mongo::BSONArrayBuilder &a_builder)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    a_builder.append(m_val);
}

SV * ObjectString::toPerl()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    SV *ret = NULL;
    ret = newSVpv(m_val.c_str(), m_val.size());
    return ret;
}

string ObjectString::toString()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val;
}

string ObjectString::serialize(
    const string &)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string ret = "\"" + m_val + "\"";
    return ret;
}

bool ObjectString::empty()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val.empty();
}

uint64_t ObjectString::size()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val.size();
}

void ObjectString::assign(
    const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_val = a_val;
}

const char * ObjectString::c_str() noexcept
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val.c_str();
}

char ObjectString::at(
    const int32_t   &a_pos)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val.at(a_pos);
}

void ObjectString::toLower()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    // force to be lower
    transform(
        m_val.begin(),
        m_val.end(),
        m_val.begin(),
        ::tolower
    );
}

void ObjectString::add(const string &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_val += a_val;
    setDirty(1);
}

void ObjectString::add(
    const char &a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_val += a_val;
    setDirty(1);
}

void ObjectString::add(
    const char      *a_buff,
    const uint64_t  &a_len)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    m_val.append(a_buff, a_len);
    setDirty(1);
}

void ObjectString::add(
    ObjectStringSptr a_val)
{
    lock_guard<recursive_mutex> guard(m_mutex);

    add(a_val->c_str(), a_val->size());
    setDirty(1);
}

string ObjectString::getVal()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val;
}

string ObjectString::hexdump()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    string ret;

    ret = Utils::dump(
        m_val.c_str(),
        m_val.size()
    );

    return ret;
}
/*
ObjectVectorSptr ObjectString::split(
    const char  &a_delim,
    const bool  &a_delim_can_repeat)
{
    ObjectVectorSptr ret;

    ret = ObjectString::split(
        m_val,
        a_delim,
        a_delim_can_repeat
    );

    return ret;
}
*/

ObjectString::operator string()
{
    lock_guard<recursive_mutex> guard(m_mutex);

    return m_val;
}

// ---------------- static ----------------

/*
 * Split string on parts by delim
 *
 * Example:
 *   a_val:   "test//123"
 *   a_delim: '/'
 *
 * if a_delim_can_repeat = 1, then result: ["test", "123"]
 * if a_delim_can_repeat = 0, then result: ["test", "", "123"]
 *
 */
ObjectVectorSptr ObjectString::split(
    const char  *a_val,
    const char  &a_delim,
    const bool  &a_delim_can_repeat)
{
    ObjectVectorSptr    vector;
    ObjectStringSptr    part;
    int32_t             i, len = 0;

    if (!a_val){
        PFATAL("missing argument 'a_val'\n");
    }

    len = strlen(a_val);
    if (!len){
        PFATAL("missing argument 'a_val'\n");
    }

    // fill vector by split on delimiter
    PROP_VECTOR(vector);

    for (i = 0; i < len; i++){
        uint8_t c = a_val[i];
        if (!part){
            PROP_STRING(part);
        }
        if (c != a_delim){
            // it's not delim
            part->add(c);
            continue;
        }
        // it's delim
        if (    part->empty()
            &&  a_delim_can_repeat)
        {
            // if delim can repeat, skip this delim
            continue;
        }
        vector->add(part);
        // clear part
        part = ObjectStringSptr();
    }

    if (part){
        vector->add(part);
    }

    return vector;
}

// ---------------- module ----------------

string ObjectString::_getType()
{
    return "core.object.string";
}

string ObjectString::_getTypePerl()
{
    return "core::object::string::main";
}

int32_t ObjectString::_init()
{
    int32_t err = 0;
    return err;
}

void ObjectString::_destroy()
{
}

ObjectSptr ObjectString::_object_create()
{
    ObjectSptr object;
    _OBJECT_STRING(object, 0);
    return object;
}

static ModuleInfo info = {
    .type           = ObjectString::_getType,
    .type_perl      = ObjectString::_getTypePerl,
    .init           = ObjectString::_init,
    .destroy        = ObjectString::_destroy,
    .object_create  = ObjectString::_object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

