#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include <amqpcpp.h>

namespace rabbitmq
{
class ConnectionHandler : public AMQP::ConnectionHandler
{
    virtual void onData(AMQP::Connection *connection, const char *data, size_t size)
    {
    }

    virtual void onConnected(AMQP::Connection *connection)
    {
    }

    virtual void onError(AMQP::Connection *connection, const char *message)
    {
        
    }
    virtual void onClosed(AMQP::Connection *connection) {}

};
} // namespace rabbitmq

#endif // CONNECTION_HANDLER_H