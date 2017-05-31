#ifndef MY_RABBIT_MQ_UTILS_H
#define MY_RABBIT_MQ_UTILS_H

#include <mutex>
#include <condition_variable>
#include <chrono>

namespace rabbitmq
{

template<class T = bool>
class SyncObj
{
private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    volatile bool m_flag = false;
    T m_value;
    int32_t m_waitSeconds;
    int32_t m_waitTimes;
public:
    SyncObj(const T& defaultValue, int32_t waitSeconds = -1, int32_t waitTimes = -1):
        m_value(defaultValue),
        m_waitSeconds(waitSeconds),
        m_waitTimes(waitTimes)
    {}

    void reset()
    {
        m_flag = false;
    }

    template<class U>
    void set(U&& u)
    {
        std::unique_lock<std::mutex> l(m_mutex);
        m_flag = true;
        m_value = std::forward<U>(u);
        m_cv.notify_one();
    }

    const T& get() const
    {
        return m_value;
    }

    void wait()
    {
        int32_t waitsCount = 0;
        std::unique_lock<std::mutex> l(m_mutex);
        while (!m_flag)
        {
            if (m_waitTimes > 0 && waitsCount >= m_waitTimes)
            {
                break;
            }
            if (m_waitSeconds > 0)
            {
                m_cv.wait_for(l, std::chrono::seconds(m_waitSeconds));
            }
            else
            {
                m_cv.wait(l);
            }
            ++ waitsCount;
        }
    }
};

template<>
struct SyncObj<bool>
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