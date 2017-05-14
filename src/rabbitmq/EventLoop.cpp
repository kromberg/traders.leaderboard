#include <unistd.h>

#include <logger/LoggerDefines.h>

#include <rabbitmq/EventLoop.h>

namespace rabbitmq
{

EventLoop::EventLoop()
{
    m_logger = logger::Logger::getLogCategory("EVENT_LOOP");
}
EventLoop::~EventLoop()
{}

void EventLoop::start()
{
    using std::swap;

    m_isRunning = true;
    std::thread mainThread(std::bind(&EventLoop::func, this));
    swap(m_mainThread, mainThread);
}

void EventLoop::stop()
{
    m_isRunning = false;
    m_mainThread.join();
}

void EventLoop::func()
{
    LOG_INFO(m_logger, "Event loop started");
    while (m_isRunning)
    {
        {
            std::unique_lock<std::mutex> l(m_connectionItemsQueueGuard);
            while (!m_connectionItemsQueue.empty())
            {
                ConnectionItem& item = m_connectionItemsQueue.front();
                if (0 == item.m_flags)
                {
                    // remove item
                    auto fdsIt = std::find_if(m_pollFds.begin(), m_pollFds.end(), [&item](const pollfd& fd) -> bool {
                        return fd.fd == item.m_fd;
                    });
                    if (m_pollFds.end() == fdsIt)
                    {
                        LOG_DEBUG(m_logger, "Trying to remove fd: idx = %zu; fd = %d that is not present",
                            idx, item.m_fd);
                    }
                    else
                    {
                        size_t idx = std::distance(m_pollFds.begin(), fdsIt);

                        LOG_DEBUG(m_logger, "Erasing fd: idx = %zu; fd = %d",
                            idx, item.m_fd);

                        // erase
                        m_pollFds.erase(fdsIt);

                        auto connectionItemIt = m_connectionItems.begin();
                        std::advance(connectionItemIt, idx);
                        m_connectionItems.erase(connectionItemIt);
                    }
                }
                else
                {
                    // remove item
                    auto fdsIt = std::find_if(m_pollFds.begin(), m_pollFds.end(), [&item](const pollfd& fd) -> bool {
                        return fd.fd == item.m_fd;
                    });
                    if (m_pollFds.end() == fdsIt)
                    {
                        LOG_DEBUG(m_logger, "Adding new fd: connection = %p; "
                            "fd = %d; flags = %d",
                            item.m_connection, item.m_fd, item.m_flags);

                        pollfd newfd;
                        memset(&newfd, sizeof(newfd), 0);
                        newfd.fd = item.m_fd;
                        newfd.events = POLLERR | POLLHUP | POLLNVAL;
                        if (item.m_flags & AMQP::readable)
                        {
                            newfd.events |= POLLIN;
                        }
                        if (item.m_flags & AMQP::writable)
                        {
                            newfd.events |= POLLOUT;
                        }
                        m_pollFds.emplace_back(std::move(newfd));
                        m_connectionItems.emplace_back(std::move(item));
                        continue;
                    }
                    else
                    {
                        LOG_DEBUG(m_logger, "Updating fd: fd = %d, flags = %d",
                            item.m_fd, item.m_flags);

                        fdsIt->events = POLLERR | POLLHUP | POLLNVAL;
                        if (item.m_flags & AMQP::readable)
                        {
                            fdsIt->events |= POLLIN;
                        }
                        if (item.m_flags & AMQP::writable)
                        {
                            fdsIt->events |= POLLOUT;
                        }
                    }
                }

                m_connectionItemsQueue.pop();
            }
        }
        if (m_pollFds.empty())
        {
            sleep(1);
            continue;
        }
        int res = poll(&m_pollFds[0], m_pollFds.size(), 100);
        if (res < 0)
        {
            LOG_ERROR(m_logger, "Poll operation failed. Errno: %d(%s)",
                errno, std::strerror(errno));
            continue;
        }
        else if (res == 0)
        {
            continue;
        }

        for (size_t i = 0, size = m_pollFds.size(); i < size && res > 0; ++i)
        {
            if (m_pollFds[i].revents & (POLLHUP | POLLERR))
            {
                LOG_ERROR(m_logger, "FD %d is bad. Errno: %d(%s)",
                    m_connectionItems[i].m_fd, errno, std::strerror(errno));
                -- res;
            }
            else if (m_pollFds[i].revents & (POLLIN | POLLOUT))
            {
                m_connectionItems[i].m_connection->process(
                    m_connectionItems[i].m_fd, m_connectionItems[i].m_flags);
                -- res;
            }
        }
    }
    LOG_INFO(m_logger, "Event loop stopped");
}

} // namespace rabbitmq