#include "prop/string.hpp"
#include "prop/map.hpp"

// -------------------------- PropMapIt ----------------------

PropMapIterator::PropMapIterator()
{
}

PropMapIterator::~PropMapIterator()
{
}

// !=

bool PropMapIterator::operator != (
    const PropMapIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool PropMapIterator::operator != (
    const PropMapConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

// ==

bool PropMapIterator::operator == (
    const PropMapIterator &a_right) const
{
    return m_it == a_right.m_it;
}

bool PropMapIterator::operator == (
    const PropMapConstIterator &a_right) const
{
    return m_it == a_right.m_it;
}

// ++

void PropMapIterator::operator ++ (int32_t){
    m_it++;
}

// ->

PropMapValsIt PropMapIterator::operator -> (){
    return m_it;
}

// -------------------------- PropMapConstIt -----------------

PropMapConstIterator::PropMapConstIterator()
{
}

PropMapConstIterator::~PropMapConstIterator()
{
}

bool PropMapConstIterator::operator != (
    const PropMapConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool PropMapConstIterator::operator != (
    const PropMapIterator &a_right) const
{
    return m_it != a_right.m_it;
}

void PropMapConstIterator::operator ++ (int32_t){
    m_it++;
}

PropMapValsConstIt PropMapConstIterator::operator -> (){
    return m_it;
}

// -------------------------- PropMap ------------------------

PropMap::PropMap()
    :   Prop(Prop::PROP_TYPE_MAP)
{
}

PropMap::PropMap(const map<PropSptr, PropSptr> &a_vals)
    :   Prop(Prop::PROP_TYPE_MAP)
{
    map<PropSptr, PropSptr>::const_iterator it;
    for (it = a_vals.begin();
        it != a_vals.end();
        it++)
    {
        m_vals[it->first] = it->second;
    }
    setDirty(1);
}

PropMap::PropMap(
    const PropSptr  &a_key,
    const PropSptr  &a_val)
    :   Prop(Prop::PROP_TYPE_MAP)
{
    add(a_key, a_val);
    setDirty(1);
}

PropMap::~PropMap()
{
}

void PropMap::add(
    const PropSptr &a_key,
    const PropSptr &a_val)
{
    m_vals[a_key] = a_val;
    setDirty(1);
}

void PropMap::add(
    const string &a_key,
    const string &a_val)
{
    PropSptr key(new PropString(a_key));
    PropSptr val(new PropString(a_val));

    m_vals[key] = val;
    setDirty(1);
}

void PropMap::add(
    const string    &a_key,
    const PropSptr  &a_val)
{
    PropSptr key(new PropString(a_key));

    m_vals[key] = a_val;
    setDirty(1);
}

bool PropMap::has(const string &a_key)
{
    return has(PropSptr(new PropString(a_key)));
}

bool PropMap::has(const PropSptr &a_key)
{
    bool                ret = false;
    PropMapValsConstIt  it;

    it = m_vals.find(a_key);
    if (m_vals.end() != it){
        ret = true;
    }
    return ret;
}

string PropMap::serialize(
    const string &a_delim) const
{
    string              ret, data;
    PropMapValsConstIt  it;

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        PropSptr    key_prop    = it->first;
        PropSptr    val_prop    = it->second;

        if (data.size()){
            data += ", ";
        }

        data += key_prop->serialize(a_delim);
        data += a_delim;
        data += val_prop->serialize(a_delim);
    }

    ret += "{";
    if (data.size()){
        ret += " " + data + " ";
    }
    ret += "}";

    return ret;
}

string PropMap::toString() const
{
    return serialize();
}

PropMap::operator PropSptr() const
{
    return PropSptr(new PropMap(*this));
}

PropMap::iterator PropMap::begin()
{
    PropMap::iterator it;
    it.m_it = m_vals.begin();
    return it;
}

PropMap::iterator PropMap::end()
{
    PropMap::iterator it;
    it.m_it = m_vals.end();
    return it;
}

PropMap::iterator PropMap::find(
    const PropSptr &a_key)
{
    PropMap::iterator it;
    it.m_it = m_vals.find(a_key);
    return it;
}

PropMap::iterator PropMap::find(
    const string &a_key)
{
    PropMap::iterator it;
    it.m_it = m_vals.find(
        PROP_STRING(a_key)
    );
    return it;
}

void PropMap::erase(const PropMap::iterator &a_it)
{
    m_vals.erase(a_it.m_it);
    setDirty(1);
}

void PropMap::erase(const PropSptr &a_key)
{
    PropMap::iterator it = find(a_key);
    if (end() != it){
        erase(it);
    }
}

PropSptr PropMap::get(const PropSptr &a_key)
{
    PropSptr ret;

    if (has(a_key)){
        ret = (*this)[a_key];
    }

    return ret;
}

PropSptr PropMap::get(const string &a_key)
{
    PropSptr ret;
    PropSptr key(new PropString(a_key));

    if (has(key)){
        ret = (*this)[key];
    }

    return ret;
}

bool PropMap::empty() const
{
    return m_vals.empty();
}

// for assign only!
// Example:
// PropMapSptr map = PROP_MAP();
// map["key"] = PROP_STRING("value");
//
// method will create empty pair!
// Example:
// PropSptr val = (*(map.get())["key"];
// will create key "key" with empty val PropSptr()
//
PropSptr & PropMap::operator [] (const PropSptr &a_key)
{
    PropMapValsIt it;

    // create new if not exist
    it = m_vals.find(a_key);
    if (m_vals.end() == it){
        m_vals[a_key] = PropSptr();
    }

    setDirty(1);

    // search again
    it = m_vals.find(a_key);
    return it->second;
}

PropSptr & PropMap::operator [] (const string &a_key)
{
    PropSptr prop_key = PROP_STRING(a_key);
    prop_key->setDirty(0);
    return (*this)[prop_key];
}

