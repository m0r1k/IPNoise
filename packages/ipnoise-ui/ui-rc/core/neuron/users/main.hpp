#include "class.hpp"
DEFINE_CLASS(NeuronUsers);
DEFINE_CLASS(NeuronUser);

#ifndef NEURON_USERS_HPP
#define NEURON_USERS_HPP

#include <stdio.h>

#include <map>
#include <string>
#include <memory>

#include "prop/map.hpp"
#include "prop/int.hpp"
#include "prop/string.hpp"
#include "prop.hpp"
#include "prop/neuron/type.hpp"
#include "http_answer.hpp"

#include "core/neuron/neuron/main.hpp"

#undef  SUPER_CLASS
#define SUPER_CLASS   Neuron

#define NEURON_USERS_ID "core.users"

using namespace std;

class NeuronUsers
    :   public  SUPER_CLASS
{
    public:
        NeuronUsers();
        virtual ~NeuronUsers();

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);

        // generic
        virtual int32_t do_autorun();
        NeuronUserSptr  createUser();
        void            getUsers(
            vector<NeuronUserSptr> &a_out
        );
        NeuronUserSptr  getUserByLogin(
            const string &a_login
        );
        NeuronUserSptr  authUser(
            const string &a_login,
            const string &a_password = ""
        );

        // static
        static NeuronUserSptr       create_user();
        static NeuronSessionSptr    get_session(
            PropNeuronIdSptr a_sessid
        );
        static NeuronUserSptr       auth_user(
            const string &a_login,
            const string &a_password = ""
        );

        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    private:
        void do_init();
};

#endif

