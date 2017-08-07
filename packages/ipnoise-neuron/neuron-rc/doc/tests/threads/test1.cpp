#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <mutex>
#include <thread>

#include <ipnoise-common/log_common.h>

using namespace std;

int32_t debug_level = 10;

// number of threads = total virtual memory / (stack size*1024*1024)
// Thus, the number of threads per process can be increased by increasing total virtual memory or by decreasing stack size. But, decreasing stack size too much can lead to code failure due to stack overflow while max virtual memory is equals to the swap memory.
// Check you machine:
// Total Virtual Memory:
//      ulimit -v (default is unlimited, thus you need to increase swap memory to increase this)
// Total Stack Size:
//      ulimit -s (default is 8Mb)
// Command to increase these values:
//      ulimit -s newvalue
//      ulimit -v newvalue

void worker_cb(
    int32_t a_id)
{
    int32_t sleep_time = 10;

    // sleep_time = 10.0f * rand() / RAND_MAX;

    PDEBUG(10, "start worker: '%d' sleep_time: '%d'\n",
        a_id,
        sleep_time
    );

    sleep(sleep_time);

    PDEBUG(10, "exit worker: '%d'\n",
        a_id
    );
}

int32_t main(void)
{
    int32_t     i, err          = -1;
    int32_t     test            = 1;
    int32_t     threads_count   = 500;

    thread *    threads[threads_count];

    // clear
    for (i = 0; i < threads_count; i++){
        threads[i] = NULL;
    }

    // init random
    if (test){
        srand(0);
    } else {
        srand(time(NULL));
    }

    // create threads
    for (i = 0; i < threads_count; i++){
        threads[i] = new thread(worker_cb, i);
    }

    // join all
    for (i = 0; i < threads_count; i++){
        threads[i]->join();
    }

    // all ok
    err = 0;

    return err;
}

