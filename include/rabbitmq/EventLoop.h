#ifndef MY_RABBIT_MQ_EVENT_LOOP_H
#define MY_RABBIT_MQ_EVENT_LOOP_H

#include <queue>
#include <vector>
#include <unordered_set>
#include <thread>
#include <mutex>

#include <poll.h>

#include <amqpcpp.h>

#include "../logger/LoggerFwd.h"

#include "Fwd.h"

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

    logger::CategoryPtr m_logger;

    std::thread m_mainThread;
    std::queue<ConnectionItem> m_connectionItemsQueue;
    std::mutex m_connectionItemsQueueGuard;

    std::vector<ConnectionItem> m_connectionItems;
    std::vector<pollfd> m_pollFds;

    volatile bool m_isRunning = false;

    std::unordered_set<Handler*> m_handlers;
    std::mutex m_handlersGuard;

private:
    void func();
    void processConnectionItem(ConnectionItem& item);
    void add(ConnectionItem& item);
    void update(const ConnectionItem& item, std::vector<pollfd>::iterator it);
    void remove(const ConnectionItem& item);

public:
    EventLoop();
    ~EventLoop();

    void start();
    void stop();

    void registerHandler(Handler* handler);
    void unregisterHandler(Handler* handler);

    template<class... Args>
    void addConnectionItem(Args... args);
};

////////////////////////////////////////////////////////////////////////////
// INLINE
////////////////////////////////////////////////////////////////////////////
template<class... Args>
inline void EventLoop::addConnectionItem(Args... args)
{
    std::unique_lock<std::mutex> l(m_connectionItemsQueueGuard);

    m_connectionItemsQueue.emplace(std::forward<Args>(args)...);
}

} // namespace rabbitmq

#endif // MY_RABBIT_MQ_EVENT_LOOP_H