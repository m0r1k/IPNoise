#include "class.hpp"
DEFINE_CLASS(FdInfo);

#ifndef FD_INFO_HPP
#define FD_INFO_HPP

#include <stdint.h>
#include <string>
#include <mutex>

DEFINE_CLASS(Thread);

using namespace std;

class FdInfo
{
    public:
        FdInfo();
        FdInfo(
            const int32_t   &a_fd,
            const string    &a_thread_name,
            const string    &a_sess_id,
            const string    &a_req_id
        );
        virtual ~FdInfo();

        // generic
        void            lock();
        void            unlock();

        int32_t         getFd()             const;
        string          getThreadName()     const;
        string          getSessId()         const;
        string          getReqId()          const;

        int32_t         isOnline()          const;
        void            dump(string &)      const;
        ThreadSptr      getThread()         const;

    private:
        int32_t             m_fd;
        string              m_thread_name;
        string              m_sess_id;
        string              m_req_id;
        recursive_mutex     m_mutex;
};

#endif

