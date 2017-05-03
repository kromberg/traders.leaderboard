#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include <amqpcpp.h>

namespace rabbitmq
{
class ConnectionHandler : public AMQP::ConnectionHandler
{
    virtual void onData(AMQP::Connection *connection, const char *data, size_t size)
    {
        std::cout << "Data received [" << size << "]: " << data << "\n";
    }

    virtual void onConnected(AMQP::Connection *connection)
    {
        std::cout << "Connected\n";
    }

    virtual void onError(AMQP::Connection *connection, const char *message)
    {
        std::cout << "Error\n";
    }
    virtual void onClosed(AMQP::Connection *connection)
    {
        std::cout << "Closed\n";
    }

};
} // namespace rabbitmq

#endif // CONNECTION_HANDLER_H