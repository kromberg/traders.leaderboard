#ifndef MY_RABBIT_MQ_UTILS_H
#define MY_RABBIT_MQ_UTILS_H

#include <mutex>
#include <condition_variable>

namespace rabbitmq
{

struct SynchObj
{
    std::mutex m_mutex;
    std::condition_variable m_cv;
    volatile bool m_flag = false;

    void reset()
    {
        m_flag = false;
    }
    void set()
    {
        std::unique_lock<std::mutex> l(m_mutex);
        m_flag = true;
        m_cv.notify_one();
    }
    void wait()
    {
        std::unique_lock<std::mutex> l(m_mutex);
        while (!m_flag)
        {
            m_cv.wait(l);
        }
    }
};

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_UTILS_H