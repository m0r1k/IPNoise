#include "class.hpp"

#ifndef MAIN_HPP
#define MAIN_HPP

#include <map>
#include <string>
#include <thread>
#include <memory>
#include <mutex>

#define IPNOISE_VERSION     "0.3"

typedef enum {
    STATE_INIT              = 0,
    STATE_MAIN_LOOP_STARTED = 10,
    STATE_SHUTDOWN          = 20
} State;

using namespace std;

DEFINE_CLASS(Object);

#include "module.hpp"

typedef map<int32_t, string>            InotifyWatches;
typedef InotifyWatches::iterator        InotifyWatchesIt;
typedef InotifyWatches::const_iterator  InotifyWatchesConstIt;

typedef shared_ptr<thread>              ThreadSptr;

typedef map<string, ThreadSptr>         Threads;
typedef Threads::iterator               ThreadsIt;
typedef Threads::const_iterator         ThreadsConstIt;

void        usage(
    int32_t a_argc,
    char    *a_argv[]
);
void        shutdownThreads();
int32_t     initDb();
void        thread_run(const string &a_thread_path);
int32_t     run();

string      getVersion();
ObjectSptr  objectCreate(
    const string        &a_type,
    const char          *a_data         = NULL,
    const uint64_t      &a_data_size    = 0,
    const CreateFlags   &a_create_flags = CreateFlags::CREATE_DEFAULT
);
ObjectSptr  propCreate(
    const string        &a_type,
    const char          *a_data         = NULL,
    const uint64_t      &a_data_size    = 0,
    const CreateFlags   &a_create_flags = CreateFlags::CREATE_DEFAULT
);

string      normalizePath(const char *a_path);
ObjectSptr  objectLoad(const char *a_path);
void        objectActionIn(
    ObjectSptr  a_object,
    ObjectSptr  a_action
);
void        objectActionOut(
    ObjectSptr  a_object,
    ObjectSptr  a_action
);
void        shutdownModules();
int32_t     initModule(const string &a_path);
int32_t     initModules(const string &a_path);
string      getModuleTypeFromPath(const string &a_type);

#endif

