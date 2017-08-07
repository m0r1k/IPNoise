#include "class.hpp"

#ifndef MODULE_HPP
#define MODULE_HPP

#include <memory>
#include <map>
#include <string>
#include <vector>

#define IPNOISE_MODULES_DIR     "modules"

DEFINE_CLASS(Object);

using namespace std;

enum CreateFlags
{
    CREATE_DEFAULT                  = 0,
    CREATE_OBJECT                   = 0,       // bit 0
    CREATE_PROP                     = 1,       // bit 0
    REMOVE_WHEN_LINKS_LOST          = (1 << 1),// bit 1
    REMOVE_WHEN_LINKS_LOST_FORCE    = (1 << 2) // bit 2
};

// after change this structure all already compiled modules
// will NOT work, until modules will not be recompilated,
// so never change it in production
typedef struct
{
    string      (*getVersion)();
    ObjectSptr  (*objectCreate)(
        const string        &a_type,
        const char          *a_data,
        const uint64_t      &a_data_size,
        const CreateFlags   &a_create_flags
    );
    ObjectSptr  (*objectLoad)(const char *a_path);
/*
    void        (*objectActionIn)(
        ObjectSptr  a_object,
        ObjectSptr  a_action
    );
    void        (*objectActionOut)(
        ObjectSptr  a_object,
        ObjectSptr  a_action
    );
*/
    ObjectSptr  (*propCreate)(
        const string        &a_type,
        const char          *a_data,
        const uint64_t      &a_data_size,
        const CreateFlags   &a_create_flags
    );
    string      tmp_path;
} EngineInfo;
typedef shared_ptr<EngineInfo>  EngineInfoSptr;

// TestInfo
typedef struct
{
    string  name;
    int32_t (*run)(const char *tmp_path);
    int32_t success_code;
} TestInfo;
typedef shared_ptr<TestInfo>  TestInfoSptr;

// action handlers
typedef int32_t (*ActionHandler)(
    Object *a_object,
    Object *a_action
);

typedef map<string, ActionHandler>      ActionsHandlers;
typedef ActionsHandlers::iterator       ActionsHandlersIt;
typedef ActionsHandlers::const_iterator ActionsHandlersConstIt;

// tests
typedef vector<TestInfo>                Tests;
typedef Tests::iterator                 TestsIt;
typedef Tests::const_iterator           TestsConstIt;

// action's translate table
typedef map<string, string>             TranslateTable;
typedef TranslateTable::iterator        TranslateTableIt;
typedef TranslateTable::const_iterator  TranslateTableConstIt;

// ModuleInfo
typedef struct
{
    string      (*getType)(void);
    int32_t     (*init)(EngineInfo *);
    int32_t     (*shutdown)(void);
    ObjectSptr  (*objectCreate)(const CreateFlags &);
    void        (*getTests)(vector<TestInfo> &);
//    void        (*getActionsIn)(ActionsHandlers &);
//    void        (*getActionsOut)(ActionsHandlers &);
    uint32_t    flags;
    ObjectSptr  object;
} ModuleInfo;

// ModuleInfoFull
typedef struct
{
    ModuleInfo          *so;
    void                *so_handle;
//    ActionsHandlers     actions_in;
//    ActionsHandlers     actions_out;
    vector<TestInfo>    tests;
} ModuleInfoFull;
typedef shared_ptr<ModuleInfoFull>      ModuleInfoFullSptr;

// Modules
typedef map<string, ModuleInfoFullSptr> Modules;
typedef Modules::iterator               ModulesIt;
typedef Modules::const_iterator         ModulesConstIt;

#endif

