#include <stdio.h>
#include <stdint.h>
#include <getopt.h>
#include <signal.h>

// libevent
#include <event2/event.h>

// stl
#include <thread>

#include <ipnoise-common/log_common.h>
#include "core/object/object/main.hpp"

#include "thread/main.hpp"
#include "thread/test.hpp"
#include "thread/worker.hpp"

#include "main.hpp"

int32_t g_debug_level = 10;

void usage(
    int32_t a_argc,
    char    *a_argv[])
{
    PINFO("Usage:     %s [-options]                     \n"
        "options:                                       \n"
        "         -h, --help          This help         \n"
        "         -c, --config        Path to config.xml\n"
        "         -v, --version       Program version   \n"
        "         --debug   <level>   Debug level       \n"
        "         --workers <count>   Workers count     \n"
        "\n",
        a_argv[0]
    );
}

int32_t main(
    int32_t argc,
    char    *argv[])
{
    char            buffer[1024]    = { 0x00 };
    int32_t         res, err        = 0;
    int32_t         workers_count   = 1;
    ThreadMainSptr  thread_main;

    // get args
    while (1){
        int32_t option_index = 0;
        static struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"version",     0, 0, 'v'},
            {"debug",       1, 0, 'd'},
            {"workers",     1, 0, 'w'},
            {0, 0, 0, 0}
        };

        int32_t option = getopt_long(argc, argv, "hvd:w:",
            long_options,
            &option_index
        );
        if (-1 == option){
            break;
        }

        switch (option){
            case 0:
                break;

            case 'h':
                usage(argc, argv);
                exit(0);

            case 'd':
                g_debug_level = atoi(optarg);
                break;

            case 'w':
                workers_count = atoi(optarg);
                break;

            case 'v':
                PINFO("Version: '%s'\n",
                    IPNOISE_OBJECT_VERSION
                );
                exit(0);

            case '?':
                exit (1);
        }
    }

    // check another args
    if (optind < argc){
        char *buffer_ptr = buffer;
        res = snprintf(
            buffer_ptr,
            (buffer + sizeof(buffer)) - buffer_ptr,
            "Warning: argument(s) will be ignored: "
        );
        if (0 < res){
            buffer_ptr += res;
        }
        while (optind < argc){
            snprintf(
                buffer_ptr,
                (buffer + sizeof(buffer)) - buffer_ptr,
                "'%s' ",
                argv[optind++]
            );
        }
        PERROR("%s\n", buffer);
    }

    if (g_debug_level){
        PINFO("Options:\n");
        PINFO("debug level:   '%d'\n", g_debug_level);
        PINFO("workers count: '%d'\n", workers_count);
        PINFO("thread::hardware_concurrency: '%d'\n",
            thread::hardware_concurrency()
        );
    }

    // init random generator
    srand(time(NULL));

    if (g_debug_level){
        event_enable_debug_mode();
    }

    // init modules
    res = Object::initModules();
    if (res){
        err = res;
        PERROR("cannot init modules\n");
        goto fail;
    }

    CREATE_THREAD(thread_main, ThreadMain, "main");

    if (0){
        // test
        ThreadTestSptr  thread_test1;
        ThreadTestSptr  thread_test2;
        ThreadTestSptr  thread_test3;

        // parent 1-2-3 child

        // create test thread 1
        CREATE_THREAD(thread_test1, ThreadTest, "1");
        if (!thread_test1){
            PERROR("cannot create thread_test1\n");
            goto fail;
        }
        thread_test1->setWaitSecInLoopBefore(5);
        thread_test1->setWaitSecInLoopAfter(5);

        // create test thread 2
        CREATE_THREAD(thread_test2, ThreadTest, "2");
        if (!thread_test2){
            PERROR("cannot create thread_test2\n");
            goto fail;
        }
        thread_test2->setWaitSecInLoopBefore(3);
        thread_test2->setWaitSecInLoopAfter(3);
        thread_test2->addParent(thread_test1);

        // create test thread 3
        CREATE_THREAD(thread_test3, ThreadTest, "3");
        if (!thread_test3){
            PERROR("cannot create thread_test3\n");
            goto fail;
        }
        thread_test3->setWaitSecInLoopBefore(1);
        thread_test3->setWaitSecInLoopAfter(1);
        thread_test3->addParent(thread_test2);

        // start pending threads
        Thread::startAllThreads();
    }

    // start workers
    ThreadWorker::setWorkersCount(workers_count);

    // start main thread
    thread_main->start();

    Thread::stopAllThreads();

    PINFO("have a nice day!\n");

    // all ok
    err = 0;

out:
  return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

