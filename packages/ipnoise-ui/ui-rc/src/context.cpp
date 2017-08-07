#include "core/neuron/user/main.hpp"
#include "core/neuron/session/main.hpp"
#include "thread/http.hpp"

#include "context.hpp"

Context::Context(NeuronUserSptr a_user)
{
    m_user  = a_user;
    m_fd    = -1;
}

Context::~Context()
{
}

NeuronSessionSptr Context::getSession()
{
    return m_session;
}

void Context::setSession(
    NeuronSessionSptr a_session)
{
    // store session in context
    m_session = a_session;
}

NeuronUserSptr Context::getUser()
{
    return m_user;
}

int32_t Context::getFd()
{
    return m_fd;
}

void Context::setFd(
    int32_t a_fd)
{
    m_fd = a_fd;
}

void Context::serializeToPropMap(
    PropMapSptr a_out)
{
    PropMapSptr info_user       = PROP_MAP();
    PropMapSptr info_session    = PROP_MAP();

    m_user->serializeToPropMap(info_user);
    m_session->serializeToPropMap(info_session);

    (*(a_out.get()))[PROP_STRING("user")]    = info_user;
    (*(a_out.get()))[PROP_STRING("session")] = info_session;
}

string Context::serialize(
    const string &a_delim)
{
    string      ret;
    PropMapSptr map = PROP_MAP();

    serializeToPropMap(map);
    ret = map->serialize(a_delim);

    return ret;
}

NeuronUserSptr Context::getCurUser()
{
    ContextSptr         context;
    NeuronUserSptr      context_user;

    // get curent user
    context = ThreadHttp::getContext();
    if (context){
        context_user = context->getUser();
    }

    return context_user;
}

NeuronSessionSptr Context::getCurSession()
{
    ContextSptr         context;
    NeuronSessionSptr   context_session;

    // get curent user
    context = ThreadHttp::getContext();
    if (context){
        context_session = context->getSession();
    }

    return context_session;
}

