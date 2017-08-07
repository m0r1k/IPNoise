#include "prop/neuron/acl.hpp"

PropNeuronAclIterator::PropNeuronAclIterator()
{
}

PropNeuronAclIterator::~PropNeuronAclIterator()
{
}

bool PropNeuronAclIterator::operator != (
    const PropNeuronAclIterator &a_right) const
{
    return m_it != a_right.m_it;
}

void PropNeuronAclIterator::operator ++ (int32_t){
    m_it++;
}

PropNeuronAclValsIt PropNeuronAclIterator::operator -> (){
    return m_it;
}

// -------------------------- PropNeuronAcl ------------------------

PropNeuronAcl::PropNeuronAcl()
    :   Prop(Prop::PROP_TYPE_MAP)
{
}

PropNeuronAcl::PropNeuronAcl(const map<PropSptr, PropSptr> &a_vals)
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

PropNeuronAcl::PropNeuronAcl(
    const PropSptr  &a_key,
    const PropSptr  &a_val)
    :   Prop(Prop::PROP_TYPE_MAP)
{
    add(a_key, a_val);
    setDirty(1);
}

PropNeuronAcl::~PropNeuronAcl()
{
}

void PropNeuronAcl::add(
    const PropSptr &a_key,
    const PropSptr &a_val)
{
    m_vals[a_key] = a_val;
    setDirty(1);
}

bool PropNeuronAcl::has(
    const PropSptr &a_key)
{
    bool                ret = false;
    PropNeuronAclValsConstIt  it;

    it = m_vals.find(a_key);
    if (m_vals.end() != it){
        ret = true;
    }
    return ret;
}

string PropNeuronAcl::serialize(
    const string &a_delim) const
{
    string              ret, data;
    PropNeuronAclValsConstIt  it;

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

string PropNeuronAcl::toString() const
{
    return serialize();
}

PropNeuronAcl::operator PropSptr() const
{
    return PropSptr(new PropNeuronAcl(*this));
}

PropNeuronAcl::iterator PropNeuronAcl::begin()
{
    PropNeuronAcl::iterator it;
    it.m_it = m_vals.begin();
    return it;
}

PropNeuronAcl::iterator PropNeuronAcl::end()
{
    PropNeuronAcl::iterator it;
    it.m_it = m_vals.end();
    return it;
}

PropNeuronAcl::iterator PropNeuronAcl::find(
    const PropSptr &a_key)
{
    PropNeuronAcl::iterator it;
    it.m_it = m_vals.find(a_key);
    return it;
}

void PropNeuronAcl::erase(const PropNeuronAcl::iterator &a_it)
{
    m_vals.erase(a_it.m_it);
    setDirty(1);
}

void PropNeuronAcl::erase(const PropSptr &a_key)
{
    PropNeuronAcl::iterator it = find(a_key);
    erase(it);
}

PropSptr & PropNeuronAcl::operator [] (const PropSptr &a_key)
{
    PropNeuronAclValsIt it;

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

