#include <algorithm>

#include "prop/vector.hpp"

// -------------------------- PropVectorIt -------------------

PropVectorIterator::PropVectorIterator()
{
}

PropVectorIterator::~PropVectorIterator()
{
}

// !=

bool PropVectorIterator::operator != (
    const PropVectorIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool PropVectorIterator::operator != (
    const PropVectorConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

// ==

bool PropVectorIterator::operator == (
    const PropVectorIterator &a_right) const
{
    return m_it == a_right.m_it;
}

bool PropVectorIterator::operator == (
    const PropVectorConstIterator &a_right) const
{
    return m_it == a_right.m_it;
}

// ++

void PropVectorIterator::operator ++ (int32_t){
    m_it++;
}

// *

PropSptr PropVectorIterator::operator * (){
    return *m_it;
}

// ->

PropVectorValsIt PropVectorIterator::operator -> (){
    return m_it;
}

// -------------------------- PropVectorConstIt --------------

PropVectorConstIterator::PropVectorConstIterator()
{
}

PropVectorConstIterator::~PropVectorConstIterator()
{
}

bool PropVectorConstIterator::operator != (
    const PropVectorConstIterator &a_right) const
{
    return m_it != a_right.m_it;
}

bool PropVectorConstIterator::operator != (
    const PropVectorIterator &a_right) const
{
    return m_it != a_right.m_it;
}

void PropVectorConstIterator::operator ++ (int32_t){
    m_it++;
}

PropSptr PropVectorConstIterator::operator * (){
    return *m_it;
}

PropVectorValsConstIt PropVectorConstIterator::operator -> (){
    return m_it;
}

// -------------------------- PropVector ---------------------

PropVector::PropVector()
    :   Prop(Prop::PROP_TYPE_VECTOR)
{
}

PropVector::PropVector(const PropVectorVals &a_vals)
    :   Prop(Prop::PROP_TYPE_VECTOR)
{
    m_vals = a_vals;
    setDirty(1);
}

PropVector::~PropVector()
{
}

PropVector::iterator PropVector::begin()
{
    PropVector::iterator it;
    it.m_it = m_vals.begin();
    return it;
}

PropVector::iterator PropVector::end()
{
    PropVector::iterator it;
    it.m_it = m_vals.end();
    return it;
}

void PropVector::add(const PropSptr &a_val)
{
    push_back(a_val);
    setDirty(1);
}

PropVector::iterator PropVector::erase(
    PropVector::iterator    a_it)
{
    PropVector::iterator it;
    it.m_it = m_vals.erase(a_it.m_it);

    return it;
}

PropSptr PropVector::at(
    const uint32_t &a_id)
{
    PropSptr ret;
    ret = m_vals.at(a_id);
    return ret;
}

PropSptr PropVector::get(
    const uint32_t &a_id)
{
    PropSptr ret;
    ret = at(a_id);
    return ret;
}

void PropVector::push_back(const PropSptr &a_val)
{
    m_vals.push_back(a_val);
    setDirty(1);
}

void PropVector::pop_back()
{
    m_vals.pop_back();
    setDirty(1);
}

string PropVector::serialize(
    const string &a_delim) const
{
    string                  ret, data;
    PropVectorValsConstIt   it;

    for (it = m_vals.begin();
        it != m_vals.end();
        it++)
    {
        PropSptr prop = *it;

        if (data.size()){
            data += ", ";
        }
        data += prop->serialize(a_delim);
    }

    ret += "[";
    if (data.size()){
        ret += " " + data + " ";
    }
    ret += "]";

    return ret;
}

size_t PropVector::size() const
{
    return m_vals.size();
}

string PropVector::toString() const
{
    return serialize();
}

void PropVector::sort()
{
    std::sort(m_vals.begin(), m_vals.end());
    setDirty(1);
}

PropSptr & PropVector::operator [] (
    const uint32_t &a_pos)
{
    PropVectorValsIt it;

    // create new if not exist
    it = m_vals.begin() + a_pos;
    if (m_vals.end() == it){
        m_vals[a_pos] = PropSptr();
    }

    setDirty(1);

    // search again
    it = m_vals.begin() + a_pos;
    return *it;
}

PropVector::operator PropSptr() const
{
    return PropSptr(new PropVector(*this));
}

