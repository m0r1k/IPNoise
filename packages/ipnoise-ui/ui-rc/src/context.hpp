#include "class.hpp"
DEFINE_CLASS(Context);

#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <string>
#include <map>

#include <stdint.h>
#include "prop/map.hpp"

DEFINE_CLASS(NeuronUser);
DEFINE_CLASS(NeuronSession);

using namespace std;

class Context
{
    public:
        Context(NeuronUserSptr);
        virtual ~Context();

        NeuronSessionSptr   getSession();
        void                setSession(NeuronSessionSptr);
        void                updateEventSession(
            NeuronSessionSptr,
            int32_t a_event_fd = -1
        );

        void    serializeToPropMap(PropMapSptr);
        string  serialize(
                const string &a_delim = ": "
        );
        NeuronUserSptr  getUser();

        void    setFd(int32_t);
        int32_t getFd();

        static NeuronUserSptr       getCurUser();
        static NeuronSessionSptr    getCurSession();

    private:
        NeuronUserSptr      m_user;
        NeuronSessionSptr   m_session;
        int32_t             m_fd;
};

#endif

