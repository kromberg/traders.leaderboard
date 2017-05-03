#ifndef TCP_HANDLER_H
#define TCP_HANDLER_H

#include <poll.h>

#include <thread>
#include <mutex>
#include <amqpcpp.h>

namespace rabbitmq
{
class TcpHandler : public AMQP::TcpHandler
{
private:
    typedef AMQP::TcpHandler BaseClass;

    std::thread m_mainThread;
    std::vector<AMQP::TcpConnection*> m_connections;
    std::vector<pollfd> m_pollFds;
    std::vector<int> m_flags;
    std::mutex m_fdsGuard;

    volatile bool m_isRunning = false;

public:
    TcpHandler()
    {
        using std::swap;

        m_isRunning = true;
        std::thread mainThread(std::bind(&TcpHandler::mainThreadFunc, this));
        swap(m_mainThread, mainThread);
    }

private:

    void mainThreadFunc()
    {
        while (m_isRunning)
        {
            {
                std::unique_lock<std::mutex> l(m_fdsGuard);
                if (m_pollFds.empty())
                {
                    l.unlock();
                    sleep(1);
                    continue;
                }
                int res = poll(&m_pollFds[0], m_pollFds.size(), 1000);
                std::cout << "Poll result: " << res << '\n';
                if (res < 0)
                {
                    // todo: handle error
                }
                else if (res == 0)
                {
                    l.unlock();
                    sleep(1);
                    continue;
                }

                for (size_t i = 0, size = m_pollFds.size(); i < size; ++i)
                {
                    // TODO: check error and hup
                    if (m_pollFds[i].revents & POLLIN)
                    {
                        std::cout << "POLLIN event of fd: " << m_pollFds[i].fd << '\n';
                        m_connections[i]->process(m_pollFds[i].fd, m_flags[i]);
                    }
                    if (m_pollFds[i].revents & POLLOUT)
                    {
                        std::cout << "POLLOUT event of fd: " << m_pollFds[i].fd << '\n';
                        m_connections[i]->process(m_pollFds[i].fd, m_flags[i]);
                    }
                }
            }
            sleep(1);
        }
    }

    virtual uint16_t onNegotiate(AMQP::TcpConnection *connection, uint16_t interval) override
    {
        std::cout << "Negotiated\n";
        return BaseClass::onNegotiate(connection, interval);
    }

    virtual void onConnected(AMQP::TcpConnection *connection) override
    {
        std::cout << "Connected\n";
    }
    virtual void onError(AMQP::TcpConnection *connection, const char *message) override
    {
        std::cout << "Connection error\n";
    }
    virtual void onClosed(AMQP::TcpConnection *connection) override
    {
        std::cout << "Connection was closed\n";
        m_isRunning = false;
    }

    /**
     *  Method that is called by the AMQP-CPP library when it wants to interact
     *  with the main event loop. The AMQP-CPP library is completely non-blocking,
     *  and only make "write()" or "read()" system calls when it knows in advance
     *  that these calls will not block. To register a filedescriptor in the
     *  event loop, it calls this "monitor()" method with a filedescriptor and
     *  flags telling whether the filedescriptor should be checked for readability
     *  or writability.
     *
     *  @param  connection      The connection that wants to interact with the event loop
     *  @param  fd              The filedescriptor that should be checked
     *  @param  flags           Bitwise or of AMQP::readable and/or AMQP::writable
     */
    virtual void monitor(AMQP::TcpConnection *connection, int fd, int flags) override
    {
        std::unique_lock<std::mutex> l(m_fdsGuard);
        pollfd newfd;
        memset(&newfd, sizeof(newfd), 0);
        newfd.fd = fd;
        newfd.events = POLLERR | POLLHUP | POLLNVAL;
        std::cout << "Adding new fd with ";
        if (flags & AMQP::readable)
        {
            std::cout << "readable flag\n";
            newfd.events |= POLLIN;
        }
        if (flags & AMQP::writable)
        {
            std::cout << "writable flag\n";
            newfd.events |= POLLOUT;
        }

        m_connections.push_back(connection);
        m_pollFds.emplace_back(std::move(newfd));
        m_flags.emplace_back(flags);
    }
};
} // namespace rabbitmq

#endif // TCP_HANDLER_H