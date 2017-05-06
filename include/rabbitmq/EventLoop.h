#ifndef MY_RABBIT_MQ_EVENT_LOOP_H
#define MY_RABBIT_MQ_EVENT_LOOP_H

#include <queue>
#include <vector>
#include <thread>
#include <mutex>

#include <poll.h>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"

namespace rabbitmq
{
class EventLoop
{
private:
    struct ConnectionItem
    {
        AMQP::TcpConnection* m_connection;
        int m_fd;
        int m_flags;

        ConnectionItem(AMQP::TcpConnection* connection, const int fd, const int flags):
            m_connection(connection),
            m_fd(fd),
            m_flags(flags)
        {}
    };

    enum class State
    {
        created,
        started,
        stopped,
    };

    logger::CategoryPtr m_logger;

    std::thread m_mainThread;
    std::queue<ConnectionItem> m_connectionItemsQueue;
    std::mutex m_connectionItemsQueueGuard;

    std::vector<ConnectionItem> m_connectionItems;
    std::vector<pollfd> m_pollFds;

    //State m_state;
    volatile bool m_isRunning = false;

private:
    void func();

public:
    EventLoop();
    ~EventLoop();

    void start();
    void stop();

    template<class... Args>
    void addConnectionItem(Args... args);
};

///////////////////
template<class... Args>
inline void EventLoop::addConnectionItem(Args... args)
{
    std::unique_lock<std::mutex> l(m_connectionItemsQueueGuard);

    m_connectionItemsQueue.emplace(args...);
}

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_EVENT_LOOP_H