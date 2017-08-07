#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <dlfcn.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <ipnoise-common/log_common.h>
#include <core/object/object/main.hpp>
#include "db.hpp"
#include "module.hpp"

#include "engine.hpp"

int32_t             g_debug_level   = 0;
State               g_state         = STATE_INIT;

Threads             g_threads;
recursive_mutex     g_threads_rmutex;

Modules             g_modules;
recursive_mutex     g_modules_rmutex;

EngineInfo          *g_engine_info  = NULL;
recursive_mutex     g_engine_info_rmutex;

void usage(
    int32_t a_argc,
    char    *a_argv[])
{
    PINFO("Usage: %s [-options]                             \n"
        "                                                   \n"
        "Warning:                                           \n"
        "Don't use space between option's name and value for\n"
        "arguments with [optional value], examples:         \n"
        "%s -c core.object.string -s/db/objects/obj1 -d 10  \n"
        "%s -l/db/objects/obj1 -s/db/objects/obj2 -d 10     \n"
        "                                                   \n"
        "Options:                                           \n"
        "  -h, --help               This help               \n"
        "                                                   \n"
        "  -V, --version            Program version         \n"
        "  -D, --debug   <level>    Debug level             \n"
        "  -M, --modules <path>     Path to modules dir     \n"
        "  -S, --server             Start server            \n"
        "  -T, --tmp     <path>     Temporary dir path      \n"
        "  -B, --benchmark          Start benchmark         \n"
        "                                                   \n"
        "  -c, --create       <type>  Object create         \n"
        "  -p, --path         <path>  Object set path       \n"
        "  -l, --load         [path]  Object load           \n"
        "  -i, --init         <data>  Object init by data   \n"
        "  -a, --link-add     <path>  Object link add       \n"
        "  -e, --link-erase   <path>  Object link erase     \n"
        "  -d, --dump         [path]  Object dump           \n"
        "  -s, --save         [path]  Object save           \n"
        "  -t, --link-to      <path>  Object symlink create \n"
        "\n",
        a_argv[0],
        a_argv[0],
        a_argv[0]
    );
}

void shutdown()
{
    PINFO("shutdown..\n");

    if (STATE_MAIN_LOOP_STARTED == g_state){
        ::s_rm(IPNOISE_DB_FILE_PID);
    } else {
        exit(0);
    }
}

void sigint(int)
{
    shutdown();
}

void sigterm(int)
{
    shutdown();
}

void shutdownThreads()
{
    int32_t res;

    // request threads stop
    // TODO XXX FIXME remove this hardcode
    res = ::s_rm(IPNOISE_DB_THREADS, 1, 1);
    if (res){
        PFATAL("cannot rm: '%s', res: '%d'\n",
            IPNOISE_DB_THREADS,
            res
        );
    }

    while (g_threads.size()){
        if (g_threads_rmutex.try_lock()){
            // try lock
            ThreadsIt   it = g_threads.begin();
            ThreadSptr  thread = it->second;

            g_threads.erase(it);
            g_threads_rmutex.unlock();

            if (thread->joinable()){
                thread->join();
            }
        } else {
            // little wait
            usleep(10*1000);
        }
    }
}

void removePidFile()
{
    // remove pid file
    ::s_rm(IPNOISE_DB_FILE_PID);
}

