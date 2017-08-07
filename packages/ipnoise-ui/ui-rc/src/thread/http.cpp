#include <QCoreApplication>
#include "thread/http.hpp"

ThreadHttp::ThreadHttp(
    Network         *a_net,
    const string    &a_name)
    :   Thread(a_net, a_name)
{
}

ThreadHttp::~ThreadHttp()
{
}

int32_t ThreadHttp::do_init()
{
    int32_t ret = 0;
    return ret;
}

void ThreadHttp::do_start()
{
    exec();
}

void ThreadHttp::do_stop()
{
}

