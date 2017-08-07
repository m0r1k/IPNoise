#include "network.hpp"
#include "core/neuron/user/main.hpp"
#include "core/neuron/god/main.hpp"

#include "core/neuron/users/main.hpp"

extern Network *g_network;

NeuronUsers::NeuronUsers()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronUsers::~NeuronUsers()
{
};

void NeuronUsers::do_init()
{
    NeuronParamSptr param;

    // props
    initType(PROP_NEURON_TYPE("core.neuron.users"));
    initId(PROP_NEURON_ID("core.users"));

    // params
}

int32_t NeuronUsers::do_autorun()
{
    int32_t err = 0;

    // create god if need
    getCreateNeigh<NeuronGod>();

    return err;
}

void NeuronUsers::getAllProps(
    PropMapSptr a_props)
{
    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronUsers::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

// ---------------- user ----------------

NeuronUserSptr NeuronUsers::create_user()
{
    NeuronUserSptr  user;
    NeuronUsersSptr users;
    DbThread        *db_thread = NULL;

    db_thread = g_network->getDbThread();
    users     = db_thread->getNeuronById<NeuronUsers>(
        PROP_NEURON_ID(NEURON_USERS_ID)
    );
    user = users->createUser();

    return user;
}

NeuronUserSptr NeuronUsers::createUser()
{
    NeuronUserSptr user;

    user = createNeigh<NeuronUser>();

    return user;
}

NeuronUserSptr NeuronUsers::getUserByLogin(
    const string &a_login)
{
    NeuronUserSptr                      user;
    vector<NeuronUserSptr>              res;
    vector<NeuronUserSptr>::iterator    res_it;

    if (!a_login.size()){
        goto out;
    }

    // TODO optimize me
    // 20140609 morik@
    getNeighs<NeuronUser>(res);
    for (res_it = res.begin();
        res_it != res.end();
        res_it++)
    {
        NeuronUserSptr  cur_user = *res_it;
        NeuronParamSptr param_login;
        string          login;

        param_login = cur_user->getParam("login");
        if (param_login){
            login = param_login
                ->getValue()
                ->toString();
        }

        if (a_login == login){
            user = cur_user;
            break;
        }
    }

out:
    return user;
}

NeuronSessionSptr NeuronUsers::get_session(
    PropNeuronIdSptr    a_sessid)
{
    NeuronSessionSptr   session;
    DbThread            *db_thread = NULL;

    db_thread = g_network->getDbThread();
    session   = db_thread->getNeuronById<NeuronSession>(
        a_sessid
    );

    return session;
}

NeuronUserSptr NeuronUsers::authUser(
    const string &a_login,
    const string &a_password)
{
    NeuronUserSptr  ret, user;

    user = getUserByLogin(a_login);
    if (user){
        NeuronParamSptr param;

        param = user->getParam("password");
        if (param){
            string password = param.get()
                ->getValue().get()
                ->toString();

            if (    a_password.size()
                &&  password == a_password)
            {
                ret = user;
            }
        }
    }

    return ret;
}

NeuronUserSptr NeuronUsers::auth_user(
    const string &a_login,
    const string &a_password)
{
    NeuronUserSptr  user;
    NeuronUsersSptr users;
    DbThread        *db_thread = NULL;

    db_thread = g_network->getDbThread();
    users     = db_thread->getNeuronById<NeuronUsers>(
        PROP_NEURON_ID(NEURON_USERS_ID)
    );

    user = users->authUser(
        a_login,
        a_password
    );

    return user;
}

// ---------------- module ----------------

int32_t NeuronUsers::init()
{
    int32_t err = 0;
    return err;
}

void NeuronUsers::destroy()
{
}

NeuronSptr NeuronUsers::object_create()
{
    NeuronSptr neuron(new NeuronUsers);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.users",
    .init           = NeuronUsers::init,
    .destroy        = NeuronUsers::destroy,
    .object_create  = NeuronUsers::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