int32_t initPidFile()
{
    char        buffer[65535]   = { 0x00 };
    int32_t     res, err        = -1;
    FILE        *file           = NULL;
    string      file_name;
    string      file_mode;

    file_name = IPNOISE_DB_FILE_PID;
    file_mode = "w";
    file = fopen(file_name.c_str(), file_mode.c_str());
    if (!file){
        PERROR("cannot open: '%s' for mode: '%s'\n",
            file_name.c_str(),
            file_mode.c_str()
        );
        goto fail;
    }
    res = snprintf(buffer, sizeof(buffer),
        "%d",
        getpid()
    );
    if (0 >= res){
        fclose(file);
        PERROR("snprintf failed, res: '%d'\n", res);
        goto fail;
    }

    res = fwrite(buffer, 1, res, file);
    if (0 >= res){
        fclose(file);
        PERROR("fwrite: '%s' to '%s' failed,"
            " res: '%d\n",
            buffer,
            file_name.c_str(),
            res
        );
        goto fail;
    }

    fclose(file);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

int32_t initDb()
{
    int32_t err = -1;

    PDEBUG(10, "init DB\n");

    ::s_mkdir(IPNOISE_DB_OBJECTS, 1);
    ::s_mkdir(IPNOISE_DB_ACTIONS, 1);

    // all ok
    err = 0;

    return err;
}

void thread_run(
    const string &a_thread_path)
{
    ObjectSptr  thread;
    ThreadsIt   it;
    string      object_type;

    if (a_thread_path.empty()){
        PFATAL("missing argument: 'a_thread_path'\n");
    }

    object_type = "core.object.thread";
    thread      = objectCreate(object_type);
    if (!thread){
        PFATAL("cannot create object: '%s',"
            " object not found\n",
            object_type.c_str()
        );
    }
    thread->setObjectPath(a_thread_path.c_str());
    thread->load();
    thread->run();

    // remove from threads list
    {
        lock_guard<recursive_mutex> guard(g_threads_rmutex);

        it = g_threads.find(a_thread_path);
        if (g_threads.end() != it){
            // let him leave us
            it->second->detach();
            g_threads.erase(it);
        }
    }

    return;
}

int32_t run()
{
    char      ev_buffer[65535] = { 0x00 };
    char      *ev_buffer_read  = ev_buffer;
    char      *ev_buffer_write = ev_buffer;
    char      *ev_buffer_end   = ev_buffer + sizeof(ev_buffer);
    int32_t   res, err         = -1;
    int32_t   fd               = -1;
    uint32_t  mask             = 0;

    InotifyWatches          inotify_paths;
    struct inotify_event    *ev = NULL;

    PDEBUG(10, "run server\n");

    // init inotify
    fd = inotify_init();
    if (0 > fd){
        PERROR("cannot init inotify\n");
        goto fail;
    }

    // add for watch
    {
        string  path;
        path = IPNOISE_DB_ROOT;
        mask = IN_DELETE; //IN_ALL_EVENTS;

        PWARN("inotify_add_watch: '%s'\n", path.c_str());

        res  = inotify_add_watch(
            fd,
            path.c_str(),
            mask
        );
        if (0 > res){
            PERROR("cannot add for watch: '%s':\n"
                "  mask: 0x%x\n"
                "  res:  '%d'\n",
                path.c_str(),
                mask,
                res
            );
            goto fail;
        }
        inotify_paths[res] = path;
    }

    // add for watch
    {
        string  path;
        path = IPNOISE_DB_THREADS;
        mask = IN_CREATE; //IN_ALL_EVENTS;

        PWARN("inotify_add_watch: '%s'\n", path.c_str());

        res  = inotify_add_watch(
            fd,
            path.c_str(),
            mask
        );
        if (0 > res){
            PERROR("cannot add for watch: '%s':\n"
                "  mask: 0x%x\n"
                "  res:  '%d'\n",
                path.c_str(),
                mask,
                res
            );
            goto fail;
        }
        inotify_paths[res] = path;
    }

    // add for watch
    {
        string  path;
        path = IPNOISE_DB_ACTIONS;
        mask = IN_CREATE;

        PWARN("inotify_add_watch: '%s'\n", path.c_str());

        res  = inotify_add_watch(
            fd,
            path.c_str(),
            mask
        );
        if (0 > res){
            PERROR("cannot add for watch: '%s':\n"
                "  mask: 0x%x\n"
                "  res:  '%d'\n",
                path.c_str(),
                mask,
                res
            );
            goto fail;
        }
        inotify_paths[res] = path;
    }

    // start main loop
    g_state = STATE_MAIN_LOOP_STARTED;
    while (g_state == STATE_MAIN_LOOP_STARTED){
        ev      = (struct inotify_event *)ev_buffer_read;
        res     = read(
            fd,
            ev_buffer_write,
            ev_buffer_end - ev_buffer_write
        );

        if (0 < res){
            ev_buffer_write += res;
        }

#ifdef __x86_64__
        PDEBUG(50, "after read from fd: '%d':\n"
            "  res:                              '%d'\n"
            "  ev_buffer:                        0x%lx\n"
            "  ev_buffer read:                   0x%lx\n"
            "  ev_buffer write:                  0x%lx\n"
            "  ev_buffer end:                    0x%lx\n"
            "  ev_buffer_write - ev_buffer_read: '%ld'\n"
            "  sizeof(*ev):                      '%ld'\n",
            fd,
            res,
            (uint64_t)ev_buffer,
            (uint64_t)ev_buffer_read,
            (uint64_t)ev_buffer_write,
            (uint64_t)ev_buffer_end,
            (uint64_t)(ev_buffer_write - ev_buffer_read),
            (uint64_t)sizeof(*ev)
        );
#else
        PDEBUG(50, "after read from fd: '%d':\n"
            "  res:                              '%d'\n"
            "  ev_buffer:                        0x%x\n"
            "  ev_buffer read:                   0x%x\n"
            "  ev_buffer write:                  0x%x\n"
            "  ev_buffer end:                    0x%x\n"
            "  ev_buffer_write - ev_buffer_read: '%d'\n"
            "  sizeof(*ev):                      '%d'\n",
            fd,
            res,
            (uint32_t)ev_buffer,
            (uint32_t)ev_buffer_read,
            (uint32_t)ev_buffer_write,
            (uint32_t)ev_buffer_end,
            (uint32_t)(ev_buffer_write - ev_buffer_read),
            (uint32_t)sizeof(*ev)
        );
#endif
        while (sizeof(*ev)
            <= uint64_t(ev_buffer_write - ev_buffer_read))
        {
            string              full_path;
            InotifyWatchesIt    it;
            int32_t             wd   = ev->wd;
            uint32_t            mask = ev->mask;

            // prepare for read next event
            ev_buffer_read += sizeof(*ev);
            ev_buffer_read += ev->len;

            // search watch descriptor
            it = inotify_paths.find(wd);
            if (it == inotify_paths.end()){
                PWARN("watch descriptor: '%d' was not found"
                    " in watch list, event will be skipped\n",
                    wd
                );
                continue;
            }
            full_path = it->second;

            PDEBUG(10, "read event:\n"
                "  ev->len:     '%d'\n"
                "  ev->name:    '%s'\n"
                "  wd:          '%d'\n"
                "  mask:        0x%x\n",
                ev->len,
                (0 < ev->len) ? ev->name : "",
                wd,
                mask
            );

            // may be event has sub path?
            if (ev->len){
                full_path += ev->name;
            }

            // monitor root dir
            {
                if (    IN_DELETE & mask
                    &&  IPNOISE_DB_FILE_PID == full_path)
                {
                    PWARN("db pid file removed, shutdown..\n");
                    // request quit
                    g_state = STATE_SHUTDOWN;
                    break;
                }
            }

            // monitor threads dir
            {
                string  prefix;
                prefix = IPNOISE_DB_THREADS;
                prefix += "static.thread.";

                if (full_path.substr(0, prefix.size())
                    == prefix)
                {
                    if (IN_CREATE & mask){
                        // start new thread
                        lock_guard<recursive_mutex> guard(
                            g_threads_rmutex
                        );
                        g_threads[full_path] = ThreadSptr(
                            new thread(
                                thread_run,
                                full_path
                            )
                        );
                    }
                }
            }

            // monitor actions dir
            {
                string  prefix;
                prefix = IPNOISE_DB_ACTIONS;

                if (full_path.substr(0, prefix.size())
                    == prefix)
                {
                    if (IN_CREATE & mask){
                        DIR             *dirp   = NULL;
                        struct dirent   *dir_ent = NULL;
                        string          action_id;

                        PDEBUG(50, "new action detected: '%s'\n",
                            full_path.c_str()
                        );

                        action_id = getIdFromPath(
                            full_path.c_str()
                        );

                        // search thread for process
                        dirp = opendir(IPNOISE_DB_THREADS);
                        if (!dirp){
                            PFATAL("cannot open: '%s' dir\n",
                                IPNOISE_DB_THREADS
                            );
                        }

                        do {
                            string      thread_path;
                            string      dirent_name;
                            string      suffix;
                            struct stat st;
                            string      actions_path;
                            string      src, dst;

                            dir_ent = readdir(dirp);
                            if (!dir_ent){
                                break;
                            }

                            if (    !strcmp(".",  dir_ent->d_name)
                                ||  !strcmp("..", dir_ent->d_name))
                            {
                                continue;
                            }

                            thread_path = IPNOISE_DB_THREADS;
                            thread_path += dir_ent->d_name;

                            if (DT_LNK != dir_ent->d_type){
                                PWARN("skip '%s' because"
                                    " it is not symlink\n",
                                    thread_path.c_str()
                                );
                                continue;
                            }

                            // check object's 'actions' dir
                            actions_path = thread_path;
                            actions_path += "/";
                            actions_path += OBJECT_DIR_ACTIONS;
                            res = stat(
                                actions_path.c_str(),
                                &st
                            );
                            if (    res
                                ||  !S_ISDIR(st.st_mode))
                            {
                                PWARN("skip '%s' because"
                                    " '%s' not exit or"
                                    " it is not directory\n",
                                    thread_path.c_str(),
                                    actions_path.c_str()
                                );
                                continue;
                            }

                            // all ok, we have found thread
                            // move action to him
                            dst  = actions_path;
                            dst += action_id;

                            src = IPNOISE_DB_OBJECTS;
                            src += action_id;

                            PDEBUG(10, "move action:"
                                " '%s' => '%s'\n",
                                src.c_str(),
                                dst.c_str()
                            );

                            // may be dst already exist?
                            res = stat(dst.c_str(), &st);
                            if (!res){
                                // unlink for recreate
                                unlink(dst.c_str());
                            }

                            // create link
                            res = symlink(
                                src.c_str(),
                                dst.c_str()
                            );
                            if (res){
                                PERROR("cannot create symlink:"
                                    " '%s' to '%s'\n",
                                    src.c_str(),
                                    dst.c_str()
                                );
                                continue;
                            }
                            // all ok, unlink old symlink
                            unlink(full_path.c_str());
                            break;
                        } while (dir_ent);

                        closedir(dirp);
                    }
                }
            }

            // remove event from ev_buffer
            // and process next events
            memmove(
                ev_buffer,
                ev_buffer_read,
                ev_buffer_write - ev_buffer_read
            );
            ev_buffer_read  -= sizeof(*ev) + ev->len;
            ev_buffer_write -= sizeof(*ev) + ev->len;
        }
    }

    // all ok
    err = 0;

out:
    if (0 <= fd){
        close(fd);
    }
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

string getVersion()
{
    return IPNOISE_VERSION;
}

ObjectSptr objectCreate(
    const string        &a_type,
    const char          *a_data,
    const uint64_t      &a_data_size,
    const CreateFlags   &a_create_flags)
{
    lock_guard<recursive_mutex> guard(g_modules_rmutex);

    ModuleInfoFullSptr  info;
    ModulesIt           it;
    ObjectSptr          object;
    CreateFlags         create_flags = a_create_flags;
    int32_t             res;

    if (a_type.empty()){
        PFATAL("missing argument: 'a_type'\n");
    }

    it = g_modules.find(a_type);
    if (g_modules.end() == it){
        PDEBUG(50, "cannot create object by type: '%s',"
            " module not found\n",
            a_type.c_str()
        );
        goto out;
    }

    // force to be object
    create_flags = CreateFlags(
        create_flags & ~CreateFlags::CREATE_PROP
    );

    info    = it->second;
    object  = info->so->objectCreate(create_flags);
    if (!object){
        PFATAL("cannot create object by type: '%s',"
            " module not found\n",
            a_type.c_str()
        );
    }

    res = object->do_init_as_object(
        a_data,
        a_data_size
    );
    if (res){
        PDEBUG(50, "cannot create object by type: '%s',"
            " object init failed, res: '%d'\n",
            a_type.c_str(),
            res
        );
        object = ObjectSptr();
        goto fail;
    }

out:
    return object;
fail:
    goto out;
}

ObjectSptr propCreate(
    const string        &a_type,
    const char          *a_data,
    const uint64_t      &a_data_size,
    const CreateFlags   &a_create_flags)
{
    lock_guard<recursive_mutex> guard(g_modules_rmutex);

    ModuleInfoFullSptr  info;
    ModulesIt           it;
    ObjectSptr          object;
    CreateFlags         create_flags = a_create_flags;
    int32_t             res;

    if (a_type.empty()){
        PFATAL("missing argument: 'a_type'\n");
    }

    it = g_modules.find(a_type);
    if (g_modules.end() == it){
        PDEBUG(50, "cannot create object by type: '%s',"
            " module not found\n",
            a_type.c_str()
        );
        goto out;
    }

    // force to be prop
    create_flags = CreateFlags(
        create_flags | CreateFlags::CREATE_PROP
    );

    info    = it->second;
    object  = info->so->objectCreate(create_flags);
    if (!object){
        PFATAL("cannot create prop by type: '%s',"
            " module not found\n",
            a_type.c_str()
        );
    }

    res = object->do_init_as_prop(
        a_data,
        a_data_size
    );
    if (res){
        PDEBUG(50, "cannot create prop by type: '%s',"
            " prop init failed, res: '%d'\n",
            a_type.c_str(),
            res
        );
        object = ObjectSptr();
        goto fail;
    }

out:
    return object;
fail:
    goto out;
}

string normalize_path(
    const char *a_path)
{
    char        buffer[65535] = { 0x00 };
    int32_t     res;
    string      path;

    if (    !a_path
        ||  !strlen(a_path))
    {
        PFATAL("missing argument: 'a_path'\n");
    }

    path = a_path;

    // may be it's symlink? then make path pretty
    res = readlink(
        path.c_str(),
        buffer,
        sizeof(buffer) - 1
    );
    if (0 < res){
        // from man readlink:
        // readlink() does not append a null byte to buf
        buffer[res] = '\0';
        path = buffer;
    }

    PDEBUG(100, "before path normalize: '%s',"
        " after: '%s',"
        " res: '%d'\n",
        a_path,
        path.c_str(),
        res
    );

    return path;
}

ObjectSptr objectLoad(
    const char *a_path)
{
    char        buffer[65535] = { 0x00 };
    string      object_type;
    ObjectSptr  ret;
    int32_t     res;
    string      path;

    if (    !a_path
        ||  !strlen(a_path))
    {
        PFATAL("missing argument: 'a_path'\n");
    }

    path = normalize_path(a_path);
    if ('/' != path.at(0)){
        // it's object ID add prefix
        path = IPNOISE_DB_OBJECTS;
        path += a_path;
    }

    PDEBUG(50, "load object from: '%s'\n",
        path.c_str()
    );

    // get object type
    do {
        string      full_path;
        FILE        *file = NULL;

        full_path = path;
        full_path += "/type";

        file = fopen(full_path.c_str(), "r");
        if (!file){
            break;
        }

        res = fread(buffer, 1, sizeof(buffer) - 1, file);
        if (0 < res){
            buffer[res] = '\0';
            object_type = buffer;
        }

        fclose(file);
    } while (0);

    if (object_type.empty()){
        PDEBUG(50, "cannot load object: '%s',"
            " cannot get object type\n",
            path.c_str()
        );
        goto fail;
    }

    ret = objectCreate(object_type.c_str());
    if (ret){
        ret->setObjectPath(path.c_str());
        ret->load();
    } else {
        PDEBUG(50, "cannot load object: '%s',"
            " cannot create object with type: '%s'\n",
            path.c_str(),
            object_type.c_str()
        );
    }

out:
    return ret;
fail:
    goto out;
}

/*
void objectProcessAction(
    ObjectSptr  a_object,
    ObjectSptr  a_action)
{
    string              object_path;
    string              object_type;
    string              action_path;
    string              action_type;
    string              action_name;
    string              action_name_orig;
    ModulesIt           modules_it;
    ActionsHandlersIt   handlers_it;
    TranslateTable      in_translate;
    TranslateTableIt    in_translate_it;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    if (!a_action){
        PFATAL("missing argument: 'a_action'\n");
    }

    object_type = a_object->getType();
    object_path = a_object->getObjectPath();

    action_type      = a_action->getType();
    action_path      = a_action->getObjectPath();
    action_name_orig = a_action->getName();
    action_name      = action_name_orig;

    modules_it = g_modules.find(object_type);
    if (g_modules.end() == modules_it){
        PERROR("object: '%s' (type: '%s')"
            " cannot process action: '%s'"
            " (type: '%s', name: '%s'),"
            " module not found\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            action_type.c_str(),
            action_name.c_str()
        );
        goto fail;
    }

    // get object's input translate table
    a_object->getInTranslateTable(in_translate);
    in_translate_it = in_translate.find(action_name);
    if (in_translate.end() != in_translate_it){
        action_name = in_translate_it->second;
        PWARN("object: '%s' (type: '%s')"
            " translate action '%s' to '%s'\n",
            object_path.c_str(),
            object_type.c_str(),
            action_name_orig.c_str(),
            action_name.c_str()
        );
    }

    handlers_it = modules_it->second->actions_in.find(action_name);
    if (modules_it->second->actions_in.end() == handlers_it){
        PERROR("object: '%s' (type: '%s')"
            " cannot process object: '%s'"
            " (type: '%s', name: '%s'),"
            " handler was not found\n",
            object_path.c_str(),
            object_type.c_str(),
            action_path.c_str(),
            action_type.c_str(),
            action_name.c_str()
        );
        goto fail;
    }

    // process action
    handlers_it->second(a_object);

out:
    return;
fail:
    goto out;
}
*/

/*
void objectActionIn(
    ObjectSptr  a_object,
    ObjectSptr  a_action)
{
    lock<recursive_mutex>(g_modules_rmutex, g_engine_info_rmutex);
    lock_guard<recursive_mutex> guard1(g_modules_rmutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(g_engine_info_rmutex, adopt_lock);

    string          object_path;
    string          object_type;
    string          actions_path;
    DIR             *dirp       = NULL;
    struct dirent   *dir_ent    = NULL;

    if (!a_object){
        PFATAL("missing argument: 'a_object'\n");
    }

    object_type = a_object->getType();
    object_path = a_object->getObjectPath();
    if (object_path.empty()){
        PFATAL("cannot get object path\n");
    }

    actions_path = object_path + OBJECT_DIR_ACTIONS;

    PDEBUG(50, "object: '%s' (type: '%s')"
        " process actions from: '%s'\n",
        object_path.c_str(),
        object_type.c_str(),
        actions_path.c_str()
    );

    // open actions dir
    dirp = opendir(actions_path.c_str());
    if (!dirp){
        PFATAL("cannot open actions dir: '%s'\n",
            actions_path.c_str()
        );
    }

    // read actions dir
    do {
        ObjectSptr  action;
        string      action_path;

        dir_ent = readdir(dirp);
        if (!dir_ent){
            break;
        }

        if (    !strcmp(dir_ent->d_name, ".")
            ||  !strcmp(dir_ent->d_name, ".."))
        {
            continue;
        }

        action_path = actions_path;
        action_path += dir_ent->d_name;

        PDEBUG(50, "object: '%s', was found action: '%s'\n",
            object_path.c_str(),
            action_path.c_str()
        );

        // load action
        action = g_engine_info->objectLoad(action_path.c_str());
        if (!action){
            PERROR("cannot load action from path: '%s'\n",
                action_path.c_str()
            );
        }

        // unlink action from action's dir
        unlink(action_path.c_str());

        if (action){
            objectProcessAction(a_object, action);
        }
    } while (dir_ent);

    closedir(dirp);
    PWARN("objectActionOut, object: '%s', action: '%s'\n",
        a_object->getObjectPath().c_str(),
        a_action->getObjectPath().c_str()
    );
}

void objectActionOut(
    ObjectSptr  a_object,
    ObjectSptr  a_action)
{
    lock<recursive_mutex>(g_modules_rmutex, g_engine_info_rmutex);
    lock_guard<recursive_mutex> guard1(g_modules_rmutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(g_engine_info_rmutex, adopt_lock);

    PWARN("objectActionOut, object: '%s', action: '%s'\n",
        a_object->getObjectPath().c_str(),
        a_action->getObjectPath().c_str()
    );
}
*/

void shutdownModules()
{
    lock_guard<recursive_mutex> guard(g_modules_rmutex);

    int32_t res;

    PDEBUG(10, "shutdown modules..\n");

    while (g_modules.size()){
        ModulesIt           it             = g_modules.begin();
        string              module_name    = it->first;
        ModuleInfoFullSptr  module_info    = it->second;

        PDEBUG(15, "shutdown module: '%s'\n",
            module_name.c_str()
        );

        res = module_info->so->shutdown();
        if (res){
            PERROR("cannot shutdown module: '%s',"
                " res: '%d'\n",
                module_name.c_str(),
                res
            );
        }

        if (module_info->so_handle){
            dlclose(module_info->so_handle);
            module_info->so_handle = NULL;
        }

        g_modules.erase(it);
    }
}

int32_t initModule(
    const string &a_path)
{
    lock<recursive_mutex>(g_modules_rmutex, g_engine_info_rmutex);
    lock_guard<recursive_mutex> guard1(g_modules_rmutex, adopt_lock);
    lock_guard<recursive_mutex> guard2(g_engine_info_rmutex, adopt_lock);

    int32_t     res, err        = -1;
    uint32_t    i;
    ModulesIt   it;
    string      symbol_name;
    string      module_type;
    string      module_type2;

    ModuleInfoFullSptr  info;

    if (a_path.empty()){
        PFATAL("missing argument: 'a_path'\n");
    }

    module_type = getModuleTypeFromPath(a_path);
    if (module_type.empty()){
        PFATAL("cannot get module type from path: '%s'\n",
            a_path.c_str()
        );
    }

    PDEBUG(10, "init module: '%s', from file: '%s'\n",
        module_type.c_str(),
        a_path.c_str()
    );

    // may be already registered?
    it = g_modules.find(module_type);
    if (g_modules.end() != it){
        // already inited
        PWARN("attempt to double init module: '%s'\n",
            module_type.c_str()
        );
        goto fail;
    }

    // prepare info
    info = shared_ptr<ModuleInfoFull>(new ModuleInfoFull);

    // store in modules list
    g_modules[module_type] = info;

    // open dll
    info->so_handle = dlopen(a_path.c_str(), RTLD_LAZY);
    if (!info->so_handle){
        PERROR("cannot open: '%s' for execute (%s)\n",
            a_path.c_str(),
            dlerror()
        );
        goto fail;
    }

    // prepare module type for dll
    for (i = 0; i < module_type.size(); i++){
        char c = module_type.at(i);
        if ('.' == c){
            module_type2 += '_';
        } else {
            module_type2 += c;
        }
    }

    // search symbol
    {
        symbol_name = "g_debug_level";
        int32_t *debug_level = (int32_t *)dlsym(
            info->so_handle,
            symbol_name.c_str()
        );
        if (!debug_level){
            PERROR("cannot find symbol: '%s'"
                " in module: '%s'\n",
                symbol_name.c_str(),
                a_path.c_str()
            );
            goto fail;
        }
        *debug_level = g_debug_level;
    }

    // search symbol
    {
        ModuleInfo  *mod_info = NULL;

        symbol_name = module_type2;
        mod_info = (ModuleInfo *)dlsym(
            info->so_handle,
            symbol_name.c_str()
        );
        if (!mod_info){
            PERROR("cannot find symbol: '%s'"
                " in module: '%s'\n",
                symbol_name.c_str(),
                a_path.c_str()
            );
            goto fail;
        }

        info->so = mod_info;
    }

    res = info->so->init(g_engine_info);
    if (res){
        PERROR("cannot init module: '%s',"
            " res: '%d'\n",
            a_path.c_str(),
            res
        );
        goto fail;
    }

    // get module's tests
    info->so->getTests(info->tests);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    if (info->so_handle){
        dlclose(info->so_handle);
        info->so_handle = NULL;
    }
    goto out;
}

int32_t initModules(
    const string &a_path)
{
    char            buffer[512] = { 0x00 };
    DIR             *dir        = NULL;
    struct dirent   *dirent     = NULL;
    int32_t         res, err    = -1;
    ModulesIt       it;

    PDEBUG(10, "loading modules from path: '%s'\n",
        a_path.c_str()
    );

    dir = opendir(a_path.c_str());
    if (!dir){
        PERROR("cannot open dir: '%s'\n", a_path.c_str());
        goto fail;
    }

    do {
        string      dirent_name;
        uint32_t    dirent_name_size;
        string      suffix;

        dirent = readdir(dir);
        if (!dirent){
            break;
        }
        if (DT_REG != dirent->d_type){
            continue;
        }

        dirent_name      = dirent->d_name;
        dirent_name_size = dirent_name.size();
        suffix           = ".so";

        if (    suffix.size() < dirent_name_size
            &&  suffix == dirent_name.substr(dirent_name_size - 3))
        {
            res = initModule(
                a_path + "/" + dirent_name
            );
            if (res){
                err = res;
                goto fail;
            }
        }
    } while (dirent);

    // test modules
    for (it = g_modules.begin();
        it != g_modules.end();
        it++)
    {
        vector<TestInfo>::iterator  tests_it;
        string                      tmp_module_path;
        ModuleInfoFullSptr          info = it->second;
        string                      module_type;

        module_type = it->first;

        tmp_module_path = g_engine_info->tmp_path;
        tmp_module_path += module_type;
        tmp_module_path += "/";

        for (tests_it = info->tests.begin();
            tests_it != info->tests.end();
            tests_it++)
        {
            TestInfo    test;
            int32_t     res;
            string      line;
            string      prefix;

            test = *tests_it;

            // prepare test name
            prefix  = "[" + module_type + "] " + test.name + " ";
            line    = ::s_align_len(prefix.c_str(), 60, '.');
            line    += " ";

            if (!test.run){
                // test function not found
                line += "TODO";
                PDEBUG(10, "%s\n", line.c_str());
                continue;
            }

            // run test
            PDEBUG(15, "module: '%s', start test: '%s'\n",
                a_path.c_str(),
                test.name.c_str()
            );
            res = test.run(tmp_module_path.c_str());
            PDEBUG(15, "res: '%d', test.success_code: '%d'\n",
                res,
                test.success_code
            );
            if (res != test.success_code){
                snprintf(buffer, sizeof(buffer),
                    "failed (res: '%d', but must be: '%d')",
                    res,
                    test.success_code
                );
                line += buffer;
                PDEBUG(15, "%s\n", line.c_str());
                PERROR("cannot init module: '%s',"
                    " test: '%s' %s\n",
                    module_type.c_str(),
                    test.name.c_str(),
                    buffer
                );
                goto fail;
            } else {
                line += "success";
                PDEBUG(10, "%s\n", line.c_str());
            }
        }
    }

    // all ok
    err = 0;

out:
    if (dir){
        closedir(dir);
    }
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

string getModuleTypeFromPath(
    const string &a_type)
{
    string      ret;
    uint32_t    i;
    string      suffix = ".so";

    // remove path
    for (i = 0; i < a_type.size(); i++){
        char c = a_type.at(i);
        if ('/' == c){
            ret = "";
            continue;
        }

        ret += c;
    }

    if (suffix.size() >= ret.size()){
        // too short
        ret = "";
        goto out;
    }

    if (ret.substr(ret.size() - suffix.size()) != suffix){
        // invalid suffix
        ret = "";
        goto out;
    }

    // remove suffix
    ret = ret.substr(0, ret.size() - suffix.size());

out:
    return ret;
}

int32_t main(
    int32_t     a_argc,
    char        *a_argv[])
{
    char        buffer[1024]    = { 0x00 };
    int32_t     res, err        = -1;
    string      modules_path    = IPNOISE_MODULES_DIR;
    int32_t     start_server    = 0;
    string      obj_create_type;
    string      obj_path;
    int32_t     obj_load        = 0;
    string      obj_load_path;
    string      obj_init;
    string      obj_link_add;
    string      obj_link_erase;
    int32_t     obj_dump        = 0;
    string      obj_dump_path;
    int32_t     obj_save        = 0;
    string      obj_save_path;
    string      obj_link_to;
    int32_t     benchmark       = 0;
    string      tmp_path;

    // get args
    int32_t                 option_index    = 0;
    int32_t                 option          = 0;
    static struct option    long_options[]  = {
        {"help",            0, 0, 'h'},
        {"version",         0, 0, 'V'},
        {"debug",           1, 0, 'D'},
        {"modules",         1, 0, 'M'},
        {"server",          0, 0, 'S'},
        {"tmp",             2, 0, 'T'},
        {"benchmark",       0, 0, 'B'},
        // object operations
        {"create",          1, 0, 'c'},
        {"path",            1, 0, 'p'},
        {"load",            2, 0, 'l'},
        {"init",            1, 0, 'i'},
        {"link-add",        1, 0, 'a'},
        {"link-erase",      1, 0, 'e'},
        {"dump",            2, 0, 'd'},
        {"save",            2, 0, 's'},
        {"to",              1, 0, 't'},
        {0, 0, 0, 0}
    };

    snprintf(buffer, sizeof(buffer),
        "/tmp/neuron_%d/",
        getpid()
    );
    tmp_path = buffer;

    while (1){
        option = getopt_long(
            a_argc,
            a_argv,
            "hVD:M:ST::Bc:p:l::i:a:e:d::s::t:",
            long_options,
            &option_index
        );
        if (-1  == option){
            break;
        }

        switch (option){
            case 0:
                break;

            case 'h':
                usage(a_argc, a_argv);
                err = 0;
                goto out;

            case 'V':
                PINFO("Version: '%s'\n",
                    IPNOISE_VERSION
                );
                err = 0;
                goto out;

            case 'D':
                g_debug_level = atoi(optarg);
                break;

            case 'M':
                modules_path = optarg;
                break;

            case 'S':
                start_server = 1;
                break;

            case 'T':
                if (optarg){
                    tmp_path = optarg;
                    if ('/' != tmp_path.at(
                        tmp_path.size() - 1))
                    {
                        // add '/' to end if not exist
                        tmp_path += "/";
                    }
                }
                break;

            case 'B':
                benchmark = 1;
                break;

            // object operations
            case 'c':
                obj_create_type = optarg;
                break;

            case 'p':
                obj_path = optarg;
                if ('/' != obj_path.at(obj_path.size() - 1)){
                    // add '/' to end if not exist
                    obj_path += "/";
                }
                break;

            case 'l':
                obj_load = 1;
                if (optarg){
                    obj_load_path = optarg;
                    if ('/' != obj_load_path.at(
                        obj_load_path.size() - 1)
                    ){
                        // add '/' to end if not exist
                        obj_load_path += "/";
                    }
                }
                break;

            case 'i':
                obj_init = optarg;
                break;

            case 'a':
                obj_link_add = optarg;
                if ('/' != obj_link_add.at(
                    obj_link_add.size() - 1)
                ){
                    // add '/' to end if not exist
                    obj_link_add += "/";
                }
                break;

            case 'e':
                obj_link_erase = optarg;
                if ('/' != obj_link_erase.at(
                    obj_link_erase.size() - 1)
                ){
                    // add '/' to end if not exist
                    obj_link_erase += "/";
                }
                break;

            case 'd':
                obj_dump = 1;
                if (optarg){
                    obj_dump_path = optarg;
                }
                break;

            case 's':
                obj_save = 1;
                if (optarg){
                    obj_save_path = optarg;
                    if ('/' != obj_save_path.at(
                        obj_save_path.size() - 1)
                    ){
                        // add '/' to end if not exist
                        obj_save_path += "/";
                    }
                }
                break;

            case 't':
                obj_link_to = optarg;
                break;

            case '?':
                err = -1;
                goto fail;
        }
    }

    // check another args
    if (optind < a_argc){
        char *buffer_ptr = buffer;
        res = snprintf(
            buffer_ptr,
            (buffer + sizeof(buffer)) - buffer_ptr,
            "Warning: argument(s) will be ignored: "
        );
        if (0 < res){
            buffer_ptr += res;
        }
        while (optind < a_argc){
            snprintf(
                buffer_ptr,
                (buffer + sizeof(buffer)) - buffer_ptr,
                "'%s' ",
                a_argv[optind++]
            );
        }
        PERROR("%s\n", buffer);
    }

    if (g_debug_level){
        PINFO("Options:\n");
        PINFO("debug level:      '%d'\n", g_debug_level);
        PINFO("modules path:     '%s'\n", modules_path.c_str());
        PINFO("start_server:     '%d'\n", start_server);
        PINFO("\n");
        PINFO("obj_create_type:  '%s'\n", obj_create_type.c_str());
        PINFO("obj_path:         '%s'\n", obj_path.c_str());
        PINFO("obj_load:         '%d'\n", obj_load);
        PINFO("obj_load_path:    '%s'\n", obj_load_path.c_str());
        PINFO("obj_init:         '%s'\n", obj_init.c_str());
        PINFO("obj_link_add:     '%s'\n", obj_link_add.c_str());
        PINFO("obj_link_erase:   '%s'\n", obj_link_erase.c_str());
        PINFO("obj_dump:         '%d'\n", obj_dump);
        PINFO("obj_dump_path:    '%s'\n", obj_dump_path.c_str());
        PINFO("obj_save:         '%d'\n", obj_save);
        PINFO("obj_save_path:    '%s'\n", obj_save_path.c_str());
        PINFO("obj_link_to:      '%s'\n", obj_link_to.c_str());
        PINFO("tmp_path:         '%s'\n", tmp_path.c_str());
        PINFO("hw concurrency:   '%d'\n",
            thread::hardware_concurrency()
        );
    }

    // init random generator
    srand(time(NULL));

    // fill info about engine
    {
        lock_guard<recursive_mutex> guard(g_engine_info_rmutex);

        g_engine_info               = new EngineInfo;
        g_engine_info->getVersion   = getVersion;
        g_engine_info->objectCreate = objectCreate;
        g_engine_info->objectLoad   = objectLoad;
        g_engine_info->propCreate   = propCreate;
        g_engine_info->tmp_path     = tmp_path;
    }

    // create tmp_path
    res = ::s_mkdir(tmp_path.c_str(), 1);
    if (res){
        err = res;
        PERROR("cannot create dir: '%s', res: '%d'\n",
            tmp_path.c_str(),
            res
        );
        goto fail;
    }

    signal(SIGINT,  sigint);
    signal(SIGTERM, sigterm);

    // init DB
    res = initDb();
    if (res){
        PERROR("cannot init db, res: '%d'\n", res);
        err = res;
        goto fail;
    }

    // init modules
    res = initModules(modules_path);
    if (res){
        PERROR("cannot init one or more modules:\n"
            "  dir: '%s'\n"
            "  res: '%d'\n",
            modules_path.c_str(),
            res
        );
        err = res;
        goto fail;
    }

    if (benchmark){
        int32_t         objects_count = 1000;
        ObjectSptr      test[objects_count];
        int32_t         i;
        struct timeval  tv;
        double          start_time      = 0.0f;
        double          end_time        = 0.0f;
        double          elapsed_time    = 0.0f;
        string          cmd, tmp_path;

        // recreate tmp dir
        {
            lock_guard<recursive_mutex> guard(g_engine_info_rmutex);

            snprintf(buffer, sizeof(buffer),
                "%stest_timings/",
                g_engine_info->tmp_path.c_str()
            );
            tmp_path = buffer;
        }

        res = ::s_recreate_dir(tmp_path.c_str());
        if (res){
            PFATAL("cannot recreate: '%s', res: '%d'\n",
                tmp_path.c_str(),
                res
            );
        }

        PINFO("will be created, saved and loaded:"
            " '%d' objects in temp dir: '%s'\n",
            objects_count,
            tmp_path.c_str()
        );

        // start work with objects
        gettimeofday(&tv, NULL);
        start_time = tv.tv_sec + tv.tv_usec / 1e6;

        for (i = 0; i < objects_count; i++){
            snprintf(buffer, sizeof(buffer),
                "%s/test_%d",
                tmp_path.c_str(),
                i
            );
            test[i] = objectCreate("core.object.test");
            test[i]->save(buffer);
            test[i]->load(buffer);
        }

        gettimeofday(&tv, NULL);
        end_time     = tv.tv_sec + tv.tv_usec / 1e6;
        elapsed_time = end_time - start_time;

        // clean temp dir
        res = ::s_rm(tmp_path.c_str(), 1, 1);
        if (res){
            PFATAL("cannot rm: '%s', res: '%d'\n",
                tmp_path.c_str(),
                res
            );
        }

        PINFO("was created, saved and loaded: '%d' objects:\n"
            "  time elapsed:    '%f' msec\n"
            "  time per object: '%f' msec\n",
            objects_count,
            1000 * elapsed_time,
            1000 * elapsed_time/objects_count
        );
    }

    if (start_server){
        int32_t res;

        ::s_recreate_dir(IPNOISE_DB_THREADS);

        res = initPidFile();
        if (res){
            err = res;
            PERROR("cannot init pid file\n");
            goto fail;
        }

        res = run();
        if (res){
            PERROR("cannot init inotify, res: '%d'\n", res);
            err = res;
            goto fail;
        }

        shutdownThreads();
        removePidFile();
    } else {
        ObjectSptr  object;
        struct stat st;
        int32_t     res;

        res = stat(IPNOISE_DB_ROOT, &st);
        if (res){
            // looks like it's first start, init DB
            initDb();
        }

        // work with object
        if (!obj_create_type.empty()){
            object = objectCreate(
                obj_create_type.c_str(),
                obj_init.c_str(),
                obj_init.size()
            );
            if (!object){
                PERROR("cannot create object: '%s',"
                    " object not found\n",
                    obj_create_type.c_str()
                );
                goto fail;
            }
        }
        if (!obj_path.empty()){
            if (object){
                object->setObjectPath(obj_path.c_str());
            } else {
                PERROR("cannot set object path,"
                    " empty object\n"
                );
                goto fail;
            }
        }
        if (obj_load){
            if (object){
                if (!obj_load_path.empty()){
                    object->load(obj_load_path.c_str());
                } else {
                    object->load();
                }
            } else {
                if (obj_load_path.empty()){
                    PERROR("cannot load object,"
                        " empty load path\n"
                    );
                    goto fail;
                }
                object = objectLoad(obj_load_path.c_str());
            }
        }
        if (!obj_link_add.empty()){
            if (object){
                object->linkAdd(obj_link_add.c_str());
            } else {
                PERROR("link add failed, empty object\n");
            }
        }
        if (!obj_link_erase.empty()){
            if (object){
                object->linkErase(obj_link_erase.c_str());
            } else {
                PERROR("link remove failed, empty object\n");
            }
        }
        if (obj_dump){
            if (!obj_dump_path.empty()){
                PFATAL("not implemented\n");
            }
            if (object){
                string val;
                object->serialize(val);
                fprintf(stdout, "%s\n", val.c_str());
            } else {
                PERROR("dump failed, empty object\n");
            }
        }
        if (obj_save){
            if (object){
                if (!obj_save_path.empty()){
                    object->save(obj_save_path.c_str());
                } else {
                    object->save();
                }
            } else {
                PERROR("save failed, empty object\n");
            }
        }
        if (!obj_link_to.empty()){
            string      src;
            string      dst;
            struct stat st;

            src = object->getObjectPath();
            if (src.empty()){
                PERROR("cannot get object's path\n");
                goto fail;
            }
            dst = obj_link_to;

            // may be dst is directory?
            res = stat(dst.c_str(), &st);
            if (    !res
                &&  S_ISDIR(st.st_mode))
            {
                // dst is directory, work around
                dst += "/";
                dst += object->getId();
            }

            // may be dst already exist?
            res = stat(dst.c_str(), &st);
            if (!res){
                // unlink for recreate
                unlink(dst.c_str());
            }

            // create link
            res = symlink(src.c_str(), dst.c_str());
            if (res){
                PERROR("symlink from: '%s' to: '%s' failed\n",
                    src.c_str(),
                    dst.c_str()
                );
            }
        }
    }

    // all ok
    err = 0;

out:
    shutdownModules();
    res = ::s_rm(tmp_path.c_str(), 1, 1);
    if (res){
        PERROR("cannot remove: '%s'\n", tmp_path.c_str());
    }
    // Note: if any of Object* destructor will be called
    // after shutdownModules(), we will get SEGFAULT
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

