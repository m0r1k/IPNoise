#include <unistd.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/context/main.hpp"
#include "core/object/action/main.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/string/main.hpp"
#include "core/object/time/main.hpp"
#include "core/object/timing/main.hpp"
#include "core/object/transaction/main.hpp"
#include "core/object/object/__END__.hpp"
#include "db.hpp"

#include "thread/worker.hpp"

Workers             g_workers;
recursive_mutex     g_workers_mutex;

ThreadWorker::ThreadWorker(
    const string    &a_name)
    :   Thread(a_name)
{
}

ThreadWorker::~ThreadWorker()
{
}

int32_t ThreadWorker::loop_before()
{
    int32_t ret = 0;

    PDEBUG(60, "thread: '%s' (0x%llx)"
        " loop_before\n",
        getName().c_str(),
        getAddr()
    );

    return ret;
}

void ThreadWorker::loop_tick()
{
//    lock_guard<recursive_mutex> guard(m_mutex);

    //PTIMING(0.030f, "thread: '%s' (0x%llx)"
    //    " loop_tick\n",
    //    getName().c_str(),
    //    getAddr()
    //);

    ObjectVectorSptr    actions_for_process;
    ObjectVectorSptr    objects_for_remove;
    ObjectVectorIt      it;
    ObjectContextSptr   context;
    DbSptr              db = Thread::getCurThreadDb();

    PROP_VECTOR(actions_for_process);
    PROP_VECTOR(objects_for_remove);

    db->getActionsForProcess(actions_for_process);
    for (it = actions_for_process->begin();
        it != actions_for_process->end();
        it++)
    {
        ObjectSptr       cur_action_tmp = *it;
        ObjectActionSptr cur_action;

        //PTIMING(0.030f, "thread: '%s' (0x%llx)"
        //    " process action: '%s' (name: '%s')\n",
        //    getName().c_str(),
        //    getAddr(),
        //    cur_action_tmp->getId().c_str(),
        //    cur_action_tmp->getName()->toString().c_str()
        //);

        context = contextReset();

        cur_action = dynamic_pointer_cast<ObjectAction>(
            cur_action_tmp
        );
        context->transactionAdd(cur_action_tmp);

        // action will remove itself,
        // when all links will be lost (look inside process)
        //cur_action->process();

        {
            ObjectVectorSptr    res;
            ObjectVectorIt      res_it;

            PROP_VECTOR(res);
            cur_action->getNeighs(
                res,
                "",     // name
                0,      // get_removed
                1       // db_lock
            );

            for (res_it = res->begin();
                res_it != res->end();
                res_it++)
            {
                ObjectSptr  object = *res_it;

                context->transactionAdd(object);

                PDEBUG(50, "process action:\n"
                    "  action ID:   '%s'\n"
                    "  action name: '%s'\n"
                    "  time_create: '%s'\n"
                    "  object ID:   '%s'\n"
                    "  object type: '%s'\n"
                    "  object name: '%s'\n"
                    "  object lock: '%s'\n"
                    "\n",
                    cur_action->getId().c_str(),
                    cur_action->getName()->c_str(),
                    cur_action->getTimeCreate()->toString().c_str(),
                    object->getId().c_str(),
                    object->getType().c_str(),
                    object->getName()->toString().c_str(),
                    object->getLock()->toString().c_str()
                );

                Object::processAction(object, cur_action);
                Object::neighUnlink(cur_action, object);
            }
        }

        //context->remove(cur_action, 1);
        context->transactionCommit();
    }

    db->getObjectsForRemove(objects_for_remove);
    for (it = objects_for_remove->begin();
        it != objects_for_remove->end();
        it++)
    {
        ObjectSptr cur_object = *it;

        context = contextReset();
        context->transactionAdd(cur_object);

//        PWARN("MORIK dump transaction before remove: '%s'\n",
//            context->getTransaction()->toString().c_str()
//        );
        //cur_object->remove(1);
        context->remove(cur_object, 1);

        context->transactionCommit();
    }

    context = contextReset();

    //  PWARN("actions_for_process: '%d',"
    //      " objects_for_remove: '%d'\n",
    //      actions_for_process->size(),
    //      objects_for_remove->size()
    //  );

    if (    !actions_for_process->empty()
        ||  !objects_for_remove->empty())
    {
        // we have job, so try search again right now
        speedUp();
    } else {
        // object(s) not found, rest
        speedDown();
    }
}

int32_t ThreadWorker::loop_after()
{
    int32_t ret = 0;

    PDEBUG(60, "thread: '%s' (0x%llx)"
        " loop_after\n",
        getName().c_str(),
        getAddr()
    );

    return ret;
}

void ThreadWorker::setWorkersCount(
    const int32_t &a_count)
{
    lock_guard<recursive_mutex> guard(g_workers_mutex);

    int32_t i;
    char    buffer[1024] = { 0x00 };
    int32_t need_start   = a_count          - g_workers.size();
    int32_t need_stop    = g_workers.size() - a_count;

    // start workers if it need
    for (i = 0; i < need_start; i++){
        ThreadWorkerSptr  thread_worker;
        snprintf(buffer, sizeof(buffer),
            "worker_%d",
            i
        );
        CREATE_THREAD(thread_worker, ThreadWorker, buffer);
        thread_worker->addParent("main");
        g_workers.push_back(thread_worker);
        thread_worker->start();
    }

    // stop workers if it need
    for (i = 0; i < need_stop; i++){
        ThreadWorkerSptr    thread_worker;
        WorkersIt           it;
        if (g_workers.empty()){
            break;
        }
        it              = g_workers.begin();
        thread_worker   = *it;
        thread_worker->stop();
        g_workers.erase(it);
    }
}

