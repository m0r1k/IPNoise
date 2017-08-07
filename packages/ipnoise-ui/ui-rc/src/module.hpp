#include "class.hpp"
DEFINE_CLASS(Neuron);

#ifndef MODULE_HPP
#define MODULE_HPP

#include <memory>
#include <string>
#include <map>
#include <stdint.h>

//#include "network.hpp"
//#include "core/neuron/neuron/main.hpp"

using namespace std;

typedef struct
{
    const char  *type;
    int32_t     (*init)(void);
    void        (*destroy)(void);
    NeuronSptr  (*object_create)(void);
    uint32_t    flags;
    NeuronSptr  neuron;
} ModuleInfo;

typedef shared_ptr<ModuleInfo>      ModuleInfoSptr;
typedef map<string, ModuleInfoSptr> Modules;
typedef Modules::iterator           ModulesIt;

extern Modules g_modules;

#define REGISTER_MODULE(info) \
    static int32_t s_module_inited = Network::register_module(info)

#endif

