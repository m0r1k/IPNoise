//#include "thread/http.hpp"

#include "fd_info.hpp"

FdInfo::FdInfo()
{
    m_fd = -1;
}

FdInfo::FdInfo(
    const int32_t   &a_fd,
    const string    &a_thread_name,
    const string    &a_sess_id,
    const string    &a_req_id)
{
    m_fd            = a_fd;
    m_thread_name   = a_thread_name;
    m_sess_id       = a_sess_id;
    m_req_id        = a_req_id;
}

FdInfo::~FdInfo()
{
}

void FdInfo::lock()
{
    m_mutex.lock();
}

void FdInfo::unlock()
{
    m_mutex.unlock();
}

string FdInfo::getThreadName() const
{
    return m_thread_name;
}

int32_t FdInfo::getFd() const
{
    return m_fd;
}

string FdInfo::getSessId() const
{
    return m_sess_id;
}

string FdInfo::getReqId() const
{
    return m_req_id;
}

int32_t FdInfo::isOnline() const
{
    int32_t ret = 0;
    if (0 <= m_fd){
        ret = 1;
    }
    return ret;
}

void FdInfo::dump(
    string &a_out) const
{
    char buffer[65535] = { 0x00 };
    snprintf(buffer, sizeof(buffer),
        "fd: '%d', sess_id: '%s', req_id: '%s', thread: '%s', online: '%d'\n",
        m_fd,
        m_sess_id.c_str(),
        m_req_id.c_str(),
        m_thread_name.c_str(),
        isOnline()
    );
    a_out += buffer;
}

ThreadSptr FdInfo::getThread() const
{
    ThreadSptr ret;
//    ret = Thread::getThreadByName(m_thread_name);
    return ret;
}

