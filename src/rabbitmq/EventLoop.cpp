#include <unistd.h>

#include <logger/Logger.h>

#include <rabbitmq/EventLoop.h>

namespace rabbitmq
{

EventLoop::EventLoop()
{}
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
    LOG_INFO("EVENT_LOOP", "Event loop started");
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
                    size_t idx = std::distance(m_pollFds.begin(), fdsIt);

                    LOG_DEBUG("EVENT_LOOP", "Erasing fd: idx = %zu; fd = %d",
                        idx, item.m_fd);

                    // erase
                    m_pollFds.erase(fdsIt);

                    auto connectionItemIt = m_connectionItems.begin();
                    std::advance(connectionItemIt, idx);
                    m_connectionItems.erase(connectionItemIt);
                }
                else
                {
                    LOG_DEBUG("EVENT_LOOP", "Adding new fd: connection = %p; "
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
        //std::cout << "Poll result: " << res << '\n';
        if (res < 0)
        {
            // todo: handle error
        }
        else if (res == 0)
        {

            continue;
        }

        for (size_t i = 0, size = m_pollFds.size(); i < size && res > 0; ++i)
        {
            if (m_pollFds[i].revents & (POLLHUP | POLLERR))
            {
                // TODO: log error
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
    LOG_INFO("EVENT_LOOP", "Event loop stopped");
}

} // namespace rabbitmq