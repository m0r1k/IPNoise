#include <unistd.h>

#include "core/object/object/__BEGIN__.hpp"
#include "core/object/vector/main.hpp"
#include "core/object/object/__END__.hpp"

#include "thread/test.hpp"

ThreadTest::ThreadTest(
    const string    &a_name)
    :   Thread(a_name)
{
    m_wait_sec_in_loop_before   = 0;
    m_wait_sec_in_loop_after    = 0;
}

ThreadTest::~ThreadTest()
{
}

void ThreadTest::setWaitSecInLoopBefore(
    const int32_t &a_val)
{
    m_wait_sec_in_loop_before = a_val;
}

void ThreadTest::setWaitSecInLoopAfter(
    const int32_t &a_val)
{
    m_wait_sec_in_loop_after = a_val;
}

int32_t ThreadTest::loop_before()
{
    int32_t ret = 0;
    PDEBUG(30, "thread: '%s', sleep: '%d'\n",
        getName().c_str(),
        m_wait_sec_in_loop_before
    );
    sleep(m_wait_sec_in_loop_before);
    return ret;
}

int32_t ThreadTest::loop_after()
{
    int32_t ret = 0;
    PDEBUG(30, "thread: '%s', sleep: '%d'\n",
        getName().c_str(),
        m_wait_sec_in_loop_after
    );
    sleep(m_wait_sec_in_loop_after);
    return ret;
}

