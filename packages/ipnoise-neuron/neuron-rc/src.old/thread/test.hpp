#include "class.hpp"
DEFINE_CLASS(ThreadTest);

#ifndef THREAD_TEST_HPP
#define THREAD_TEST_HPP

using namespace std;

#include "thread.hpp"

class ThreadTest
    :   public Thread
{
    public:
        ThreadTest(
            const string &a_name = "test"
        );
        virtual ~ThreadTest();

        // generic
        void setWaitSecInLoopBefore(const int32_t &);
        void setWaitSecInLoopAfter(const int32_t &);

    protected:
        virtual int32_t loop_before();
        virtual int32_t loop_after();

    private:
        int32_t m_wait_sec_in_loop_before;
        int32_t m_wait_sec_in_loop_after;
};

#endif

